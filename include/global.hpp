#pragma once

// This is the structure used in a message  xQueue between the receiver and the main app task

struct Message {
  int       length;
  uint8_t * data;
};

typedef uint8_t MacAddr[6];
typedef uint8_t LmkKey[17];

struct EncryptedDevice {
  MacAddr   mac;
  LmkKey  * key;
};
