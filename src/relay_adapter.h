#pragma once

#include <ArduinoWebsockets.h>
#include "api.h"
#include "backoff.h"
#include "prometheus.h"
#include "reader.h"
#include "router.h"
#include "settings.h"

namespace ArduMower
{
  namespace Modem
  {
    class RelayAdapter : public ArduMower::Modem::Api::Relay
    {
    private:
      Settings::Settings &settings;
      Router &router;
      unsigned int count;
      ArduMower::Modem::Prometheus::CallbackMeasurement *countMetric;

      websockets::WebsocketsClient client;
      bool connected;
      ArduMower::Util::Backoff backoff;

      String sendToRelay;
      String sendToMower;
      uint32_t lastConnect, lastPing, lastPong;
      float rtt;
      int _connectionCount;
      float _connectionDuration;

      void setupAuthorizationHeader();
      bool loopConnection();
      void loopTransfer();
      void loopPing();

      void onRouterResponse(String res, int err);

      void onMessageCallback(websockets::WebsocketsMessage message);
      void onEventsCallback(websockets::WebsocketsEvent event, String data);

      unsigned int writeCount(char *buffer, unsigned int size);

      uint32_t pingInterval();

    public:
      RelayAdapter(Settings::Settings &s, Router &r);

      void begin();
      void loop();

      virtual bool isConnected() override;
      virtual float pingRTT() override;
      virtual int connectionCount() override;
      virtual uint32_t connectionTime() override;
      virtual float connectionDuration() override;
    };
  }
}
