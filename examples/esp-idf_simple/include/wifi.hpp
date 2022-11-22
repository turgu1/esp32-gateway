#pragma once

#include <cstring>
#include <mutex>
#include <esp_wifi.h>
#include <esp_event.h>

#include "config.hpp"

class Wifi
{
  public:
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

  private:
    static constexpr char const * TAG = "Wifi Class";

    static State      state;
    static std::mutex mutex;
    static int8_t     rssi;
    static uint32_t   ip;
    static char       ip_cstr[20];

    static void wifi_event_handler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data);
    static void   ip_event_handler(void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data);

    void      set_credentials(const char * ssid, const char * password);
    static esp_err_t  connect(void);
    esp_err_t    retrieve_mac(void);

    uint8_t            mac_addr[6];
    char               mac_addr_cstr[18];

    wifi_init_config_t wifi_init_cfg;
    wifi_config_t      wifi_sta_cfg;
    #ifdef WIFI_AP_ENABLE
      wifi_config_t    wifi_ap_cfg;
    #endif

  public:
    Wifi(void);

    esp_err_t                     init();

    inline const State  &    get_state(void) { return state; }
    inline const uint8_t *     get_mac(void) { return mac_addr; }
    inline const char   * get_mac_cstr(void) { return mac_addr_cstr; }
    inline uint32_t             get_ip(void) { return ip; }
    inline const char   *  get_ip_cstr(void) { return ip_cstr; }
    inline int8_t             get_rssi(void) { return rssi; }
    
    void        prepare_for_deep_sleep();

    static void      show_state();
}; // Wifi class
