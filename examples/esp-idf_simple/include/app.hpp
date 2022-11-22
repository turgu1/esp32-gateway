#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_err.h>

#include "config.hpp"
#include "wifi.hpp"

#ifdef UDP_SENDER
  #include "udp_sender.hpp"
#endif

#ifdef ESP_NOW_SENDER
  #include "esp_now_sender.hpp"
#endif

class App
{
  private:
    static constexpr char const * TAG = "Main App";
    
    static Wifi        wifi;
    
    #ifdef UDP_SENDER
      static UDPSender udp;
    #endif

    #ifdef ESP_NOW_SENDER
      static ESPNowSender esp_now;
    #endif

    static xTaskHandle task;
    static void main_task(void * params);

  public:
    esp_err_t init();

};