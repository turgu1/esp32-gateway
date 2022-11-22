#pragma once

#include <esp_wifi.h>

#include "config.hpp"

// Both MQTT and UDP require STA (Station) mode initialisation.

constexpr const char * WIFI_STA_SSID  = "your_sta_ssid";
constexpr const char * WIFI_STA_PASS  = "your_sta_password";

