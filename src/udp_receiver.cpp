#include "config.hpp"
#ifdef UDP_GATEWAY

#include <esp_log.h>

#include "global.hpp"
#include "udp_receiver.hpp"

xTaskHandle   UDPReceiver::task      = nullptr;
QueueHandle_t UDPReceiver::msg_queue = nullptr;
bool          UDPReceiver::abort     = false;

esp_err_t UDPReceiver::init(QueueHandle_t queue)
{
  abort = false;
  msg_queue = queue;

  if (xTaskCreate(receive_server, "udp_server", 4096, (void*)AF_INET, 5, &task) != pdPASS) {
    ESP_LOGE(TAG, "Unable to create receive server task.");
    return ESP_FAIL;
  }
  return ESP_OK;
}

void UDPReceiver::receive_server(void * params) 
{
  static char rx_buffer[MAX_MSG_SIZE];
  static char  addr_str[128];
  int addr_family = AF_INET;
  int ip_protocol = 0;
  struct sockaddr_in6 dest_addr;

  while (!abort) {
    struct sockaddr_in * dest_addr_ip4 = (struct sockaddr_in *) &dest_addr;

    dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr_ip4->sin_family      = AF_INET;
    dest_addr_ip4->sin_port        = htons(UDP_IN_PORT);
    ip_protocol                    = IPPROTO_IP;

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        break;
    }
    ESP_LOGI(TAG, "Socket created");

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
    ESP_LOGI(TAG, "Socket bound, port %d", UDP_IN_PORT);

    struct sockaddr_storage source_addr; // Large enough for both IPv4 or IPv6
    socklen_t socklen = sizeof(source_addr);

    while (!abort) {
      ESP_LOGI(TAG, "Waiting for data");
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

        rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string...
        ESP_LOGI(TAG, "Received %d bytes from %s:", len, addr_str);
        ESP_LOGI(TAG, "%s", rx_buffer);

        Message msg;

        msg.data = (char *) malloc(len + 1);
        if (msg.data == nullptr) {
          ESP_LOGE(TAG, "Unable to allocation memory for message data.");
        }
        else if (msg_queue != nullptr) {
          memcpy(msg.data, rx_buffer, len + 1);
          msg.length = len + 1;
          xQueueSend(msg_queue, &msg, 0);
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