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

  ESP_ERROR_CHECK(nvs_mgr.init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(wifi.init());

  #ifdef CONFIG_IOT_ENABLE_UDP
    wifi.show_state();

    Wifi::State state = wifi.get_state();
    while (!((state == Wifi::State::CONNECTED) || (state == Wifi::State::ERROR)) || (state == Wifi::State::WAITING_FOR_IP)) {
      vTaskDelay(pdMS_TO_TICKS(500));
      state = wifi.get_state();
    }

    if (state == Wifi::State::ERROR) return ESP_FAIL;
  #endif

  #ifdef CONFIG_IOT_ENABLE_UDP
    // UDPSender initialization

    status = udp.init();
    ESP_ERROR_CHECK(status);
  #endif

  #ifdef CONFIG_IOT_ENABLE_ESP_NOW
    // EspNowSender initialization

    status = esp_now.init();
    ESP_ERROR_CHECK(status);
  #endif

  ESP_LOGD(TAG, "Creating Main Task.");

  if (xTaskCreate(main_task, "main_task", 4*4096, nullptr, 5, &task) != pdPASS) {
    ESP_LOGE(TAG, "Unable to create main_task.");
    status = ESP_FAIL;
  }
 
  return status;
}

void App::main_task(void * params)
{
  ESP_LOGD(TAG, "Start of Main Task...");

  bool toggle = false;
  char msg[100];

  if (toggle) {
    // JSON Diet version
    sprintf(msg, "%s;{seq:%d,rssi:%d,err:%d}", 
      CONFIG_IOT_TOPIC_SUFFIX, 
      sequence_number++, 
      wifi.get_rssi(), 
      error_count);
  }
  else {
    sprintf(msg, "%s|{\"seq\":%d,\"rssi\":%d,\"err\":%d}", 
      CONFIG_IOT_TOPIC_SUFFIX, 
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
    ESPNowSender::SendEvent evt;
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

  #ifdef CONFIG_IOT_ENABLE_UDP
    udp.prepare_for_deep_sleep();
  #endif

  #ifdef CONFIG_IOT_ENABLE_ESP_NOW
    esp_now.prepare_for_deep_sleep();
  #endif

  wifi.prepare_for_deep_sleep();

  esp_deep_sleep(10*1e6); // never return

  ESP_LOGE(TAG, "Leaving Main Task.");
}
