#pragma once

#include "global.hpp"

#ifdef CONFIG_EXERCISER_ENABLE_ESP_NOW

#include <freertos/FreeRTOS.h>
#include <esp_now.h>

class ESPNowSender
{
  private:
    static constexpr char const * TAG = "ESP-NOW Sender";

    static bool          abort;

    //static void send_handler(const MacAddr * mac_addr, const uint8_t * incoming_data, int len);

    MacAddr ap_mac_addr;

  public:
    esp_err_t               init(const uint8_t * remote_ap_mac_addr);
    esp_err_t               send(const uint8_t * data, int len);
    void  prepare_for_deep_sleep();
};

#endif