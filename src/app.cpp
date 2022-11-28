#include <nvs_flash.h>
#include <esp_log.h>

#include "global.hpp"
#include "utils.hpp"
#include "app.hpp"

Wifi             App::wifi;
MQTT             App::mqtt;
QueueHandle_t    App::msg_queue = nullptr;
xTaskHandle      App::task      = nullptr;

#ifdef CONFIG_GATEWAY_ENABLE_UDP
  UDPReceiver    App::udp;
#endif

#ifdef CONFIG_GATEWAY_ENABLE_ESP_NOW
  ESPNowReceiver App::esp_now;
#endif

esp_err_t App::init() 
{
  esp_log_level_set(TAG, CONFIG_GATEWAY_LOG_LEVEL);

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

  msg_queue = xQueueCreate(CONFIG_GATEWAY_MSG_QUEUE_SIZE, sizeof(Message));
  if (msg_queue == nullptr) {
    ESP_LOGE(TAG, "Unable to create Message Queue.");
    return ESP_FAIL;
  }

  #ifdef CONFIG_GATEWAY_ENABLE_UDP
    // UDPReceiver initialization

    status = udp.init(msg_queue);
    ESP_ERROR_CHECK(status);
  #endif

  #ifdef CONFIG_GATEWAY_ENABLE_ESP_NOW
    // EspNowReceiver initialization

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
      while ((len > 0) && !((*data == ';') || (*data == '|'))) { data++; len--; }

      if (len <= 0) {
        ESP_LOGE(TAG, "Paquet format error. Topic suffix or ';' or '|' not found.");
      }
      else {
        uint8_t * d;
        int d_len;
        static uint8_t buff[2048];
        if (*data == ';') {
          *data++ = 0; len--;
          if ((d_len = to_json(data, len, (char *) buff, sizeof(buff))) > 0) {
            d = buff;
          }
          else {
            d     = data;
            d_len = len;
          }
        }
        else {
          *data++ = 0; len--;
          d     = data;
          d_len = len;
        }

        if (len <= 0) {
          ESP_LOGE(TAG, "Paquet format error. Empty packet.");
        }
        else {
          char * topic = (char *) malloc(strlen(topic_suffix) + strlen(CONFIG_GATEWAY_MQTT_TOPIC_PREFIX) + 1);
          if (topic == nullptr) {
            ESP_LOGE(TAG, "Unable to allocate memory for topic name.");
          }
          else {
            strcpy(topic, CONFIG_GATEWAY_MQTT_TOPIC_PREFIX);
            strcat(topic, topic_suffix);

            while (mqtt.publish(topic, d, d_len, CONFIG_GATEWAY_MQTT_DEFAULT_QOS, CONFIG_GATEWAY_MQTT_DEFAULT_RETAIN) == ESP_FAIL) {
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