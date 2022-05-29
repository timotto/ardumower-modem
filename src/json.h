#pragma once

#include "domain.h"
#include <Arduino.h>

namespace ArduMower
{
  namespace Domain
  {
    class Json
    {
    public:
      static String encode(ArduMower::Domain::Robot::Properties &props);
      static String encode(ArduMower::Domain::Robot::State::State &state);
      static String encode(ArduMower::Domain::Robot::Stats::Stats &stats);
      static String encode(ArduMower::Domain::Robot::DesiredState &desiredState);
    };
  }
}
