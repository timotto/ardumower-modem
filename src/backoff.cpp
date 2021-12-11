#include "backoff.h"

using namespace ArduMower::Util;

Backoff::Backoff(uint32_t _min, uint32_t _max, float _factor)
    : factor(_factor), min(_min), max(_max),
      active(false), last_time(0), last_value(0)
{
}

void Backoff::reset()
{
  active = false;
  last_time = 0;
  last_value = 0;
}

uint32_t Backoff::next()
{
  if (!active)
  {
    active = true;
    last_value = min;

    return min;
  }

  uint32_t next_value = last_value * factor;
  if (next_value > max)
    next_value = max;

  last_value = next_value;

  return next_value;
}
