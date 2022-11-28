#pragma once

#include <lwip/sockets.h>
#include <esp_log.h>
#include <esp_err.h>
#include <esp_now.h>

#if defined(CONFIG_EXERCISER_ENABLE_UDP) && defined(CONFIG_EXERCISER_ENABLE_ESP_NOW)
  #error "You must define only one of CONFIG_EXERCISER_ENABLE_UDP or CONFIG_EXERCISER_ENABLE_ESP_NOW"
#endif

#if !defined(CONFIG_EXERCISER_ENABLE_UDP) && !defined(CONFIG_EXERCISER_ENABLE_ESP_NOW)
  #error "You must define one of CONFIG_EXERCISER_ENABLE_UDP or CONFIG_EXERCISER_ENABLE_ESP_NOW"
#endif

typedef uint8_t MacAddr[6];
typedef char    MacAddrStr[18];

// Define the log level for all classes
// One of (ESP_LOG_<suffix>: NONE, ERROR, WARN, INFO, DEBUG, VERBOSE
//
// To take effect the CONFIG_LOG_MAXIMUM_LEVEL configuration parameter must
// be as high as the targeted log level. This is set in menuconfig:
//     Component config > Log output > Maximum log verbosity
constexpr const esp_log_level_t LOG_LEVEL = ESP_LOG_VERBOSE;

// Log Level

#if defined(CONFIG_EXERCISER_LOG_NONE)
  #define CONFIG_EXERCISER_LOG_LEVEL ESP_LOG_NONE
#elif defined(CONFIG_EXERCISER_LOG_ERROR)
  #define CONFIG_EXERCISER_LOG_LEVEL ESP_LOG_ERROR
#elif defined(CONFIG_EXERCISER_LOG_WARN)
  #define CONFIG_EXERCISER_LOG_LEVEL ESP_LOG_WARN
#elif defined(CONFIG_EXERCISER_LOG_INFO)
  #define CONFIG_EXERCISER_LOG_LEVEL ESP_LOG_INFO
#elif defined(CONFIG_EXERCISER_LOG_DEBUG)
  #define CONFIG_EXERCISER_LOG_LEVEL ESP_LOG_DEBUG
#elif defined(CONFIG_EXERCISER_LOG_VERBOSE)
  #define CONFIG_EXERCISER_LOG_LEVEL ESP_LOG_VERBOSE
#endif

// Router authorization mode

#if defined(CONFIG_EXERCISER_WIFI_STA_WPA3)
  #define WIFI_STA_AUTH_MODE WIFI_AUTH_WPA3_PSK
#elif defined(CONFIG_EXERCISER_WIFI_STA_WPA2)
  #define WIFI_STA_AUTH_MODE WIFI_AUTH_WPA2_PSK
#elif defined(CONFIG_GEXERCISERWIFI_STA_WPA)
  #define WIFI_STA_AUTH_MODE WIFI_AUTH_WPA_PSK
#elif defined(CONFIG_EXERCISER_WIFI_STA_WEP)
  #define WIFI_STA_AUTH_MODE WIFI_AUTH_WEP_PSK
#endif
