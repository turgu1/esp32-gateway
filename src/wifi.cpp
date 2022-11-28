#include "esp_log.h"
#include "esp_netif.h"

#include "config.hpp"
#include "wifi.hpp"

// Class variables

std::mutex  Wifi::mutex       = {};
Wifi::State Wifi::state       = State::NOT_INITIALIZED;
uint32_t    Wifi::ip          = 0;
int8_t      Wifi::rssi        = 0;
char        Wifi::ip_cstr[20] = "0.0.0.0";

// Wifi Contructor

Wifi::Wifi(void)
{
  esp_log_level_set(TAG, CONFIG_GATEWAY_LOG_LEVEL);

  mac_addr_cstr[0] = 0;
  wifi_init_cfg    = WIFI_INIT_CONFIG_DEFAULT();
  memset(&wifi_sta_cfg, 0, sizeof(wifi_config_t));

  #ifdef WIFI_AP_ENABLE
    memset(&wifi_ap_cfg, 0, sizeof(wifi_config_t));
  #endif
}

void Wifi::wifi_event_handler(void             * arg, 
                              esp_event_base_t   event_base,
                              int32_t            event_id, 
                              void             * event_data)
{
  if (event_base == WIFI_EVENT) {
    const wifi_event_t event_type = static_cast<wifi_event_t>(event_id);

    switch (event_type) {
      case WIFI_EVENT_STA_START: {
        std::lock_guard<std::mutex> state_guard(mutex);
        state = State::READY_TO_CONNECT;
        show_state();
        connect();
        break;
      }

      case WIFI_EVENT_STA_CONNECTED: {
        std::lock_guard<std::mutex> state_guard(mutex);
        state = State::WAITING_FOR_IP;
        show_state();
        break;
      }

      case WIFI_EVENT_STA_DISCONNECTED: {
        std::lock_guard<std::mutex> state_guard(mutex);
        state = State::DISCONNECTED;
        show_state();
        connect();
        break;
      }

      default:
        break;
    }
  }
}

void Wifi::ip_event_handler(void             * arg, 
                            esp_event_base_t   event_base,
                            int32_t            event_id, 
                            void             * event_data)
{
  if (event_base == IP_EVENT) {
    const ip_event_t event_type = static_cast<ip_event_t>(event_id);

    switch (event_type) {
      case IP_EVENT_STA_GOT_IP: {
        std::lock_guard<std::mutex> state_guard(mutex);
        state = State::CONNECTED;

        wifi_ap_record_t ap;
        esp_err_t status = esp_wifi_sta_get_ap_info(&ap);
        if (status == ESP_OK) {
          rssi = ap.rssi;
          ESP_LOGD(TAG, "RSSI: %d.", rssi);
        }
        else {
          ESP_LOGE(TAG, "Unable to retrieve rssi: %s.", esp_err_to_name(status));
        }

        tcpip_adapter_ip_info_t ipInfo;        
        status = tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);   
        if (status == ESP_OK) {
          ip = ipInfo.ip.addr;
          snprintf(ip_cstr, sizeof(ip_cstr), IPSTR, IP2STR(&ipInfo.ip));
          ESP_LOGD(TAG, "IP Address: %s.", ip_cstr);
        } 
        else {
          ESP_LOGE(TAG, "Unable to retrieve IP Address: %s.", esp_err_to_name(status));
        }    

        show_state();
        break;
      }

      case IP_EVENT_STA_LOST_IP: {
        std::lock_guard<std::mutex> state_guard(mutex);
        if (state != State::DISCONNECTED) {
          state = State::WAITING_FOR_IP;
          show_state();
        }
        break;
      }

      default:
        break;
    }
  }
}

esp_err_t Wifi::connect(void)
{
  esp_err_t status = ESP_OK;

  switch (state) {
    case State::READY_TO_CONNECT:
    case State::DISCONNECTED:
      status = esp_wifi_connect();
      if (status == ESP_OK) {
        state = State::CONNECTING;
        show_state();
      }
      else {
        ESP_LOGE(TAG, "Unable to start wifi connection: %s.", esp_err_to_name(status));
      }
      break;
        
    case State::CONNECTING:
    case State::WAITING_FOR_IP:
    case State::CONNECTED:
      break;

    case State::NOT_INITIALIZED:
    case State::INITIALIZED:
    case State::ERROR:
      ESP_LOGE(TAG, "Connecting at a wrong time.");
      show_state();
      status = ESP_FAIL;
      break;
  }

  return status;
}

