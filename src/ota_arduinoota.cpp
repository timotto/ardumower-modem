#include "ota_arduinoota.h"
#include <ArduinoOTA.h>

using namespace ArduMower::Modem::Ota;

void ArduinoOta::begin()
{
  _active = false;
  ArduinoOTA.setHostname(_settings.general.name.c_str());
  ArduinoOTA
      .onStart([&]()
               { _active = true; })
      .onEnd([&]()
             { _active = false; })
      .onError([&](ota_error_t error)
               { _active = false; });

  ArduinoOTA.begin();
}

void ArduinoOta::loop()
{
  ArduinoOTA.handle();
}
