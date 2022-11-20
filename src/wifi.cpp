#include "esp_log.h"
#include "esp_netif.h"

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
  mac_addr_cstr[0] = 0;
  wifi_init_cfg    = WIFI_INIT_CONFIG_DEFAULT();
  wifi_cfg         = {};
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
        State::WAITING_FOR_IP;
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
          ESP_LOGI(TAG, "RSSI: %d.", rssi);
        }
        else {
          ESP_LOGE(TAG, "Unable to retrieve rssi: %s.", esp_err_to_name(status));
        }

        tcpip_adapter_ip_info_t ipInfo;        
        status = tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);   
        if (status == ESP_OK) {
          ip = ipInfo.ip.addr;
          snprintf(ip_cstr, 20, IPSTR, IP2STR(&ipInfo.ip));
          ESP_LOGI(TAG, "IP Address: %s.", ip_cstr);
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

esp_err_t Wifi::init(const char * ssid, const char * password)
{
  std::lock_guard<std::mutex> mutx_guard(mutex);

  esp_err_t status = ESP_OK;

  set_credentials(ssid, password);

  if (mac_addr_cstr[0] == 0) {
    if ((status = retrieve_mac()) != ESP_OK) {
      ESP_LOGE(TAG, "Unable to retrieve Mac Adress: %s", esp_err_to_name(status));
      return ESP_FAIL;
    }
  }

  if (state == State::NOT_INITIALIZED) {
    status = esp_netif_init();
    if (status == ESP_OK) {
      const esp_netif_t * const p_netif = esp_netif_create_default_wifi_sta();

      if (p_netif == nullptr) {
        ESP_LOGE(TAG, "Unable to create default wifi STA.");
        status = ESP_FAIL;
      }
    }

    if (status == ESP_OK) {
      if ((status = esp_wifi_init(&wifi_init_cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "Unable to initialize wifi: %s.", esp_err_to_name(status));
      }
    }

    if (status == ESP_OK) {
      status = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, nullptr, nullptr);
      if (status != ESP_OK) {
        ESP_LOGE(TAG, "Unable to register Wifi event hanfler instance: %s", esp_err_to_name(status));
      }
    }

    if (status == ESP_OK) {
      status = esp_event_handler_instance_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, nullptr, nullptr);
      if (status != ESP_OK) {
        ESP_LOGE(TAG, "Unable to register IP event hanfler instance: %s", esp_err_to_name(status));
      }
    }

    if (status == ESP_OK) {
      if ((status = esp_wifi_set_mode(WIFI_MODE_STA)) != ESP_OK) {
        ESP_LOGE(TAG, "Unable to set wifi mode to STA: %s", esp_err_to_name(status));
      }
    }

    if (status == ESP_OK) {
      wifi_cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
      wifi_cfg.sta.pmf_cfg.capable    = true;
      wifi_cfg.sta.pmf_cfg.required   = false;

      if ((status = esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg)) != ESP_OK) {
        ESP_LOGE(TAG, "Unable to set config: %s", esp_err_to_name(status));
      }
    }

    if (status == ESP_OK) {
      if ((status = esp_wifi_start()) != ESP_OK) {
        ESP_LOGE(TAG, "Unable to start wifi: %s.", esp_err_to_name(status));
      }
    }

    if (status == ESP_OK) {
      state = State::INITIALIZED;
    }
  }
  else if (state == State::ERROR) {
    state = State::NOT_INITIALIZED;
  }

  return status;
}

void Wifi::set_credentials(const char *ssid, const char *password)
{
  memcpy(wifi_cfg.sta.ssid,     ssid,     std::min(strlen(ssid),     sizeof(wifi_cfg.sta.ssid)));
  memcpy(wifi_cfg.sta.password, password, std::min(strlen(password), sizeof(wifi_cfg.sta.password)));
}

// Get default MAC from API and convert to ASCII HEX
esp_err_t Wifi::retrieve_mac(void)
{
  const esp_err_t status = esp_efuse_mac_get_default(mac_addr);

  if (status == ESP_OK) {
    snprintf(mac_addr_cstr, sizeof(mac_addr_cstr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac_addr[0],
             mac_addr[1],
             mac_addr[2],
             mac_addr[3],
             mac_addr[4],
             mac_addr[5]);
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

  ESP_LOGI(TAG, "State: %s", msg);
}