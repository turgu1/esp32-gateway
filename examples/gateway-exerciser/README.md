### Simple ESP-IDF based ESP-NOW/UDP Exerciser

This is an exerciser application for the ESP32 Gateway. It sends
simple packets every second using UDP or ESP-NOW protocol depending on it's configuration.

### Configuration

The following files must be adjusted to reflect your environment:

- `include/global.hpp`
- `include/secret.hpp`

Both files are absent from the retrieved project. Vanilla versions of the files are supplied in the`include` folder: `global-dist.hpp` and `secret-dist.hpp`.

This project uses the ESP-IDF framework. Platformio is used to control the compilation process. 