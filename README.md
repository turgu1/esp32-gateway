### ESP32-Gateway - ESP32 based ESP-NOW/UDP -> MQTT gateway

2022-11-25 - Version 0.1

This project is working. This is still under eavy modifications.

This is a simple gateway that allow the transmission of ESP-NOW or UDP pacquets received from other ESP32/ESP8266 to a MQTT server. The intent is to optimize the battery powered ESP32/ESP8266 sensing devices using ESP-NOW and C/C++ coding instead of Tasmota or ESPHome solutions. Accessing the sensors information from products like OpenHab and HomeAssistant will be then possible through a MQTT server.

Using ESP-NOW, a ESP32 sensor can read a PIO and send a pacquet in less than a second from the device bootstrap to the packet transmission.

Here are the principal characteristics:

- UDP and ESP-NOW sender support: UDP can be used for any kind of sensors that can send UDP packets on the local network. ESP-NOW can be used with Espressif supported devices (ESP32 and ESP8266 families, maybe others)
- Packet size limit of 248 bytes for ESP-NOW and configurable for UDP.
- Verbose output for debugging and monitoring through standard ESP32 development board USB port.
- Allow for the use of a JSON Lite format in the sensor received packets: when possible, double-quotes can be omited and will be added before being sent to the MQTT server. This is to optimize the packet content coming from the sensor.
- ESP-IDF based. PlatformIO is used to manage the gateway build process.

The gateway is expected to be powered through AC adaptor and be always ON.

### Packet format

The received packet from ESP-NOW/UDP sensors are expected to have the following fields in the order shown:

- A 16 bits CRC checksum
- A topic name suffix (normally ascii characters) followed by a separator. That suffix is used by the gateway to generate a topic name of the form `<topic name prefix>/<topic name suffix>`. For exemple, if the topic name prefix is `iot/` and the topic name suffix is `home_temp` the MQTT topic will be `iot/home_temp`. The topic name prefix is adjustable in the `config.hpp` file. The separator can be `;` for JSON Lite content, or `|` for plain content.
- The data to be send to the MQTT server. It can be some text, json, json lite, or even binary.

### Plain content

No specific processing is done on plain content: it is sent as-is to the MQTT broker.

### JSON Lite content

The JSON Lite processing add double-quotes on strings without them. It also eliminate irrelevant spaces that could be part of the packet. It keeps everything that present after the last `}`. If a string contains any of the following characters, it is required that the input string in the received packet stay with double-quotes:

```spaces : , ] }```

The following characters are processed as spaces:

```space horizontal-tab vertical-tab new-line carriage-return form-feed```

The JSON Lite procesing doesn't expect unicode characters. Augmented ASCII (8 bits characters) is OK.

### Configuration

The following files must be adjusted to reflect your environment:

- `include/config.hpp`
- `include/secret.hpp`

Both files are absent from the retrieved project. Vanilla versions of the files are supplied in the`include` folder: `config-dist.hpp` and `secret-dist.hpp`.

### Channel usage

ESP-NOW relies on the use of a single channel to transmit packets between devices. To allow the gateway to transmit packets to the MQTT server, it needs to connect to the Wifi network as a station mode device. As such, the Wifi router channel is used to communicate. As the ESP32 only have a single Wifi antenna, the same channel must be used for both Station connection to the Wifi network and for ESP-NOW. It is then important to freeze the router channel (usually to one of 1, 6 or 11 to mitigate interference between channels) and configure all ESP-NOW devices to use that channel.

----

This project uses the ESP-IDF framework. Platformio is used to control the compilation process. 

For now this is a one-way capability (sensors only). The following enhancements will come after the basic functionality be completed:

1) Optional Encryption for MQTT
2) Optional Encryption for ESP-NOW or UDP
3) End to end two-way communication
4) Gateway's web based config access and logging
5) OTA support
