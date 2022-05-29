#ifndef _MODEM_CLI_H
#define _MODEM_CLI_H

#include "router.h"
#include <Arduino.h>

namespace ArduMower
{
  namespace Modem
  {
    class Cli : public RxDrain
    {
    private:
      Router &router;

      void respondVersion();
      void echoResponse(String &req);

    public:
      Cli(Router &r) : router(r) {}
      void begin() { router.sniffRx(this); }
      virtual void drainRx(String line, bool &stop) override;
    };
  }
}
#endif
