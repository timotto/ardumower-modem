#pragma once

#include "api.h"

namespace ArduMower
{
  namespace Modem
  {
    class ArduinoESP : public Api::OS {
    public:
      virtual void restart() override;
    };
  }
}