#pragma once

#include <cstring>
#include <mutex>
#include <esp_wifi.h>
#include <esp_event.h>

#include "global.hpp"

class Wifi
{
  public:
    #ifdef UDP_SENDER
      enum class State : int8_t {
        NOT_INITIALIZED,
        INITIALIZED,
        READY_TO_CONNECT,
        CONNECTING,
        WAITING_FOR_IP,
        CONNECTED,
        DISCONNECTED,
        ERROR
      };
    #endif

  private:
    static constexpr char const * TAG = "Wifi Class";

    esp_err_t    retrieve_mac(void);

    MacAddr            mac_addr;
    MacAddrStr         mac_addr_cstr;

    #ifdef UDP_SENDER
      static State      state;
      static std::mutex mutex;
      static int8_t     rssi;
      static uint32_t   ip;
      static char       ip_cstr[20];

      static void wifi_event_handler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data);
      static void   ip_event_handler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data);

      static esp_err_t  connect(void);

      wifi_init_config_t wifi_init_cfg;
      wifi_config_t      wifi_sta_cfg;
    #endif

  public:
    Wifi(void);

    esp_err_t                      init();
    void         prepare_for_deep_sleep();

    inline const MacAddr *      get_mac(void) { return &mac_addr; }
    inline const char    * get_mac_cstr(void) { return mac_addr_cstr; }

    #ifdef UDP_SENDER
      inline const State   &    get_state(void) { return state; }
      inline uint32_t              get_ip(void) { return ip; }
      inline const char    *  get_ip_cstr(void) { return ip_cstr; }
      inline int8_t              get_rssi(void) { return rssi; }
      static void              show_state();
    #endif
};