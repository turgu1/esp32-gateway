#pragma once

#include <lwip/sockets.h>
#include <esp_log.h>

// This is the structure used in a message  xQueue between the receiver and the main app task

struct Message {
  int       length;
  uint8_t * data;
};

typedef uint8_t MacAddr[6];
typedef uint8_t LmkKey[17];

struct EncryptedDevice {
  MacAddr   mac;
  LmkKey  * key;
};

// ----

#if !defined(CONFIG_GATEWAY_ENABLE_UDP) && !defined(CONFIG_GATEWAY_ENABLE_ESP_NOW)
  #error "You must define at least one of CONFIG_GATEWAY_ENABLE_UDP or CONFIG_GATEWAY_ENABLE_ESP_NOW" through menuconfig.
#endif

#ifdef CONFIG_GATEWAY_ENABLE_ESP_NOW

  // Pre-defined encrypted devices.

  // The following are the maximum count allowed by ESP-IDF.
  constexpr const int MAX_LMK_KEY_COUNT           = 6;
  constexpr const int MAX_ENCRYPTED_DEVICES_COUNT = 6;

  extern LmkKey lmk_keys[MAX_LMK_KEY_COUNT];
  extern EncryptedDevice encrypted_devices[MAX_ENCRYPTED_DEVICES_COUNT];

  #define WIFI_AP_ENABLE 1

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
