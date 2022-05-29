#pragma once

#include <Arduino.h>
#include <list>
#include <functional>
#include "reader.h"

namespace ArduMower
{
  namespace Modem
  {
    
    enum XferError {
      SUCCESS = 0,
      TIMEOUT = 1,
      UNKNOWN
    };

    typedef std::function<void(String, XferError)> responseCb;
    typedef std::function<uint32_t(void)> timeSource;

    class RxDrain
    {
    public:
      virtual void drainRx(String line, bool &stop) = 0;
    };

    class TxDrain
    {
    public:
      virtual void drainTx(String line, bool &stop) = 0;
    };

    class Router
    {
    private:
      Stream &down;
      timeSource _millis;
      std::list<RxDrain *> drains;
      std::list<TxDrain *> txDrains;
      Reader downRx;
      bool sendCommand;
      bool expectResponse;
      String command;
      String lastCommand;
      responseCb cb;
      uint32_t lastTx, lastRx;

      void loopSend();
      void loopReceive();
      void loopDrain();
      void loopTimeout();
      
      bool isRxTimeout();

    public:
      Router(Stream &d) : Router(d, millis) {}

      Router(Stream &d, timeSource t)
          : down(d), _millis(t),
            downRx("\r\n"), sendCommand(false), expectResponse(false),
            lastTx(0), lastRx(0) {}
      void sniffRx(RxDrain *d);
      void sniffTx(TxDrain *d);
      void begin();
      void loop();
      bool send(String _command, responseCb _cb);
      bool sendWithoutResponse(String line);
    };

  }
}
