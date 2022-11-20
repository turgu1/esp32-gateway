#pragma once

#include <cstring>
#include <mutex>
#include <esp_event.h>
#include <mqtt_client.h>

class MQTT
{
  public:
    enum class State : int8_t {
      NOT_INITIALIZED,
      INITIALIZED,
      CONNECTING,
      CONNECTED,
      DISCONNECTED,
      PUBLISHED,
      ERROR
    };

  private:
    static constexpr char const * TAG = "MQTT Class";

    static State      state;
    static std::mutex mutex;

    static void event_handler(void * args, esp_event_base_t base, int32_t event_id, void *event_data);

    esp_mqtt_client_config_t mqtt_cfg;
    esp_mqtt_client_handle_t client_handle;

  public:
    esp_err_t                init();
    esp_err_t             publish(const char * topic, const char * data, int len, int qos, int retain);
    const State &       get_state(void) { return state; }
    void   prepare_for_deep_sleep();
    static void        show_state();
};