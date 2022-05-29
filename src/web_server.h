#ifndef _LANDING_PAGE_H
#define _LANDING_PAGE_H

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

namespace ArduMower
{
  namespace Modem
  {
    class WebServer
    {
    private:
      AsyncWebServer _server;

    public:
      WebServer() : _server(AsyncWebServer(80)) {}

      void begin();
      AsyncWebServer &server() { return _server; }
    };
  }
}
#endif
