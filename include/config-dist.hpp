#pragma once

#include <lwip/sockets.h>

#define UDP_GATEWAY 1
#define ESP_NOW_GATEWAY 1

// #if defined(UDP_GATEWAY) && defined(ESP_NOW_GATEWAY)
//   #error "You must define only one of UDP_GATEWAY or ESP_NOW_GATEWAY"
// #endif

#if !defined(UDP_GATEWAY) && !defined(ESP_NOW_GATEWAY)
  #error "You must define one of UDP_GATEWAY or ESP_NOW_GATEWAY"
#endif

#ifdef UDP_GATEWAY
  // All UDP devices must use the following port number to transmit packets
  // to the gateway
  constexpr const in_port_t UDP_IN_PORT = 3333;
#endif

#ifdef ESP_NOW_GATEWAY
  // All ESP-NOW devices must use the following channel number to transmit
  // packets to the gateway
  constexpr const int ESP_NOW_AP_CHANNEL = 1;
  constexpr const int ESP_NOW_MAX_CONNECTIONS = 20;
#endif

constexpr const char * MQTT_SERVER_URI = "mqtt://0.0.0.0:1883";
constexpr const char * TOPIC_PREFIX    = "iot/";
constexpr const int    DEFAULT_QOS     = 0;
constexpr const int    DEFAULT_RETAIN  = 0;

// The maximum number of packets in the queue waiting for transmission through MQTT
constexpr const int    MSG_QUEUE_SIZE  =  20;

// The maximu size of packets transmitted by sensors. ESP-NOW limit is 250 bytes.
constexpr const int    MAX_MSG_SIZE    = 250;