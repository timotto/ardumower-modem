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

      class Relay {
      public:
        virtual bool isConnected() = 0;
        virtual float pingRTT() = 0;
        virtual int connectionCount() = 0;
        virtual uint32_t connectionTime() = 0;
        virtual float connectionDuration() = 0;
      };

      class Api
      {
      public:
        OS & os;
        Bluetooth * ble;
        Relay * relay;

        Api(OS & _os) : os(_os) {};

        void begin(Bluetooth * _ble, Relay * _relay)
        {
          ble = _ble;
          relay = _relay;
        }
      };
    }
  }
}
