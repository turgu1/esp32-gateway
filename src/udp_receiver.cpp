#include "global.hpp"
#ifdef CONFIG_GATEWAY_ENABLE_UDP

#include <esp_log.h>
#include <esp_crc.h>

#include "utils.hpp"
#include "udp_receiver.hpp"

xTaskHandle   UDPReceiver::task      = nullptr;
QueueHandle_t UDPReceiver::msg_queue = nullptr;
bool          UDPReceiver::abort     = false;

esp_err_t UDPReceiver::init(QueueHandle_t queue)
{
  abort     = false;
  msg_queue = queue;

  esp_log_level_set(TAG, CONFIG_GATEWAY_LOG_LEVEL);

  if (xTaskCreate(receive_server, "udp_server", 4096, (void*)AF_INET, 5, &task) != pdPASS) {
    ESP_LOGE(TAG, "Unable to create receive server task.");
    return ESP_FAIL;
  }
  return ESP_OK;
}

void UDPReceiver::receive_server(void * params) 
{
  static uint8_t rx_buffer[CONFIG_GATEWAY_UDP_MAX_PKT_SIZE];
  static char  addr_str[128];
  int addr_family = AF_INET;
  int ip_protocol = 0;
  struct sockaddr_in6 dest_addr;

  while (!abort) {
    struct sockaddr_in * dest_addr_ip4 = (struct sockaddr_in *) &dest_addr;

    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family      = AF_INET;
    dest_addr_ip4->sin_port        = htons(CONFIG_GATEWAY_UDP_PORT);
    ip_protocol                    = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        break;
    }
    ESP_LOGD(TAG, "Socket created");

    // Set timeout
    // struct timeval timeout;
    // timeout.tv_sec = 10;
    // timeout.tv_usec = 0;
    // setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
      ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
      break;
    }
    ESP_LOGD(TAG, "Socket bound, port %d", CONFIG_GATEWAY_UDP_PORT);

    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t socklen = sizeof(source_addr);

    while (!abort) {
      ESP_LOGD(TAG, "Waiting for data");
      int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

      if (abort) break;

      // Error occurred during receiving
      if (len < 0) {
        ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
        break;
      }
      // Data received
      else {
        // Get the sender's ip address as string
        inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr, addr_str, sizeof(addr_str) - 1);

        ESP_LOGD(TAG, "Received %d bytes from %s:", len, addr_str);
        dump_data(TAG, rx_buffer, len);
 
        Message msg;

        struct PKT {
          uint16_t crc;
          char data[0];
        } __attribute__((packed)) * pkt;

        pkt = (PKT *) rx_buffer;
        int data_length = len - 2;

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
    }

    if (sock >= 0) {
      ESP_LOGE(TAG, "Shutting down socket and restarting...");
      shutdown(sock, 0);
      close(sock);
    }
  }

  vTaskDelete(NULL);
}

void UDPReceiver::prepare_for_deep_sleep()
{
  if (task != nullptr) {
    abort = true;
    xTaskAbortDelay(task);
  }
}

#endif