#include <nvs_flash.h>
#include <esp_log.h>

#include "config.hpp"
#include "global.hpp"

#include "app.hpp"

Wifi             App::wifi;
MQTT             App::mqtt;
QueueHandle_t    App::msg_queue = nullptr;
xTaskHandle      App::task      = nullptr;

#ifdef UDP_GATEWAY
  UDPReceiver    App::udp;
#endif

#ifdef ESP_NOW_GATEWAY
  ESPNowReceiver App::esp_now;
#endif

esp_err_t App::init() 
{
  esp_log_level_set(TAG, LOG_LEVEL);

  // Initialize NVS
  esp_err_t status = nvs_flash_init();
  if (status == ESP_ERR_NVS_NO_FREE_PAGES || status == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    if ((status = nvs_flash_erase()) == ESP_OK) status = nvs_flash_init();
  }
  ESP_ERROR_CHECK(status);

  // Default Event Loop initialization

  esp_event_loop_create_default();

  // Wifi initialization

  status = wifi.init();
  ESP_ERROR_CHECK(status);
  wifi.show_state();

  Wifi::State state = wifi.get_state();
  while (!((state == Wifi::State::CONNECTED) || (state == Wifi::State::ERROR))) {
    vTaskDelay(pdMS_TO_TICKS(1000));
    state = wifi.get_state();
  }

  if (state == Wifi::State::ERROR) return ESP_FAIL;

  // MQTT initialization

  status = mqtt.init();
  ESP_ERROR_CHECK(status);
  mqtt.show_state();

  // Messages Queue initialization

  msg_queue = xQueueCreate(MSG_QUEUE_SIZE, sizeof(Message));
  if (msg_queue == nullptr) {
    ESP_LOGE(TAG, "Unable to create Message Queue.");
    return ESP_FAIL;
  }

  #ifdef UDP_GATEWAY
    // UDPReceiver initialization

    status = udp.init(msg_queue);
    ESP_ERROR_CHECK(status);
  #endif

  #ifdef ESP_NOW_GATEWAY
    // UDPReceiver initialization

    status = esp_now.init(msg_queue);
    ESP_ERROR_CHECK(status);
  #endif

  if (xTaskCreate(main_task, "main_task", 4096, nullptr, 5, &task) != pdPASS) {
    ESP_LOGE(TAG, "Unable to create main_task.");
    status = ESP_FAIL;
  }
 
  return status;
}

void App::main_task(void * params)
{
  while (true) {

    Message msg;
    if (xQueueReceive(msg_queue, &msg, pdMS_TO_TICKS(1000))) {
 
      int       len          = msg.length;
      uint8_t * data         = msg.data;
      char    * topic_suffix = nullptr;

      while ((len > 0) && (*data == ' ')) { data++; len--; }
      topic_suffix = (char *) data;
      while ((len > 0) && (*data != ';')) { data++; len--; }

      if (len <= 0) {
        ESP_LOGE(TAG, "Paquet format error. Topic suffix or ';' not found.");
      }
      else {
        *data++ = 0; len--;

        if (len <= 0) {
          ESP_LOGE(TAG, "Paquet format error. Empty packet.");
        }
        else {
          char * topic = (char *) malloc(strlen(topic_suffix) + strlen(MQTT_TOPIC_PREFIX) + 1);
          if (topic == nullptr) {
            ESP_LOGE(TAG, "Unable to allocate memory for topic name.");
          }
          else {
            strcpy(topic, MQTT_TOPIC_PREFIX);
            strcat(topic, topic_suffix);

            while (mqtt.publish(topic, data, len, MQTT_DEFAULT_QOS, MQTT_DEFAULT_RETAIN) == ESP_FAIL) {
              vTaskDelay(pdMS_TO_TICKS(1000));
            }

            free(msg.data);
            free(topic);
          }
        }
      }
    }
  }
}