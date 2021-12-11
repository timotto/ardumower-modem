#pragma once

#include <inttypes.h>

namespace ArduMower
{
  namespace Util
  {
    class Backoff
    {
    private:
      uint32_t min;
      uint32_t max;
      float factor;

      bool active;
      uint32_t last_time;
      uint32_t last_value;

    public:
      Backoff(uint32_t _min, uint32_t _max, float _factor);

      void reset();
      uint32_t next();
    };
  }
}