#ifndef _PROMETHEUS_ADAPTER_H
#define _PROMETHEUS_ADAPTER_H

#include "domain.h"
#include "prometheus.h"
#include "prometheus_util.h"
#include "settings.h"
#include <inttypes.h>
#include <ESPAsyncWebServer.h>

namespace ArduMower
{
  namespace Modem
  {
    namespace Prometheus
    {
      class Adapter
      {
      private:
        Settings::Settings &_settings;
        AsyncWebServer &_server;
        ArduMower::Domain::Robot::StateSource &_source;
        ArduMower::Domain::Robot::CommandExecutor &_cmd;

        void metrics(AsyncWebServerRequest *request);

      public:
        Adapter(Settings::Settings &settings, 
                AsyncWebServer &server,
                ArduMower::Domain::Robot::StateSource &source,
                ArduMower::Domain::Robot::CommandExecutor &cmd);

        void begin();
      };

      class MetricLine
      {
      public:
        String key;
        std::list<String> attrs;
        double value;

        MetricLine() {}

        String line()
        {
          String result = key;
          if (!attrs.empty())
          {
            result += "{";
            for (auto it : attrs)
            {
              result += it;
              result += ",";
            }
            result += "}";
          }
          result += " ";
          result += value;
        }
      };
    }
  }
}

#endif
