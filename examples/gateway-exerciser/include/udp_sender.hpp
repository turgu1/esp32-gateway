#pragma once

#include "config.hpp"

#ifdef CONFIG_EXERCISER_ENABLE_UDP

#include <lwip/sockets.h>

class UDPSender
{
  private:
    static constexpr char const * TAG = "UDP Sender";

    int                sock;
    struct sockaddr_in dest_addr;

  public:
    esp_err_t                   init();
    esp_err_t                   send(const uint8_t * data, int len);
    void      prepare_for_deep_sleep();
};

#endif