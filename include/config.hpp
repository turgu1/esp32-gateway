#pragma once

#include <lwip/sockets.h>
#include <esp_log.h>

#include "global.hpp"

//#define UDP_GATEWAY     1
//#define ESP_NOW_GATEWAY 1

#if !defined(CONFIG_GATEWAY_ENABLE_UDP) && !defined(CONFIG_GATEWAY_ENABLE_ESP_NOW)
  #error "You must define at least one of CONFIG_GATEWAY_ENABLE_UDP or CONFIG_GATEWAY_ENABLE_ESP_NOW" through menuconfig.
#endif

#ifdef CONFIG_GATEWAY_ENABLE_ESP_NOW

  #define WIFI_AP_ENABLE 1

  // Pre-defined encrypted devices.

  // The following are the number of entries in the tables as defined in the src/config.cpp file.
  constexpr const int LMK_KEY_COUNT           = 1;
  constexpr const int ENCRYPTED_DEVICES_COUNT = 0;

  // The following are the maximum count allowed by ESP-IDF.
  constexpr const int MAX_LMK_KEY_COUNT           = 6;
  constexpr const int MAX_ENCRYPTED_DEVICES_COUNT = 6;

  extern LmkKey lmk_keys[MAX_LMK_KEY_COUNT];
  extern EncryptedDevice encrypted_devices[MAX_ENCRYPTED_DEVICES_COUNT];

  // Log Level

  #if defined(CONFIG_GATEWAY_LOG_NONE)
    #define CONFIG_GATEWAY_LOG_LEVEL ESP_LOG_NONE
  #elif defined(CONFIG_GATEWAY_LOG_ERROR)
    #define CONFIG_GATEWAY_LOG_LEVEL ESP_LOG_ERROR
  #elif defined(CONFIG_GATEWAY_LOG_WARN)
    #define CONFIG_GATEWAY_LOG_LEVEL ESP_LOG_WARN
  #elif defined(CONFIG_GATEWAY_LOG_INFO)
    #define CONFIG_GATEWAY_LOG_LEVEL ESP_LOG_INFO
  #elif defined(CONFIG_GATEWAY_LOG_DEBUG)
    #define CONFIG_GATEWAY_LOG_LEVEL ESP_LOG_DEBUG
  #elif defined(CONFIG_GATEWAY_LOG_VERBOSE)
    #define CONFIG_GATEWAY_LOG_LEVEL ESP_LOG_VERBOSE
  #endif

  // Router authorization mode

  #if defined(CONFIG_GATEWAY_WIFI_STA_WPA3)
    #define WIFI_STA_AUTH_MODE WIFI_AUTH_WPA3_PSK
  #elif defined(CONFIG_GATEWAY_WIFI_STA_WPA2)
    #define WIFI_STA_AUTH_MODE WIFI_AUTH_WPA2_PSK
  #elif defined(CONFIG_GATEWAY_WIFI_STA_WPA)
    #define WIFI_STA_AUTH_MODE WIFI_AUTH_WPA_PSK
  #elif defined(CONFIG_GATEWAY_WIFI_STA_WEP)
    #define WIFI_STA_AUTH_MODE WIFI_AUTH_WEP_PSK
  #endif

  // AP Authorization Mode

  #if defined(CONFIG_GATEWAY_WIFI_AP_WPA3)
    #define WIFI_AP_AUTH_MODE WIFI_AUTH_WPA3_PSK
  #elif defined(CONFIG_GATEWAY_WIFI_AP_WPA2)
    #define WIFI_AP_AUTH_MODE WIFI_AUTH_WPA2_PSK
  #elif defined(CONFIG_GATEWAY_WIFI_AP_WPA)
    #define WIFI_AP_AUTH_MODE WIFI_AUTH_WPA_PSK
  #elif defined(CONFIG_GATEWAY_WIFI_AP_WEP)
    #define WIFI_AP_AUTH_MODE WIFI_AUTH_WEP_PSK
  #endif

#endif