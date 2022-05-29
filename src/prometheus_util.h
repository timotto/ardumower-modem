#pragma once

#include "domain.h"

namespace ArduMower
{
  namespace Modem
  {
    namespace Prometheus
    {
      void registerSystemProperties();
      void registerRobotProperties(ArduMower::Domain::Robot::Properties *p);
      void registerRobotState(ArduMower::Domain::Robot::State::State *s);
      void registerRobotStatistics(ArduMower::Domain::Robot::Stats::Stats *s);
    }
  }
}