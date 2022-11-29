#pragma once

#include "config.hpp"

#ifdef CONFIG_GATEWAY_ENABLE_ESP_NOW
  // The following are the number of entries in the tables as defined in the src/config.cpp file.
  constexpr const int LMK_KEY_COUNT           = 1;
  constexpr const int ENCRYPTED_DEVICES_COUNT = 1;
#endif