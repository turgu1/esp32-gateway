#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "app.hpp"

const char * TAG = "Esp-Now Sender";

App app;

extern "C" {
  void app_main() {
    esp_log_level_set(TAG, CONFIG_EXERCISER_LOG_LEVEL);

    if (app.init() != ESP_OK) {
      ESP_LOGE(TAG, "Main App Initialization failed...");
    }
  }
}