#pragma once

#include "config.hpp"
#ifdef ESP_NOW_SENDER

#include <freertos/FreeRTOS.h>
#include <esp_check.h>
#include <esp_now.h>

class ESPNowSender
{
  private:
    static constexpr char const * TAG = "ESP-NOW Sender";

    static bool          abort;

    static void send_handler(const uint8_t * mac_addr, const uint8_t * incoming_data, int len);

  public:
    esp_err_t               init();
    void  prepare_for_deep_sleep();
};

#endif