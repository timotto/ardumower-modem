#ifndef _IOBROKER_H
#define _IOBROKER_H

#include <Arduino.h>
#include <functional>
#include "settings.h"
#include "domain.h"
#include <MQTT.h>

namespace ArduMower
{
  namespace Modem
  {
    namespace IOBroker
    {
      class Adapter
      {
      private:
        ArduMower::Modem::Settings::Settings &settings;
        ArduMower::Domain::Robot::StateSource &source;
        ArduMower::Domain::Robot::CommandExecutor &cmd;
        std::function<bool(const String &, const String &)> tx;

        MQTTClient &client;

      public:
        Adapter(ArduMower::Modem::Settings::Settings &_settings,
                ArduMower::Domain::Robot::StateSource &_source,
                ArduMower::Domain::Robot::CommandExecutor &_cmd,
                std::function<bool(const String &, const String &)> _tx)
            : settings(_settings), source(_source), cmd(_cmd), tx(_tx){};  

        setMQTTClient(MQTTClient &_client):client(_client){};    
      };

    }
  }
}
#endif