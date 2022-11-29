#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_err.h>

#include "config.hpp"
#include "wifi.hpp"
#include "mqtt.hpp"

#ifdef CONFIG_GATEWAY_ENABLE_UDP
  #include "udp_receiver.hpp"
#endif

#ifdef CONFIG_GATEWAY_ENABLE_ESP_NOW
  #include "esp_now_receiver.hpp"
#endif

class App
{
  private:
    static constexpr char const * TAG = "App Class";
    
    static Wifi        wifi;
    static MQTT        mqtt;
    
    #ifdef CONFIG_GATEWAY_ENABLE_UDP
      static UDPReceiver udp;
    #endif

    #ifdef CONFIG_GATEWAY_ENABLE_ESP_NOW
      static ESPNowReceiver esp_now;
    #endif

    static xTaskHandle task;
    static QueueHandle_t msg_queue;
    static void main_task(void * params);

  public:
    esp_err_t init();

};