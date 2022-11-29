### ESP32-Gateway - ESP32 based ESP-NOW/UDP → MQTT gateway

2022-11-28 - Version 0.2.0

Note: This is still under heavy modification. Must be condidered as BETA-Level code.

This is a simple gateway that allows the transmission of ESP-NOW or UDP packets received from other ESP32/ESP8266 to an MQTT server. The intent is to optimize the battery-powered ESP32/ESP8266 sensing devices using ESP-NOW and C/C++ coding instead of Tasmota or ESPHome solutions. Accessing the sensor information from products like OpenHab and HomeAssistant will be then possible through an MQTT server.

Using ESP-NOW, an ESP32 sensor can read a PIO and send a packet in less than a second from the device bootstrap to the packet transmission to the gateway.

Here are the principal characteristics:

- UDP and ESP-NOW sender support: UDP can be used for any kind of sensor that can send UDP packets on the local network. ESP-NOW can be used with Espressif-supported devices (ESP32 and ESP8266 families, maybe others)
- Configurable maximum packet size limit. Max of 248 bytes for ESP-NOW and 1450 for UDP.
- Verbose output for debugging and monitoring through standard ESP32 development board USB port.
- Allow for the use of a **JSON diet** format in the sensor received packets: when possible, double quotes can be omitted and will be added before being sent to the MQTT server. This is to optimize the packet length coming from the sensor.
- ESP-IDF based. PlatformIO is used to manage the gateway build process.
- Support for ESP-NOW encryption through a pre-defined list of encrypted (sensing) devices.

The gateway is expected to be powered through an AC adaptor and be always ON.

### Packet format

The received packet from ESP-NOW/UDP sensors is expected to have the following fields in the order shown:

- A 16 bits CRC checksum. It is automatically computed and added to the packet by the esp_now class.
- A topic name suffix (normally ASCII characters) followed by a separator. That suffix is used by the gateway to generate a topic name of the form `<topic name prefix>/<topic name suffix>`. For example, if the topic name prefix is `iot/` and the topic name suffix is `home_temp` the MQTT topic will be `iot/home_temp`. The topic name prefix is adjustable through `menuconfig`. The separator can be `;` for **JSON diet** content, or `|` for plain content.
- The data to be sent to the MQTT server. It can be some text, JSON, **JSON diet**, or even binary.

### Plain content

No specific processing is done on plain content: it is sent as-is to the MQTT broker.

### JSON diet

The **JSON diet** format is a version of the JSON format for which double quotes around simple strings are omitted.

The gateway's **JSON diet** processing adds double quotes on strings without them. It also eliminates irrelevant spaces that could be part of the packet. It keeps everything present after the last `}`. If a string contains any of the following characters or starts with a number character, it is required that the input string in the received packet stay with double quotes:

```spaces : , ] }```

The following characters are processed as spaces:

```space horizontal-tab vertical-tab new-line carriage-return form-feed```

The JSON Lite process doesn't expect UTF-8 (Unicode) characters. Augmented ASCII (8 bits characters) is OK.

### Configuration

The ESP32 Gateway configuration is done through the menuconfig capability associated with ESP-IDF. The following PlatformIO's menu option can be used to access the menuconfig application: `Platform > Run Menuconfig`.

All options for the Gateway will be found under the menuconfig entry named `ESP-32 Gateway Configuration`.

Here is the list of the configuration items:

- **Message queue size**: The maximum number of messages queued in the gateway waiting to be sent to the MQTT server.
- **Log Level**: Maximum log level used by the gateway to report various log information on the USB port.
- **Enable UDP packet reception**: When enabled, a task will wait for UDP packets for transmission to the MQTT broker.
- **Enable ESP-NOW packet reception**: When enabled, a task will wait for ESP-NOW packets for transmission to the MQTT broker.

For the UDP Protocol:
- **UDP Port**: The UDP Port to be used by devices to transmit packets to the gateway.
- **UDP Max Packet Size**: The UDP maximum packet size allowed.

