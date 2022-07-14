# ArduMower ESP Modem

This is an Arduino Sketch for the ESP32 companion of the [ArduMower](https://www.ardumower.de/).
It provides the same Bluetooth LE and HTTP connectivity as the `esp32_ble` Sketch from the [Sunray](https://github.com/ardumower/Sunray) repository.

It also offers support for MQTT and Prometheus to ease the integration of ArduMower into home automation and monitoring setups.

## License

ArduMower Modem - Firmware for the ESP32 connected as Modem to an ArduMower
Copyright (c) 2022 Tim Otto

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.

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

### PS4 controller 
- The robotic lawnmower can be controlled with a PS4 controller
- Button assignment:
   - left joystick -> fast movements, 
   - right joystick -> slow movements, 
   - cross + R2 -> linear movements + rotation on the spot,
   - triangle -> start automatic mowing,
   - rectangle -> stop automatic mowing,  
   - circle -> mowing motor on/off,
   - cross -> skip next mowing point,
   - L1 -> reduce mowing speed,
   - R1 -> increase mowing speed
- The configuration is done via the web interface

-> The compilation is done with PlatformIO. Required dependencies are stored in platformio.ini. 

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
- [PS4-esp32](https://github.com/aed3/PS4-esp32.git)

Two libraries need to be installed manually:
- [ESPAsyncWebServer](https://github.com/timotto/ESPAsyncWebServer)
- [AsyncTCP](https://github.com/timotto/AsyncTCP)

The original versions from `me-no-dev` work great as long as load and throughput are low. The [AsyncTCP fork from OttoWinter](https://github.com/OttoWinter/AsyncTCP.git) and [ESPAsyncWebServer fork from lorol](https://github.com/lorol/ESPAsyncWebServer.git) were already almost stable. Still, both required a small fix, so I forked them again until I have my commits merged upsream.

### Automation

Several tasks defined in [Taskfile.yml](Taskfile.yml) allow integration with an arbitrary IDE or just the command line by using the [Arduino CLI](https://github.com/arduino/arduino-cli).

## Gratitude

My ArduMower is my only lawn mower. It saved me countless hours of manual labor which I was able to spend tinkering with this source code and other hobbies.

I am very grateful to the ArduMower community for building an awesome hardware and software platform and making it available to the public. With this contribution I want to become an active member of the ArduMower community.
