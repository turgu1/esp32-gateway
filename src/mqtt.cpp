#include <esp_log.h>

#include "mqtt.hpp"
#include "config.hpp"

// Class variables

std::mutex         MQTT::mutex           = {};
MQTT::State        MQTT::state           = State::NOT_INITIALIZED;

esp_err_t MQTT::init(const char * username, const char * id, const char * password)
{
  set_credentials(username, id, password);
  
  mqtt_cfg.uri       = MQTT_SERVER_URI;
  mqtt_cfg.username  = mqtt_username;
  mqtt_cfg.password  = mqtt_password;
  mqtt_cfg.client_id = mqtt_client_id;

  client_handle = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_register_event(client_handle, MQTT_EVENT_ANY, event_handler, nullptr);
  
  esp_mqtt_client_start(client_handle);

  return ESP_OK;
}

void MQTT::set_credentials(const char * username, const char * id, const char * password)
{
  strncpy(mqtt_username,  username, sizeof(mqtt_username));
  strncpy(mqtt_client_id, id,       sizeof(mqtt_client_id));
  strncpy(mqtt_password,  password, sizeof(mqtt_password));
}

void MQTT::publish(const char * topic, const char * data, int len, int qos, int retain) 
{
  if ((state == State::CONNECTED) || (state == State::PUBLISHED)) {
    ESP_LOGI(TAG, "Sending [%s]: [%-*s] with qos:%d, retain:%d.", 
                  topic, 
                  (data[len-1] == 0) ? len - 1 : len, 
                  data, 
                  qos, 
                  retain);
    esp_mqtt_client_publish(client_handle, topic, data, len, qos, retain);
  }
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

  ESP_LOGI(TAG, "State: %s", msg);
}