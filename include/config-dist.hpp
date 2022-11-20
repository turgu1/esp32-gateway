#pragma once

#include <lwip/sockets.h>

// Only one of UDP_GATEWAY or ESP_NOW_GATEWAY must be defined below.

#define UDP_GATEWAY 1
//#define ESP_NOW_GATEWAY 1

#if defined(UDP_GATEWAY) && defined(ESP_NOW_GATEWAY)
  #error "You must define only one of UDP_GATEWAY or ESP_NOW_GATEWAY"
#endif

#if !defined(UDP_GATEWAY) && !defined(ESP_NOW_GATEWAY)
  #error "You must define one of UDP_GATEWAY or ESP_NOW_GATEWAY"
#endif

#ifdef UDP_GATEWAY
  // Port number for receiving UDP packets
  constexpr const in_port_t UDP_IN_PORT = 3333;
#endif

constexpr const char * MQTT_SERVER_URI   = "mqtt://0.0.0.0:1883";
constexpr const char * MQTT_TOPIC_PREFIX = "iot/";

constexpr const int MSG_QUEUE_SIZE     =  20;
constexpr const int MAX_MSG_SIZE       = 250;