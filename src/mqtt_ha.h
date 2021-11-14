#ifndef _HA_H
#define _HA_H

#include <Arduino.h>
#include <functional>
#include "settings.h"
#include "domain.h"

namespace ArduMower
{
  namespace Modem
  {
    namespace HomeAssistant
    {
      class Adapter
      {
      private:
        ArduMower::Modem::Settings::Settings &settings;
        ArduMower::Domain::Robot::StateSource &source;
        ArduMower::Domain::Robot::CommandExecutor &cmd;
        std::function<bool(const String &, const String &)> tx;

        float batteryVoltage;
        int job;
        float speed;
        bool mowerEnabled;

        bool commandSpeedPending;
        float commandSpeedValue;
        bool commandMowerEnabledPending;
        bool commandMowerEnabledValue;

        void loopReport(const uint32_t now);
        void loopCommand(const uint32_t now);

      public:
        Adapter(ArduMower::Modem::Settings::Settings &_settings,
                ArduMower::Domain::Robot::StateSource &_source,
                ArduMower::Domain::Robot::CommandExecutor &_cmd,
                std::function<bool(const String &, const String &)> _tx)
            : settings(_settings), source(_source), cmd(_cmd), tx(_tx),
              batteryVoltage(0), job(0), speed(0), mowerEnabled(false),
              commandSpeedPending(false), commandSpeedValue(0){};

        void loop(const uint32_t now);
        void onFanSpeedMessage(const String &payload);
      };

      class DiscoveryDocument
      {
      private:
        ArduMower::Modem::Settings::Settings &settings;

      public:
        DiscoveryDocument(ArduMower::Modem::Settings::Settings &_settings)
            : settings(_settings){};

        String topic();
        String toJson(String topicPrefix);
      };
    }
  }
}
#endif
