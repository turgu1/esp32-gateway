## ESP32-Gateway - ESP32 based ESP-NOW/UDP -> MQTT gateway

Work in progress... nothing works for now

This is a simple gateway that allow the transmission of ESP-NOW or UDP pacquets received from other ESP32/ESP8266 to a MQTT server. The intent is to optimize the battery powered ESP32/ESP8266 sensing devices using ESP-NOW and c/c++ coding instead of Tasmota or ESPHome solutions. Accessing the sensors information from products like OpenHab and HomeAssistant will be then possible through a MQTT server.

The gateway can be configured to accept ESP-NOW or UDP sensors packets through configuration paramters. The gateway is expected to be powered through AC adaptor and be always ON.

The received packet from ESP-NOW/UDP sensors are expected to have a topic name suffix at the beginning of the pacquet, followed by a semicolon, followed by the data to be send to the MQTT server. The topic name suffix is used by the gateway to generate a topic name of the form `<topic name prefix>/<topic name suffix>`. For exemple, if the topinc name prefix is `iot/` and the topic name suffix is `home_temp` the MQTT topic will be `iot/home_temp`. The topic name prefix is adjustable in the `config.hpp` file.

The following files must be adjusted to reflect your environment:

- `include/config.hpp`
- `include/secret.hpp`

Both files are absent from the retrieved project. Vanilla versions of the files are supplied in the`include` folder: `config-dist.hpp` and `secret-dist.hpp`.

This project uses the ESP-IDF framework. Platformio is used to control the compilation process. 

For now this is a one-way capability (sensors only). The following enhancement will come after the basic functionality be completed:

1) Optional Encryption for MQTT
2) Optional Encryption for ESP-NOW or UDP
3) If possible, support for both UDP and ESP-NOW at the same time
3) End to end two-way communication
4) Gateway's web based config access and logging
5) OTA support
