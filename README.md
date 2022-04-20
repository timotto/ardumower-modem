# ArduMower ESP Modem

This is an Arduino Sketch for the ESP32 companion of the [ArduMower](https://www.ardumower.de/).
It provides the same Bluetooth LE and HTTP connectivity as the `esp32_ble` Sketch from the [Sunray](https://github.com/ardumower/Sunray) repository.

It also offers support for MQTT and Prometheus to ease the integration of ArduMower into home automation and monitoring setups.

## License

```
ArduMower Modem - Firmware for the ESP32 connected as Modem to an ArduMower
Copyright (c) 2021  Tim Otto

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.
```

## Flashing the ArduMower Modem firmware

Flashing the firmware onto the ESP32 for the first time requires some effort. Subsequent updates can be installed comfortably using the web interface of the Modem.

### `modem_install` Arduino Sketch

Use the [modem_install](util/modem_install/modem_install.ino) Arduino Sketch from the `util` folder to install the latest release binary on your ESP32.
This Arduino Sketch requires nothing but a vanilla Arduino setup with the ESP32 package installed.
No additional libraries are required.

### Source code and Arduino IDE

Download the source code of the [latest release](https://github.com/timotto/ardumower-modem/releases) and use the Arduino IDE to compile and flash it onto the ESP32 using the Board `ESP32 Dev Module` and Partition Scheme `Minimal SPIFFS`. You will need the Arduino libraries listed in the Dependencies section below to compile the source code.

## First time WiFi setup

Once the firmware is running it will start a WiFi access point called `ArduMower Modem` with the password `ArduMower Modem`. Connect to that access point to access the Modem's web interface at [http://192.168.4.1/](http://192.168.4.1/). From there you are able to configure your WiFi credentials, Bluetooth security settings and everything else.

## Secondary features

The primary feature of the firmware is the same as the `esp32_ble` Arduino Sketch from the Sunray repository. But there's more.

### MQTT

The ArduMower Modem supports MQTT for status reporting and control. It has support for HomeAssistant Autodiscovery as a vacuum cleaner. This integrates nicely with Google Assistant, and I'm pretty sure with Alexa as well.

### Prometheus

The Prometheus endpoint of the ArduMower Modem makes it easy to collect metrics about the ArduMower and the Modem.

## Dependencies

### Development Environment

The sketch is developed using [Arduino 1.8.16](https://www.arduino.cc/) and the [Arduino core for the ESP32 1.0.6](https://github.com/espressif/arduino-esp32). All automation is orchestrated by a [Taskfile](https://taskfile.dev/).
Building the web interface requires Node JS. The tools to package the web interface and to run the validation tests require Go.

### Arduino Libraries

Most libraries are available via the Arduino Library Manager:
- [AUnit](https://github.com/bxparks/AUnit)
- [ArduinoJson](https://arduinojson.org/)
- [ArduinoWebsockets](https://github.com/gilmaimon/ArduinoWebsockets)
- [MQTT](https://github.com/256dpi/arduino-mqtt)
- [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino)

Two libraries need to be installed manually:
- [ESPAsyncWebServer](https://github.com/timotto/ESPAsyncWebServer)
- [AsyncTCP](https://github.com/timotto/AsyncTCP)

The original versions from `me-no-dev` work great as long as load and throughput are low. The [AsyncTCP fork from OttoWinter](https://github.com/OttoWinter/AsyncTCP.git) and [ESPAsyncWebServer fork from lorol](https://github.com/lorol/ESPAsyncWebServer.git) were already almost stable. Still, both required a small fix, so I forked them again until I have my commits merged upsream.

### Automation

Several tasks defined in [Taskfile.yml](Taskfile.yml) allow integration with an arbitrary IDE or just the command line by using the [Arduino CLI](https://github.com/arduino/arduino-cli).

## Gratitude

My ArduMower is my only lawn mower. It saved me countless hours of manual labor which I was able to spend tinkering with this source code and other hobbies.

I am very grateful to the ArduMower community for building an awesome hardware and software platform and making it available to the public. With this contribution I want to become an active member of the ArduMower community.
