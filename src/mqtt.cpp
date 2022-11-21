#include <esp_log.h>

#include "mqtt.hpp"
#include "config.hpp"
#include "secret.hpp"
#include "utils.hpp"

// Class variables

std::mutex         MQTT::mutex           = {};
MQTT::State        MQTT::state           = State::NOT_INITIALIZED;

esp_err_t MQTT::init()
{
  esp_log_level_set(TAG, LOG_LEVEL);

  mqtt_cfg.uri       = MQTT_SERVER_URI;
  mqtt_cfg.username  = MQTT_USERNAME;
  mqtt_cfg.password  = MQTT_PASS;
  mqtt_cfg.client_id = MQTT_CLIENT_ID;

  client_handle = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client_handle, MQTT_EVENT_ANY, event_handler, nullptr);
  
  esp_mqtt_client_start(client_handle);

  return ESP_OK;
}

esp_err_t MQTT::publish(const char * topic, const uint8_t * data, int len, int qos, int retain) 
{
  esp_err_t status = ESP_FAIL;
  if ((state == State::CONNECTED) || (state == State::PUBLISHED)) {
    ESP_LOGD(TAG, "Sending msg of length %d to [%s] with qos:%d, retain:%d:", 
                  len,
                  topic, 
                  qos, 
                  retain);
    dump_data(TAG, data, len);
    status = (esp_mqtt_client_publish(client_handle, topic, (const char *) data, len, qos, retain) == -1) ? ESP_FAIL : ESP_OK;
  }

  return status;
}

void MQTT::event_handler(void * args, esp_event_base_t base, int32_t event_id, void *event_data)
{
  const esp_mqtt_event_id_t event_type = static_cast<esp_mqtt_event_id_t>(event_id);

  switch (event_type) {
    case MQTT_EVENT_BEFORE_CONNECT: {
      std::lock_guard<std::mutex> state_guard(mutex);
      state = State::INITIALIZED;
      show_state();
      break;
    }

    case MQTT_EVENT_CONNECTED: {
      std::lock_guard<std::mutex> state_guard(mutex);
      state = State::CONNECTED;
      show_state();
      break;
    }

    case MQTT_EVENT_DISCONNECTED: {
      std::lock_guard<std::mutex> state_guard(mutex);
      state = State::DISCONNECTED;
      show_state();
      break;
    }

    case MQTT_EVENT_PUBLISHED: {
      std::lock_guard<std::mutex> state_guard(mutex);
      state = State::ERROR;
      show_state();
      break;
    }

    case MQTT_EVENT_DATA:          // Not implemented
    case MQTT_EVENT_SUBSCRIBED:    // Not implemented
    case MQTT_EVENT_UNSUBSCRIBED:  // Not implemented
    case MQTT_EVENT_ERROR: {
      std::lock_guard<std::mutex> state_guard(mutex);
      state = State::ERROR;
      show_state();
      break;
    } 

    case MQTT_EVENT_ANY:
    case MQTT_EVENT_DELETED:
      break;
  }
}


void MQTT::prepare_for_deep_sleep()
{
  esp_mqtt_client_stop(client_handle);
}

void MQTT::show_state()
{
  const char * msg = nullptr;
  switch (state) {
    case State::DISCONNECTED:
      msg = "DISCONNECTED";
      break;
    case State::CONNECTING:
      msg = "CONNECTING";
      break;
    case State::ERROR:
      msg = "ERROR";
      break;
    case State::CONNECTED:
      msg = "CONNECTED";
      break;
    case State::NOT_INITIALIZED:
      msg = "NOT_INITIALIZED";
      break;
    case State::INITIALIZED:
      msg = "INITIALIZED";
      break;
    case State::PUBLISHED:
      msg = "PUBLISHED";
      break;
    default:
      msg = "UNKNOWN!!!";
  }  

  ESP_LOGD(TAG, "State: %s", msg);
}