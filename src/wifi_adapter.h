#pragma once

#include <Arduino.h>
#include "settings.h"

namespace ArduMower
{
  namespace Modem
  {
    namespace Wifi
    {
      enum Mode
      {
        OFF = 0,
        STA = 1,
        AP = 2
      };

      class Adapter
      {
      private:
        Settings::Settings &_settings;
        Mode _mode;
        uint32_t _staTimeoutStart;
        uint32_t _offTimeoutStart;

        Mode mode();
        void loopOff();
        void loopSta();
        void loopAp();

        void beginOff();
        void beginSta();
        void beginAp();
        void stop();

        void switchToOff();

      public:
        Adapter(Settings::Settings &settings) : _settings(settings), _mode(Mode::OFF), _staTimeoutStart(0), _offTimeoutStart(0) {}

        void begin();
        void loop();
      };
    }
  }
}
