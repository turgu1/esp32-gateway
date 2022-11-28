#define __GLOBAL__ 1
#include "config.hpp"

#include "global.hpp"

Wifi   wifi;
NVSMgr nvs_mgr;

#ifdef CONFIG_EXERCISER_ENABLE_UDP
  UDPSender udp;
#endif

#ifdef CONFIG_EXERCISER_ENABLE_ESP_NOW
  ESPNowSender esp_now;
#endif

RTC_NOINIT_ATTR uint32_t sequence_number;
RTC_NOINIT_ATTR uint32_t error_count;

