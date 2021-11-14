#pragma once

#include "prometheus_impl.h"
#include <functional>
#include <list>

static const char *loopMetricKey = "ardumower_modem_looptime";
static const char *loopAttrKey = "loop";

namespace ArduMower
{
  namespace Modem
  {
    namespace Prometheus
    {
      namespace Looptime
      {
        class Function
        {
        public:
          std::function<void(void)> fn;
          double value;
          Measurements::Reference<double>* metric;

          Function(const char *name, std::function<void(void)> _fn)
              : fn(_fn),
                value(0),
                metric(new Measurements::Reference<double>(
                    loopMetricKey,
                    "%.4f",
                    &value,
                    Attributes(Attribute(loopAttrKey, name)))) {}

          void loop()
          {
            const uint32_t t0 = micros();
            fn();
            const uint32_t dt = micros() - t0;

            value = (0.999 * value) + (0.001 * ((double)dt / 1000.0));
          }
        };
      }

      class LooptimeMonitor
      {
      private:
        std::list<Looptime::Function*> loops;

      public:
        LooptimeMonitor(){};

        void add(const char *name, std::function<void(void)> loop)
        {
          loops.push_back(new Looptime::Function(name, loop));
        }

        void loop()
        {
          for (auto it : loops)
            it->loop();
        }
      };
    }
  }
}