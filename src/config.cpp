#include "config.hpp"

#ifdef CONFIG_GATEWAY_ENABLE_ESP_NOW

  LmkKey lmk_keys[MAX_LMK_KEY_COUNT] = { "lmk1234567890123" };

  EncryptedDevice encrypted_devices[MAX_ENCRYPTED_DEVICES_COUNT] = {
    // { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, &lmk_keys[0] }
  };
#endif