# ArduMower Modem CI

A [Concourse](https://concourse-ci.org/) pipeline using [Docker images for Arduino](https://hub.docker.com/u/arduinoci) (also build [with a pipeline](https://github.com/timotto/arduinoci/blob/main/pipeline.yml)) to build and test the ArduMower Modem firmware.

The tests are executed on an ESP32 connected to a [Raspberry Pi Houdini Concourse Worker](https://github.com/timotto/arduinoci/tree/main/concourse/rpi-concourse-worker).