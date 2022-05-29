#pragma once

#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include "api.h"
#include "ble_adapter.h"
#include "domain.h"
#include "http_common.h"
#include "settings.h"

namespace ArduMower
{
  namespace Modem
  {

    namespace Http
    {
      class UiAdapter : public Common
      {
      public:
        UiAdapter(Api::Api &api,
                  Settings::Settings &settings,
                  AsyncWebServer &server,
                  ArduMower::Domain::Robot::StateSource &source
                  );
        void begin();

      private:
        Api::Api &_api;
        Settings::Settings &_settings;
        AsyncWebServer &_server;
        ArduMower::Domain::Robot::StateSource &_source;
        AsyncCallbackJsonWebHandler *_settingsHandler;

        void handleRequest(AsyncWebServerRequest *request);
        bool servePath(AsyncWebServerRequest *request, const String &path);
        void handleApiGetModemSettings(AsyncWebServerRequest *request);
        void handleApiPostModemSettings(AsyncWebServerRequest *request, JsonVariant &json);
        void handleApiResetModemSettings(AsyncWebServerRequest *request);
        void handleApiGetModemInfo(AsyncWebServerRequest *request);
        void handleApiGetModemStatus(AsyncWebServerRequest *request);
        void handleApiGetRobotDesiredState(AsyncWebServerRequest *request);
        void handleApiResetModemBluetoothPairings(AsyncWebServerRequest *request);

        void delayedRestart();
      };
    }
  }
}
