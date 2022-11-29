#include "global.hpp"

#ifdef CONFIG_GATEWAY_ENABLE_ESP_NOW

  LmkKey lmk_keys[MAX_LMK_KEY_COUNT] = { "lmk1234567890123" };

  EncryptedDevice encrypted_devices[MAX_ENCRYPTED_DEVICES_COUNT] = {
    // { { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, &lmk_keys[0] }
    { { 0xac, 0x67, 0xb2, 0x2a, 0x9f, 0x88 }, &lmk_keys[0] }
  };
#endif