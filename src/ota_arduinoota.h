#pragma once

#include "ota.h"
#include "settings.h"

namespace ArduMower
{
  namespace Modem
  {
    namespace Ota
    {
      class ArduinoOta : public Ota
      {
      private:
        ArduMower::Modem::Settings::Settings & _settings;
        bool _active;

      public:
        ArduinoOta(ArduMower::Modem::Settings::Settings & settings) : _settings(settings) {}

        virtual void begin() override;
        virtual void loop() override;
        virtual bool active() override { return _active; };
      };
    }
  }
}