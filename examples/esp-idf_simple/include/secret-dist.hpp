#pragma once

#include <esp_wifi.h>

#include "config.hpp"

// Both UDP and ESP_NOW require STA (Station) mode initialisation.
// The information relate to the Gateway AP configuration

#ifdef UDP_SENDER
  constexpr const char             * WIFI_STA_SSID      = "your_wifi_router_ssid";
  constexpr const char             * WIFI_STA_PASS      = "your_wifi_router_password"; // Can be empty
#endif

#ifdef ESP_NOW_SENDER
  constexpr const char             * WIFI_STA_SSID      = "your_espnow_ap_ssid";
  constexpr const char             * WIFI_STA_PASS      = "your_espnow_ap_password"; // Can be empty
#endif

// If no authentication required, set WIFI_STA_AUTH_MODE to WIFI_AUTH_OPEN
constexpr const wifi_auth_mode_t   WIFI_STA_AUTH_MODE = WIFI_AUTH_WPA2_PSK;
