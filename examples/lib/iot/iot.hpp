#pragma once

#include "config.hpp"

#define __IOT__
#include "global.hpp"

#undef __IOT__

class IOT
{
  private:
    static constexpr char const * TAG = "IOT Class";

  public:
    esp_err_t                   init();
    esp_err_t prepare_for_deep_sleep();
};