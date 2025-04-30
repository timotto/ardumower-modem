#pragma once

#include <inttypes.h>

namespace ArduMower
{
  namespace Modem
  {
    namespace Api
    {
      class OS {
      public:
        virtual void restart() = 0;
      };

      class Bluetooth {
      public:
        virtual void clearPairings() = 0;
      };

      class Api
      {
      public:
        OS & os;
        Bluetooth * ble;

        Api(OS & _os) : os(_os) {};

        void begin(Bluetooth * _ble)
        {
          ble = _ble;
        }
      };
    }
  }
}
