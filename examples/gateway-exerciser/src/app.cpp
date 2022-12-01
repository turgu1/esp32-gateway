#include <esp_sleep.h>

#include "app.hpp"

xTaskHandle App::task = nullptr;

esp_err_t App::init() 
{
  esp_err_t status = ESP_OK;

  esp_log_level_set(TAG, CONFIG_IOT_LOG_LEVEL);

  esp_reset_reason_t reason = esp_reset_reason();

  if (reason != ESP_RST_DEEPSLEEP) {
    sequence_number = 0;
    error_count = 0;
  }

  ESP_ERROR_CHECK(iot.init(nullptr));

  if (xTaskCreate(main_task, "main_task", 4*4096, nullptr, 5, &task) != pdPASS) {
    ESP_LOGE(TAG, "Unable to create main_task.");
    status = ESP_FAIL;
  }
 
  return status;
}

void App::main_task(void * params)
{
  bool toggle = false;
  char msg[100];

  if (toggle) {
    // JSON Diet version
    sprintf(msg, "%s;{seq:%d,rssi:%d,err:%d}", 
      CONFIG_IOT_TOPIC_NAME, 
      sequence_number++, 
      wifi.get_rssi(), 
      error_count);
  }
  else {
    sprintf(msg, "%s|{\"seq\":%d,\"rssi\":%d,\"err\":%d}", 
      CONFIG_IOT_TOPIC_NAME, 
      sequence_number++, 
      wifi.get_rssi(), 
      error_count);
  }

  toggle = !toggle;

  int len = strlen(msg) + 1; // We send the null char at the end too

  #ifdef CONFIG_IOT_ENABLE_UDP
    udp.send((const uint8_t *) msg, len); // will return after pacquet completly transmitted
  #endif

  #ifdef CONFIG_IOT_ENABLE_ESP_NOW
    xQueueHandle send_handle = esp_now.get_sent_queue_handle();
    esp_now.send((const uint8_t *) msg, len);
    ESPNow::SendEvent evt;
    if (send_handle != nullptr) {
      if (xQueueReceive(send_handle, &evt, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_LOGE(TAG, "No answer after packet sent.");
        error_count++;
      }
      else {
        if (evt.status != ESP_OK) error_count++;
      }
    }
  #endif

  iot.prepare_for_deep_sleep();

  esp_deep_sleep(10*1e6); // never return

  ESP_LOGE(TAG, "Leaving Main Task.");
}
