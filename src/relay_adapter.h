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

      bool loopConnection();
      void loopTransfer();

      void onMessageCallback(websockets::WebsocketsMessage message);
      void onEventsCallback(websockets::WebsocketsEvent event, String data);

      unsigned int writeCount(char *buffer, unsigned int size);

    public:
      RelayAdapter(Settings::Settings &s, Router &r);

      void begin();
      void loop();

      virtual bool isConnected() override;
    };
  }
}
