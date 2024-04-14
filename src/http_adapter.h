#pragma once

#include "router.h"
#include "log.h"
#include "prometheus_impl.h"
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <list>
#include <Arduino.h>

namespace ArduMower
{
  namespace Modem
  {

    namespace Http
    {
      class Metrics
      {
      private:
        ArduMower::Modem::Prometheus::Measurements::Value *status200;
        ArduMower::Modem::Prometheus::Measurements::Value *status400;
        ArduMower::Modem::Prometheus::Measurements::Value *status500;
      public:
        Metrics();
        ~Metrics();

        void countStatusCode(int code);
      };

      class CommandRequest
      {
      private:
        Metrics *_metrics;

      public:
        const uint32_t id;
        int state;
        String httpRequestBody;
        String routerResponse;
        CommandRequest(const uint32_t _id, Metrics *metrics, AsyncWebServerRequest *_request, const uint32_t timeNow);

        uint32_t age(const uint32_t now) { return now - timeReceiveHttpRequest; }
        bool done(const uint32_t now);

        void reject(int code, String message);
        void onRouterResponse(String res);

      private:
        AsyncWebServerRequest *request;
        const uint32_t timeReceiveHttpRequest;
        bool _done;
        void parseHttpRequestBody();
        void readHttpRequestBody();
        void trimHttpRequestBody();
        void validateHttpRequestBody();
        bool timeout(const uint32_t now);
      };
    }

    class HttpAdapter
    {
    private:
      Router &_router;
      AsyncWebServer &_server;
      Http::Metrics *_metrics;
      std::list<Http::CommandRequest *> _queue;
      SemaphoreHandle_t _lock;
      uint32_t requestId;

      void handleCommandRequest(AsyncWebServerRequest *request);
      void handleCORSPreflightRequest(AsyncWebServerRequest *request);
      void apiReboot(AsyncWebServerRequest *request);
      void handleRootRequest(AsyncWebServerRequest *request);

      void enqueueRequest(Http::CommandRequest *req);
      bool queueIsFull();
      void processQueue();
      size_t queueSize();
      void processRequest(Http::CommandRequest *req);
      void handleRouterResponse(const uint32_t id, String response);

    public:
      HttpAdapter(Router &router, AsyncWebServer &server);
      ~HttpAdapter();

      void begin();
      void loop();
    };
  }
}
