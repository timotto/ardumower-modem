#pragma once

#include <ESPAsyncWebServer.h>
#include "http_common.h"
#include "ota.h"
#include "settings.h"

namespace ArduMower
{
  namespace Modem
  {
    namespace Ota
    {
      class HttpServer;

      namespace Http
      {
        enum Result
        {
          SUCCESS = 0,
          PENDING,
          STARTED,
          INCOMPLETE,
          ERROR,
          INDEX_MISMATCH,
          UPDATE_BEGIN_FAILED,
          VERIFY_HEADER_FAILED,
          SHORT_WRITE_ERROR,
          UPDATE_END_FAILED,
        };

        class ServerSession
        {
        private:
          HttpServer *s;
          Result result;
          size_t _index;

          bool verifyHeader(uint8_t *data, size_t len);

        public:
          ServerSession(HttpServer *_s) : s(_s), result(Result::PENDING), _index(0) {}

          void handle(size_t index, uint8_t *data, size_t len, bool final);
          void respond(AsyncWebServerRequest *request);
        };
      }

      class HttpServer : public Ota, public ArduMower::Modem::Http::Common
      {
      private:
        AsyncWebServer &_server;
        bool _active;
        bool _failed;
        bool _restart;
        uint32_t _restartTime;

        void handleUploadRequest(AsyncWebServerRequest *request);
        void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

        void handlePostRequest(AsyncWebServerRequest *request);
        void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

        void beginUpdate(AsyncWebServerRequest *request, size_t index, uint8_t *data, size_t len, bool final);
        void continueUpdate(AsyncWebServerRequest *request, size_t index, uint8_t *data, size_t len, bool final);

        void loopRestart();

      public:
        HttpServer(ArduMower::Modem::Settings::Settings & settings, AsyncWebServer &server);

        virtual void begin() override;
        virtual void loop() override;
        virtual bool active() override { return _active; };

        void requestRestart();
      };
    }
  }
}