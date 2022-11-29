### Simple ESP-IDF based ESP-NOW/UDP Exerciser

This is an exerciser application for the ESP32 Gateway. It sends simple packets every 10 second to the gateway. Between transfers, the exerciser is put in deep sleep. The folling aspects can be put in test through configuration parameters:

- Using UDP or ESP-NOW protocol
- Using packets encryption or not
- Long range communication

When the exerciser is configured to use the ESP-NOW protocol, it recovers the gateway's MAC address from the NVS (Non Volatile Storage). If it is not available, or the device was reset, it will scan for an AP to find the gateway.

When the exerciser is configured to use the UDP protocol, the gateway address must be configured through menuconfig. It can be an IP address or a DNS name.

### Configuration

The Gateway Exerciser configuration is done through the menuconfig capability associated with ESP-IDF. The following PlatformIO's menu option can be used to access the menuconfig application: `Platform > Run Menuconfig`.

All options for the Gateway will be found under the menuconfig entry named `Gateway Exerciser`.

Here is the list of the configuration items:

- **Log Level**: Max log level used by the exerciser to report various log information on the USB port.
- **MQTT Topic Suffix**: The topic suffix that will be used by the gateway to generate the topic to be sent to the MQTT broker.
- **Transmission Protocol**: The protocol to be used to transmit packets to the ESP32 Gateway. One of **UDP** or **ESP-NOW**.

For the UDP Protocol:
- **UDP Port**: The UDP Port to be used by the exerciser to transmit packets to the gateway.
- **UDP Max Packet Size**: The UDP maximum packet size allowed.
- **Gateway Address**: The Gateway address. It can be entered as a standard IPv4 dotted decimal notation (xx.xx.xx.xx) or as a DNS name.

For the ESP-NOW Protocol:
- **Primary Master Key**: The Primary Master Key (PMK) to use. The length of the PMK must be 16 characters. Please ensure that the key is in synch with the PMK defined in the gateway.
- **Gateway AP SSID Prefix**: The beginning of the SSID for the gateway Access Point (AP). This will be used to find the AP MAC address to transmit ESP-NOW packets to the gateway.
- **Encryption Enabled**: Set if this device is using packet encryption. If set, the gateway internal table of crypted devices must be modified accordingly.
- **Local Master Key**: If encryption is enabled, the Local Master Key (LMK) for the exerciser to use. The length of LMK must be 16 characters. Please ensure that the LMK reflects the gateway configuration.
- **Channel**: The Wifi channel to be used. Note that it must be the same as defined in the WiFi router. Usual values are 1, 6, or 11. These preferred values are to diminish potential r/f interference.
- **Max Packet Size**: The ESP-NOW maximum packet size allowed without considering the CRC.  Cannot be larger than 248.
- **Enable Long Range**: When enable long range, the PHY rate of ESP32 will be 512Kbps or 256Kbps.

For the Wifi sub-system:
- **Wifi Gateway SSID**: SSID associated to Wifi as defined in your gateway. Will be used to find the gateway MAC address when unknown. Only needed with the ESP-NOW protocol.
- **Wifi Gateway Password**: Password associated to Wifi as defined in your gateway. Can be empty for a password-less configuration.  Only needed with the ESP-NOW protocol.
- **Wifi Router SSID**: SSID as defined in your router. Only needed with the UDP Protocol.
- **Wifi Router Password**: Password as defined in your router. Can be empty.  Only needed with the UDP Protocol.
- **Wifi Authorization Mode**: The authorization mode. Can be WEP, WPA, WPA2, WPA3.