For the ESP-NOW Protocol:
- **Primary Master Key**: The Primary Master Key (PMK) for the ESP-NOW devices and gateway to use. The length of the PMK must be 16 bytes. Please ensure that all devices are using the same PMK.
- **Channel**: The Wifi channel to be used. Note that it must be the same as defined in the WiFi router. Usual values are 1, 6, or 11. These preferred values are to diminish potential r/f interference.
- **Max Packet Size**: The ESP-NOW maximum packet size allowed without considering the CRC. Cannot be larger than 248.
- **Enable Long Range**: When enable long range, the PHY rate of ESP32 will be 512Kbps or 256Kbps.

For the MQTT Protocol:
- **MQTT Server URI**: MQTT server URI.
- **MQTT Server topic prefix**: Topic prefix to use to construct complete topic name, appending device's topic suffix value.
- **MQTT Default QOS value**: The MQTT QOS value to use when sending messages to the MQTT server.
- **MQTT Default Retain value**: The MQTT Retain value to use when sending messages to the MQTT server.
- **MQTT Username**: Username as defined in the MQTT server configuration.
- **MQTT Client Identification**: MQTT Client Id used by the gateway.
- **MQTT User Password**: Password associated with username as defined in the MQTT server configuration.

For the Wifi sub-system:
- **Wifi Router SSID**: SSID as defined in the Wifi Router.
- **Wifi Router Password**: Password as defined in the Wifi Router.
- **Wifi Router Authorization Mode**: Authorization mode as defined in your router. Can be WEP, WPA, WPA2, WPA3.
- **Wifi AP SSID**: SSID associated with the gateway Wifi AP for gateway discovery by sensors.
- **Wifi AP Password**: Password associated with the gateway Wifi AP for gateway discovery by sensors. Can be empty for an open AP.
- **Wifi AP Authorization Mode**: Authorization mode to be used for the gateway AP. Can be WEP, WPA, WPA2, WPA3.

### Channel usage

ESP-NOW relies on the use of a single channel to transmit packets between devices. To allow the gateway to transmit packets to the MQTT server, it needs to connect to the Wifi network as a station-mode device. As such, the Wifi router channel is used to communicate. As the ESP32 only has a single Wifi antenna, the same channel must be used for both Station connection to the Wifi network and ESP-NOW. It is then important to freeze the router channel (usually to one of 1, 6, or 11 to mitigate interference between channels) and configure all ESP-NOW devices to use that channel.

### ESP-NOW Encrypted devices

Pre-defined peer-encrypted devices can be identified in the `src/global.cpp` file. A list of keys (6 maximum) and the list of mac addresses/key pointers are used to identify the peers to be added at boot time. The table's actual sizes are located in `include/global.hpp` and can be adjusted if needed (namely the LMK_KEY_COUNT, and ENCRYPTED_DEVICES_COUNT constants). 

This is to simplify the interaction between the gateway and the sensors that require encryption. Note that the number of encrypted devices is limited to 6. No specific protocol is then required, the sensors can immediately send encrypted packets, optimizing power usage. 
 
----

This project uses the ESP-IDF framework. PlatformIO is used to control the compilation process. 

For now, this is a one-way capability (sensors only). The following enhancements will come after the basic functionality is completed:

1) Optional Encryption for MQTT
2) Optional Encryption for ESP-NOW or UDP
3) End-to-end two-way communication
4) Gateway's web-based config access and logging
5) OTA support

### Copyright (c) 2022 Guy Turcotte

Permission is hereby granted, free of charge, to any person obtaining a copy of this project and associated documentation files (the "Project Content"), to deal in the Project without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Project Content, and to permit persons to whom the Project Content is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Project Content.

THE PROJECT CONTENT IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE PROJECT CONTENT OR THE USE OR OTHER DEALINGS IN THE PROJECT CONTENT.