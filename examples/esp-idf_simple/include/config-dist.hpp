#pragma once

#include <lwip/sockets.h>
#include <esp_log.h>

#define UDP_SENDER     1
//#define ESP_NOW_SENDER 1

#if defined(UDP_SENDER) && defined(ESP_NOW_SENDER)
  #error "You must define only one of UDP_SENDER or ESP_NOW_SENDER"
#endif

#if !defined(UDP_SENDER) && !defined(ESP_NOW_SENDER)
  #error "You must define one of UDP_SENDER or ESP_NOW_SENDER"
#endif

#ifdef UDP_SENDER
  // All UDP devices must use the following port number to transmit packets
  // to the gateway
  constexpr const in_port_t    UDP_OUT_PORT     = 3333;
  constexpr const char *    GATEWAY_IP_ADDR     = "0.0.0.0";
  // The maximum size of packets transmitted by UDP sensors. ESP-NOW limit is 250 bytes.
  constexpr const int       UDP_MAX_PKT_SIZE    = 250;
#endif

#ifdef ESP_NOW_SENDER
  // All ESP-NOW devices must use the following channel number to transmit
  // packets to the gateway
  constexpr const int ESP_NOW_AP_CHANNEL      =  1;
  constexpr const int ESP_NOW_MAX_CONNECTIONS = 20;
#endif

// Define the log level for all classes
// One of (ESP_LOG_<suffix>: NONE, ERROR, WARN, INFO, DEBUG, VERBOSE
//
// To take effect the CONFIG_LOG_MAXIMUM_LEVEL configuration parameter must
// be as high as the targeted log level. This is set in menuconfig:
//     Component config > Log output > Maximum log verbosity
constexpr const esp_log_level_t LOG_LEVEL = ESP_LOG_INFO;