esp_err_t Wifi::init()
{
  std::lock_guard<std::mutex> mutx_guard(mutex);

  esp_err_t status = ESP_OK;

  if (mac_addr_cstr[0] == 0) {
    if ((status = retrieve_mac()) != ESP_OK) {
      ESP_LOGE(TAG, "Unable to retrieve Mac Adress: %s", esp_err_to_name(status));
      status = ESP_FAIL;
    }
  }

  ESP_ERROR_CHECK(esp_netif_init());
  if ((status == OK) && (esp_netif_create_default_wifi_sta() == nullptr)) {
    ESP_LOGE(TAG, "Unable to create default wifi STA.");
    status = ESP_FAIL;
  }

  #ifdef WIFI_AP_ENABLE
    if ((status == OK) && (esp_netif_create_default_wifi_ap() == nullptr)) {
      ESP_LOGE(TAG, "Unable to create default wifi AP.");
      status = ESP_FAIL;
    }
  #endif

  if (status == ESP_OK) {
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, nullptr, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, nullptr, nullptr));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    
    #ifdef WIFI_AP_ENABLE
      ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    #else
      ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    #endif

    wifi_sta_cfg.sta.threshold.authmode = WIFI_STA_AUTH_MODE;
    wifi_sta_cfg.sta.pmf_cfg.capable    = true;
    wifi_sta_cfg.sta.pmf_cfg.required   = false;
    memcpy(wifi_sta_cfg.sta.ssid,     CONFIG_GATEWAY_WIFI_STA_SSID, std::min(strlen(CONFIG_GATEWAY_WIFI_STA_SSID),     sizeof(wifi_sta_cfg.sta.ssid)));
    memcpy(wifi_sta_cfg.sta.password, CONFIG_GATEWAY_WIFI_STA_PASS, std::min(strlen(CONFIG_GATEWAY_WIFI_STA_PASS), sizeof(wifi_sta_cfg.sta.password)));

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_cfg));

    #if 0
      memcpy(wifi_ap_cfg.ap.ssid,     WIFI_AP_SSID, std::min(strlen(WIFI_AP_SSID),     sizeof(wifi_ap_cfg.ap.ssid)));
      memcpy(wifi_ap_cfg.ap.password, WIFI_AP_PASS, std::min(strlen(WIFI_AP_PASS), sizeof(wifi_ap_cfg.ap.password)));
      wifi_ap_cfg.ap.authmode       = WIFI_AP_AUTH_MODE;
      wifi_ap_cfg.ap.ssid_len       = strlen(WIFI_AP_SSID);
      wifi_ap_cfg.ap.max_connection = 5;
      wifi_ap_cfg.ap.channel        = CONFIG_GATEWAY_CHANNEL;

      if (strlen(WIFI_AP_PASS) == 0) {
        wifi_ap_cfg.ap.authmode = WIFI_AUTH_OPEN;
      }

      ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_cfg));
    #endif
    
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_start());

    state = State::INITIALIZED;
  }
  
  return status;
}

void Wifi::set_credentials(const char *ssid, const char *password)
{
  esp_wifi_stop();
}

// Get default MAC from API and convert to ASCII HEX
esp_err_t Wifi::retrieve_mac(void)
{
  #ifdef WIFI_AP_ENABLE
    const esp_err_t status = esp_read_mac(mac_addr,  ESP_MAC_WIFI_SOFTAP);
  #else
    const esp_err_t status = esp_read_mac(mac_addr,  ESP_MAC_WIFI_STA);
  #endif

  if (status == ESP_OK) {
    snprintf(mac_addr_cstr, sizeof(mac_addr_cstr), MACSTR, MAC2STR(mac_addr));
  }

  return status;
}


void Wifi::prepare_for_deep_sleep()
{
  esp_err_t status = esp_wifi_stop();
  if (status != ESP_OK) {
    ESP_LOGW(TAG, "Unable to stop Wifi Properly: %s.", esp_err_to_name(status));
  }
}

void Wifi::show_state()
{
  const char * msg = nullptr;
  switch (state) {
    case State::READY_TO_CONNECT:
      msg = "READY_TO_CONNECT";
      break;
    case State::DISCONNECTED:
      msg = "DISCONNECTED";
      break;
    case State::CONNECTING:
      msg = "CONNECTING";
      break;
    case State::WAITING_FOR_IP:
      msg = "WAITING_FOR_IP";
      break;
    case State::ERROR:
      msg = "ERROR";
      break;
    case State::CONNECTED:
      msg = "CONNECTED";
      break;
    case State::NOT_INITIALIZED:
      msg = "NOT_INITIALIZED";
      break;
    case State::INITIALIZED:
      msg = "INITIALIZED";
      break;
    default:
      msg = "UNKNOWN!!!";
  }  

  ESP_LOGD(TAG, "State: %s", msg);
}