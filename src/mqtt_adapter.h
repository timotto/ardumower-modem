#ifndef _MQTT_ADAPTER_H
#define _MQTT_ADAPTER_H

#include "backoff.h"
#include "mqtt_ha.h"
#include "mqtt_iobroker.h"
#include "router.h"
#include "domain.h"
#include "settings.h"
#include <Arduino.h>
#include <functional>
#include <inttypes.h>
#include <MQTT.h>
#include <WiFi.h>

namespace ArduMower
{
  namespace Modem
  {
    class MqttAdapter
    {
    private:
      Settings::Settings &settings;
      Router &router;
      ArduMower::Domain::Robot::StateSource &source;
      ArduMower::Domain::Robot::CommandExecutor &cmd;
      HomeAssistant::Adapter ha;
      IOBroker::Adapter      iob;

      WiFiClient net;
      MQTTClient client;
      ArduMower::Util::Backoff backoff;

      void onMqttMessage(String topic, String payload);
      bool connect(const uint32_t now);
      String topic(String postfix);

      void publishState(const uint32_t now);
      void publishProps(const uint32_t now);
      void publishStats(const uint32_t now);

      bool publishTo(const String &topicPostfix, const String &message);

    public:
      MqttAdapter(Settings::Settings &_settings,
                  Router &_router,
                  ArduMower::Domain::Robot::StateSource &_source,
                  ArduMower::Domain::Robot::CommandExecutor &_cmd)
          : settings(_settings), router(_router), source(_source), cmd(_cmd),
            ha(HomeAssistant::Adapter(
                _settings, _source, _cmd,
                std::bind(&MqttAdapter::publishTo, this, std::placeholders::_1, std::placeholders::_2))),
            iob(IOBroker::Adapter(
                _settings, _source, _cmd,
                std::bind(&MqttAdapter::publishTo, this, std::placeholders::_1, std::placeholders::_2))),
            client(MQTTClient(2048)), backoff(ArduMower::Util::Backoff(1000, 6000, 1.2))
      {
      }

      void begin();
      void loop(const uint32_t now);
    };
  }
}
#endif
