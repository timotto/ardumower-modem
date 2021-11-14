#pragma once

#include "prometheus.h"
#include <Arduino.h>

namespace ArduMower
{
  namespace Modem
  {
    namespace Prometheus
    {
      namespace Measurements
      {
        class Value : ArduMower::Modem::Prometheus::Measurement
        {
        public:
          uint32_t value;
          Value(const char *key, ArduMower::Modem::Prometheus::Attributes attrs = ArduMower::Modem::Prometheus::Attributes())
              : ArduMower::Modem::Prometheus::Measurement(key, attrs), value(0) {}
          void inc() { value++; }
          void set(uint32_t v) { value = v; }

        protected:
          virtual unsigned int writeValue(char *buffer, unsigned int size) override
          {
            return snprintf(buffer, size, "%u", value);
          }
        };

        template <class T>
        class Reference : ArduMower::Modem::Prometheus::Measurement
        {
        private:
          const char *_fmt;
          T *_value;

        public:
          Reference(const char *key, const char *fmt, T *value, ArduMower::Modem::Prometheus::Attributes attrs = ArduMower::Modem::Prometheus::Attributes())
              : ArduMower::Modem::Prometheus::Measurement(key, attrs),
                _fmt(fmt), _value(value) {}

        protected:
          virtual unsigned int writeValue(char *buffer, unsigned int size) override
          {
            return snprintf(buffer, size, _fmt, *_value);
          }
        };
      }
    }
  }
}