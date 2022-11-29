#pragma once

#include "config.hpp"

#ifdef CONFIG_GATEWAY_ENABLE_ESP_NOW

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <esp_check.h>
#include <esp_now.h>

class ESPNowReceiver
{
  private:
    static constexpr char const * TAG = "ESP-NOW Receiver";

    static QueueHandle_t msg_queue;
    static bool          abort;

    static void receive_handler(const uint8_t * mac_addr, const uint8_t * incoming_data, int len);

  public:
    esp_err_t               init(QueueHandle_t queue);
    void  prepare_for_deep_sleep();
};

#endif