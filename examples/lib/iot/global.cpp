#define __GLOBAL__ 1
#include "config.hpp"

#include "global.hpp"

Wifi   wifi;
NVSMgr nvs_mgr;

#ifdef CONFIG_IOT_ENABLE_UDP
  UDP udp;
#endif

#ifdef CONFIG_IOT_ENABLE_ESP_NOW
  ESPNow esp_now;
#endif

RTC_NOINIT_ATTR uint32_t sequence_number;
RTC_NOINIT_ATTR uint32_t error_count;

