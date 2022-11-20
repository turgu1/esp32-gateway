#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_err.h>

#include "config.hpp"
#include "wifi.hpp"
#include "mqtt.hpp"

#ifdef UDP_GATEWAY
  #include "udp_receiver.hpp"
#endif

class App
{
  private:
    static constexpr char const * TAG = "Main App";
    
    static Wifi        wifi;
    static MQTT        mqtt;
    
    #ifdef UDP_GATEWAY
      static UDPReceiver udp;
    #endif

    static xTaskHandle task;
    static QueueHandle_t msg_queue;
    static void main_task(void * params);

  public:
    esp_err_t init();

};