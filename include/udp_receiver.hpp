#pragma once

#include "config.hpp"

#ifdef CONFIG_GATEWAY_ENABLE_UDP

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <cinttypes>
#include <esp_check.h>
#include <lwip/sockets.h>

class UDPReceiver
{
  private:
    static constexpr char const * TAG = "UDPReceiver Class";

    static xTaskHandle   task;
    static QueueHandle_t msg_queue;
    static bool          abort;

    static void receive_server(void * params);

  public:
    esp_err_t                   init(QueueHandle_t queue);
    void      prepare_for_deep_sleep();
};

#endif