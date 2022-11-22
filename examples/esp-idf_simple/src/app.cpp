#include <nvs_flash.h>
#include <esp_log.h>

#include "config.hpp"
#include "utils.hpp"
#include "app.hpp"

Wifi             App::wifi;
xTaskHandle      App::task      = nullptr;

#ifdef UDP_SENDER
  UDPSender    App::udp;
#endif

#ifdef ESP_NOW_SENDER
  ESPNowSender App::esp_now;
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

  #ifdef UDP_SENDER
    // UDPSender initialization

    status = udp.init();
    ESP_ERROR_CHECK(status);
  #endif

  #ifdef ESP_NOW_SENDER
    // UDPSender initialization

    status = esp_now.init();
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
  static int toggle = 0;

  while (true) {

    const char * msg;

    if (toggle == 0) {
      // JSON packet being sent
      msg = "toto|{\"string1\":[23,{\"string2\":\"string3\"}]}";
    }
    else {
      // JSON Lite packet being sent
      msg = "toto;{string1:[23,{string2:string3}]}";
    }

    toggle ^= 1;

    #ifdef UDP_SENDER
      udp.send((const uint8_t *)msg, strlen(msg));
    #endif

    #ifdef ESP_NOW_SENDER
      esp_now.send((const uint8_t *)msg, strlen(msg));
    #endif

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}