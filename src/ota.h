#pragma once

namespace ArduMower
{
  namespace Modem
  {
    namespace Ota
    {
      class Ota
      {
      public:
        virtual void begin() = 0;
        virtual void loop() = 0;
        virtual bool active() = 0;
      };
    }
  }
}
