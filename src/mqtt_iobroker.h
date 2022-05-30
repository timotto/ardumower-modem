#ifndef _IOBROKER_H
#define _IOBROKER_H

#include <Arduino.h>
#include <functional>
#include "settings.h"
#include "domain.h"
#include <MQTT.h>
#include <memory>

namespace ArduMower
{
  namespace Modem
  {
    namespace IOBroker
    {
      class Adapter
      {
      private:

      // ################# DECLARATION ########################

        ArduMower::Modem::Settings::Settings &settings;
        ArduMower::Domain::Robot::StateSource &source;
        ArduMower::Domain::Robot::CommandExecutor &cmd;
        std::function<bool(const String &, const String &)> tx;

        //std::shared_ptr<MQTTClient> iobClient;

        MQTTClient *iobClient;

      public:

      // ################# CONSTRUCTOR ########################

        Adapter(ArduMower::Modem::Settings::Settings &_settings,
                ArduMower::Domain::Robot::StateSource &_source,
                ArduMower::Domain::Robot::CommandExecutor &_cmd,
                std::function<bool(const String &, const String &)> _tx)
            : settings(_settings), source(_source), cmd(_cmd), tx(_tx){};  

        // ################# SETTER METHODS ########################

        //void setMQTTClient(std::shared_ptr<MQTTClient> _client){iobClient = std::move(_client);};
        void setMQTTClient(MQTTClient *_client) {iobClient = _client;};

        bool subscribeTopics();   

        bool createIOBrokerDataPoints();

        bool publishState(ArduMower::Domain::Robot::State::State state);

        // ################# SUPPORT METHODS ########################

        String topic(String postfix);
      };

    }
  }
}
#endif