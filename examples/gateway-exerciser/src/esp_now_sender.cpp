#include "global.hpp"

#ifdef ESP_NOW_SENDER

#include <cstring>
#include <esp_crc.h>
#include <esp_wifi.h>

#include "utils.hpp"
#include "esp_now_sender.hpp"

bool ESPNowSender::abort = false;

esp_err_t ESPNowSender::init(const uint8_t * remote_ap_mac_addr)
{
  esp_err_t status;

  esp_log_level_set(TAG, LOG_LEVEL);

  memcpy(ap_mac_addr, remote_ap_mac_addr, sizeof(ap_mac_addr));

  ESP_ERROR_CHECK(esp_now_init());

  // ESP_ERROR_CHECK(esp_now_register_recv_cb(send_handler));
 
  ESP_ERROR_CHECK(status = esp_now_set_pmk((const uint8_t *) ESP_NOW_PMK));

  esp_now_peer_info_t * peer = (esp_now_peer_info_t *) malloc(sizeof(esp_now_peer_info_t));
  if (peer == NULL) {
      ESP_LOGE(TAG, "Malloc peer information fail");
//      vSemaphoreDelete(s_example_espnow_queue);
      esp_now_deinit();
      return ESP_FAIL;
  }

  memset(peer, 0, sizeof(esp_now_peer_info_t));
  memcpy(peer->peer_addr, ap_mac_addr, 6);
  memcpy(peer->lmk, ESP_NOW_LMK, ESP_NOW_KEY_LEN);

  peer->channel   = WIFI_CHANNEL;
  peer->ifidx     = (wifi_interface_t) ESP_IF_WIFI_STA;
  peer->encrypt   = false;

  ESP_LOGD(TAG, "AP Peer MAC address: " MACSTR, MAC2STR(peer->peer_addr));

  ESP_LOGD(TAG, "esp_now_add_peer()...");

  ESP_ERROR_CHECK(status = esp_now_add_peer(peer));
  
  free(peer);

  ESP_ERROR_CHECK(esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE));

  ESP_LOGD(TAG, "End of ESPNowSender.init().");

  return status;
}

#if 0
void ESPNowSender::send_handler(const MacAddr * mac_addr, const uint8_t * incoming_data, int len)
{
  Message msg;

  ESP_LOGD(TAG, "Received %d bytes from " MACSTR ":", len, MAC2STR(mac_addr));
  dump_data(TAG, incoming_data, len);

  msg.data = (uint8_t *) malloc(len);
  if (msg.data == nullptr) {
    ESP_LOGE(TAG, "Unable to allocation memory for message data.");
  }
  else if (msg_queue != nullptr) {
    memcpy(msg.data, incoming_data, len);
    msg.length = len;
    if (xQueueSend(msg_queue, &msg, 0) != pdTRUE) {
      ESP_LOGW(TAG, "Message Queue is full, message is lost.");
    }
  }
}
#endif

esp_err_t ESPNowSender::send(const uint8_t * data, int len)
{
  esp_err_t status;
  struct {
    uint16_t crc;
    char data[MAX_PKT_SIZE];
  } __attribute__((packed)) pkt;

  if (len > MAX_PKT_SIZE) {
    ESP_LOGE(TAG, "Cannot send data of length %d, too long. Max is %d.", len, MAX_PKT_SIZE);
    status = ESP_FAIL;
  }
  else {
    memcpy(pkt.data, data, len);
    pkt.crc = esp_crc16_le(UINT16_MAX, (uint8_t *)(pkt.data), len);

    if ((status = esp_now_send(ap_mac_addr, (const uint8_t *) &pkt, len+2)) != ESP_OK) {
      ESP_LOGE(TAG, "Unable to send ESP-NOW packet: %s.", esp_err_to_name(status));
      uint8_t primary_channel;
      wifi_second_chan_t secondary_channel;
      esp_wifi_get_channel(&primary_channel, &secondary_channel);
      ESP_LOGE(TAG, "Wifi channels: %d %d.", primary_channel, secondary_channel);
    }
  }

  return status;
}

void ESPNowSender::prepare_for_deep_sleep()
{
  esp_now_deinit();
}

#endif