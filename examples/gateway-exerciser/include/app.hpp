#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "global.hpp"
#include "wifi.hpp"

#ifdef CONFIG_EXERCISER_ENABLE_UDP
  #include "udp_sender.hpp"
#endif

#ifdef CONFIG_EXERCISER_ENABLE_ESP_NOW
  #include "esp_now_sender.hpp"
#endif

class App
{
  private:
    static constexpr char const * TAG = "Main App";

    static xTaskHandle task;
    static void main_task(void * params);
    
    Wifi wifi;

    #ifdef CONFIG_EXERCISER_ENABLE_UDP
      static UDPSender udp;
    #endif

    #ifdef CONFIG_EXERCISER_ENABLE_ESP_NOW
      static ESPNowSender esp_now;
    #endif

  public:
    esp_err_t init();
};