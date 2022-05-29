#include <Arduino.h>
#include "esp_os.h"

using namespace ArduMower::Modem;

void ArduinoESP::restart()
{
  ESP.restart();
}
