#pragma once

#include <ESPAsyncWebServer.h>
#include "settings.h"

namespace ArduMower
{
  namespace Modem
  {
    namespace Http
    {
      class Common
      {
      public:
        Common(ArduMower::Modem::Settings::Settings &_settings);

      protected:
        bool auth(AsyncWebServerRequest *request);
        void reject(AsyncWebServerRequest *request, int statusCode, const String & action, const String & error);

      private:
        ArduMower::Modem::Settings::Settings &settings;
      };
    }
  }
}