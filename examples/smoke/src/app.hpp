#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "config.hpp"
#include "global.hpp"

class App
{
  private:
    static constexpr char const * TAG = "App Class";

    static xTaskHandle task;
    static void main_task(void * params);

  public:
    esp_err_t init();
};