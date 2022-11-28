#include <nvs_flash.h>

#include "app.hpp"

xTaskHandle App::task = nullptr;

#ifdef CONFIG_EXERCISER_ENABLE_UDP
  UDPSender    App::udp;
#endif

#ifdef CONFIG_EXERCISER_ENABLE_ESP_NOW
  ESPNowSender App::esp_now;
#endif

esp_err_t App::init() 
{
  esp_err_t status = ESP_OK;

  esp_log_level_set(TAG, CONFIG_EXERCISER_LOG_LEVEL);

  // Initialize NVS
  status = nvs_flash_init();
  if (status == ESP_ERR_NVS_NO_FREE_PAGES || status == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    if ((status = nvs_flash_erase()) == ESP_OK) status = nvs_flash_init();
  }
  ESP_ERROR_CHECK(status);

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ESP_ERROR_CHECK(wifi.init());

  #ifdef CONFIG_EXERCISER_ENABLE_UDP
    wifi.show_state();

    Wifi::State state = wifi.get_state();
    while (!((state == Wifi::State::CONNECTED) || (state == Wifi::State::ERROR)) || (state == Wifi::State::WAITING_FOR_IP)) {
      vTaskDelay(pdMS_TO_TICKS(500));
      state = wifi.get_state();
    }

    if (state == Wifi::State::ERROR) return ESP_FAIL;
  #endif

  #ifdef CONFIG_EXERCISER_ENABLE_UDP
    // UDPSender initialization

    status = udp.init();
    ESP_ERROR_CHECK(status);
  #endif

  #ifdef CONFIG_EXERCISER_ENABLE_ESP_NOW
    // EspNowSender initialization

    ESP_LOGD(TAG, "Remote MAC addr: " MACSTR, MAC2STR(GATEWAY_MAC_ADDR));
    status = esp_now.init(GATEWAY_MAC_ADDR);
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

  static int toggle = 0;
  const char * msg;

  while (true) {

    ESP_LOGD(TAG, "Loop...");

    if (toggle == 0) {
      // JSON packet being sent
      msg = "toto|{\"string1\":[23,{\"string2\":\"string3\"}]}";
    }
    else {
      // JSON Lite packet being sent
      msg = "toto;{string1:[23,{string2:string3}]}";
    }

    toggle ^= 1;

    int len = strlen(msg) + 1; // We send the null char at the end too

    #ifdef CONFIG_EXERCISER_ENABLE_UDP
      udp.send((const uint8_t *) msg, len);
    #endif

    #ifdef CONFIG_EXERCISER_ENABLE_ESP_NOW
      esp_now.send((const uint8_t *) msg, len);
    #endif

    vTaskDelay(pdMS_TO_TICKS(1000));
  }


  ESP_LOGE(TAG, "Leaving Main Task.");
}
