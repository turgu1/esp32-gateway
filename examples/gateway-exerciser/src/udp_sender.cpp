#include "global.hpp"

#ifdef UDP_SENDER

#include <cstring>
#include <esp_crc.h>

#include "utils.hpp"
#include "udp_sender.hpp"

esp_err_t UDPSender::init()
{
  esp_log_level_set(TAG, LOG_LEVEL);

  esp_err_t status = ESP_OK;

  dest_addr.sin_addr.s_addr = inet_addr(GATEWAY_IP_ADDR);
  dest_addr.sin_family      = AF_INET;
  dest_addr.sin_port        = htons(UDP_OUT_PORT);
  
  int addr_family           = AF_INET;
  int ip_protocol           = IPPROTO_IP;

  sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
  if (sock < 0) {
    ESP_LOGE(TAG, "Unable to create socket: %s", strerror(errno));
    status = ESP_FAIL;
  }

  return status;
}

esp_err_t UDPSender::send(const uint8_t * data, int len)
{
  esp_err_t status = ESP_OK;

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
    pkt.crc = esp_crc16_le(UINT16_MAX, (const uint8_t *)(pkt.data), len);

    int err = sendto(sock, (const uint8_t *) &pkt, len + 2, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
        status = ESP_FAIL;
    }
    else {
      ESP_LOGD(TAG, "The following message was sent:");
      dump_data(TAG, data, len);
    }
  }

  return status;
}

void UDPSender::prepare_for_deep_sleep()
{
  if (sock != -1) {
    ESP_LOGE(TAG, "Shutting down socket and restarting...");
    shutdown(sock, 0);
    close(sock);
  }
}

#endif