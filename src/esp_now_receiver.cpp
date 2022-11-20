#include "config.hpp"
#ifdef ESP_NOW_GATEWAY

#include "esp_now_receiver.hpp"

#include <cstring>

QueueHandle_t ESPNowReceiver::msg_queue = nullptr;
bool          ESPNowReceiver::abort     = false;

esp_err_t ESPNowReceiver::init(QueueHandle_t queue)
{
  esp_err_t status;

  msg_queue = queue;

  status = esp_now_init();
  if (status != ESP_OK) {
    ESP_LOGE(TAG, "Unable to initialize ESP-NOW: %s.", esp_err_to_name(status));
  }

  status = esp_now_register_recv_cb(receive_handler);
  if (status != ESP_OK) {
    ESP_LOGE(TAG, "Unable to register ESP-NOW handler: %s.", esp_err_to_name(status));
  }

  return status;
}

void ESPNowReceiver::receive_handler(const uint8_t * mac_addr, const uint8_t * incoming_data, int len)
{
  Message msg;

  msg.data = (char *) malloc(len);
  if (msg.data == nullptr) {
    ESP_LOGE(TAG, "Unable to allocation memory for message data.");
  }
  else if (msg_queue != nullptr) {
    memcpy(msg.data, incoming_data, len);
    msg.length = len;
    xQueueSend(msg_queue, &msg, 0);
  }
}

void ESPNowReceiver::prepare_for_deep_sleep()
{

}

#endif