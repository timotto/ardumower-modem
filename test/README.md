# Tests

These are some unit and integration tests running on an ESP32 written with [AUnit](https://github.com/bxparks/AUnit) which are not included in the runtime firmware binary. They rely on a compile time switch

```#c
#define ESP_MODEM_TEST
```

to run instead of the main application runtime.

To run the tests you can either put the `#define` line from above into the `ardumower-modem.ino` file or switch to the project root directory in a terminal and execute

```#sh
task test
```

The tests need a separate ESP32 with a loopback connection between `Serial2` and `Serial1`:

- `RXD2` -> `GPIO22`
- `TXD2` -> `GPIO23`

Please note the GPIO pins for `Serial1` as they are **not** the default pins. 

The loopback connection is required for some tests which run a second FreeRTOS task with a [simulated ArduMower](helper/fake_ardumower.h) connected to `Serial1`.
