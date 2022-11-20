#pragma once

#include <esp_wifi.h>

#include "config.hpp"

// Both MQTT and UDP require STA (Station) mode initialisation.

constexpr const char * WIFI_STA_SSID  = "your_sta_ssid";
constexpr const char * WIFI_STA_PASS  = "your_sta_password";

#ifdef ESP_NOW_GATEWAY
  // The ESP-NOW requires AP (Access Point) mode initialisation
  
  constexpr const char * WIFI_AP_SSID = "your_ap_ssid";
  constexpr const char * WIFI_AP_PASS = "your_ap_password";   // Can be a 0 length string
  constexpr const int    AP_AUTH_MODE = WIFI_AUTH_WPA_WPA2_PSK;
#endif

constexpr const char * MQTT_USERNAME  = "your_mqtt_username";
constexpr const char * MQTT_CLIENT_ID = "your_mqtt_client_id";
constexpr const char * MQTT_PASS      = "your_mqtt_password";
