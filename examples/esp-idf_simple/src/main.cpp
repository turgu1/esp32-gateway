#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "app.hpp"

const char * TAG = "esp32-gateway";

App app;

extern "C" {

  void app_main() 
  {
    if (app.init() == ESP_OK) {
      vTaskStartScheduler();
    }
  }

}
