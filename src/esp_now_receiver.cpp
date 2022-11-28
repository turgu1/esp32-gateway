#include "config.hpp"
#ifdef CONFIG_GATEWAY_ENABLE_ESP_NOW

#include <cstring>
#include <esp_crc.h>
#include <esp_wifi.h>

#include "utils.hpp"
#include "esp_now_receiver.hpp"

QueueHandle_t ESPNowReceiver::msg_queue = nullptr;
bool          ESPNowReceiver::abort     = false;

esp_err_t ESPNowReceiver::init(QueueHandle_t queue)
{
  esp_err_t status = ESP_OK;

  msg_queue = queue;

  esp_log_level_set(TAG, CONFIG_GATEWAY_LOG_LEVEL);

  ESP_ERROR_CHECK(esp_now_init());
  ESP_ERROR_CHECK(status = esp_now_set_pmk((const uint8_t *) CONFIG_GATEWAY_ESPNOW_PMK));
  ESP_ERROR_CHECK(esp_now_register_recv_cb(receive_handler));

  ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_GATEWAY_CHANNEL, WIFI_SECOND_CHAN_NONE));

  if (ENCRYPTED_DEVICES_COUNT > 0) {
    ESP_LOGI(TAG, "----- Encrypted devices: -----");
    for (int i = 0; i < ENCRYPTED_DEVICES_COUNT; i++) {
      esp_now_peer_info_t peer;
      memset(&peer, 0, sizeof(esp_now_peer_info_t));
      memcpy(peer.peer_addr, &encrypted_devices[i].mac, sizeof(MacAddr));
      memcpy(peer.lmk, encrypted_devices[i].key, ESP_NOW_KEY_LEN);
      peer.channel = CONFIG_GATEWAY_CHANNEL;
      peer.ifidx   = (wifi_interface_t) ESP_IF_WIFI_AP;
      peer.encrypt = true;
      ESP_ERROR_CHECK(esp_now_add_peer(&peer));
      ESP_LOGI(TAG, "  MAC: " MACSTR ", LMK: %s", MAC2STR(encrypted_devices[i].mac), (char *) encrypted_devices[i].key);
    }
    ESP_LOGI(TAG, "----- End of list. -----");
  }

  return status;
}

void ESPNowReceiver::receive_handler(const uint8_t * mac_addr, const uint8_t * incoming_data, int len)
{
  Message msg;

  ESP_LOGD(TAG, "Received %d bytes from " MACSTR ":", len, MAC2STR(mac_addr));
  dump_data(TAG, incoming_data, len);

  struct PKT {
    uint16_t crc;
    char data[0];
  } __attribute__((packed)) * pkt;

  pkt = (PKT *) incoming_data;
  int data_length = len - 2;

  // If MAC address does not exist in peer list, add it to the list.
  if (esp_now_is_peer_exist(mac_addr) == false) {
    esp_now_peer_info_t peer;
    memset(&peer, 0, sizeof(esp_now_peer_info_t));
    peer.channel = CONFIG_GATEWAY_CHANNEL;
    peer.ifidx   = (wifi_interface_t) ESP_IF_WIFI_AP;
    peer.encrypt = false;
    //memcpy(peer.lmk, ESP_NOW_LMK, ESP_NOW_KEY_LEN);
    memcpy(peer.peer_addr, mac_addr, ESP_NOW_ETH_ALEN);
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));
  }

  if (esp_crc16_le(UINT16_MAX, (const uint8_t *)(pkt->data), data_length) != pkt->crc) {
    ESP_LOGE(TAG, "Received packet crc error.");
  }
  else {
    msg.data = (uint8_t *) malloc(data_length);
    if (msg.data == nullptr) {
      ESP_LOGE(TAG, "Unable to allocation memory for message data.");
    }
    else if (msg_queue != nullptr) {
      memcpy(msg.data, pkt->data, data_length);
      msg.length = data_length;
      if (xQueueSend(msg_queue, &msg, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Message Queue is full, message is lost.");
      }
    }
  }
}

void ESPNowReceiver::prepare_for_deep_sleep()
{
  esp_now_deinit();
}

#endif