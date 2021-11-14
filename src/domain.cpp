#include "domain.h"

using namespace ArduMower::Domain::Robot;

#define same(other, prop) (prop == other.prop)

bool Properties::operator==(const Properties &other)
{
  return same(other, timestamp) && same(other, firmware) && same(other, version);
}

bool Stats::Stats::operator==(const Stats &other)
{
  return same(other, timestamp) && same(other, durations) && same(other, recoveries) && same(other, obstacles) && same(other, mowDistanceTraveled) && same(other, mowMaxDgpsAge) && same(other, tempMin) && same(other, tempMax) && same(other, gpsChecksumErrors) && same(other, dgpsChecksumErrors) && same(other, maxMotorControlCycleTime) && same(other, serialBufferSize) && same(other, freeMemory) && same(other, resetCause) && same(other, gpsJumps);
}

bool Stats::Durations::operator==(const Durations &other)
{
  return same(other, idle) && same(other, charge) && same(other, mow) && same(other, mowFloat) && same(other, mowFix) && same(other, mowInvalid);
}

bool Stats::Recoveries::operator==(const Recoveries &other)
{
  return same(other, mowFloatToFix) && same(other, imu) && same(other, mowInvalid);
}

bool Stats::Obstacles::operator==(const Obstacles &other)
{
  return same(other, count) && same(other, sonar) && same(other, bumper) && same(other, gpsMotionLow);
}

bool State::State::operator==(const State &other)
{
  return same(other, timestamp) && same(other, batteryVoltage) && same(other, position) && same(other, target) && same(other, job) && same(other, sensor) && same(other, amps) && same(other, mapCrc);
}

bool State::Position::operator==(const Position &other)
{
  return Point::operator==(other) && same(other, delta) && same(other, solution) && same(other, age) && same(other, accuracy) && same(other, visibleSatellites) && same(other, visibleSatellitesDgps) && same(other, mowPointIndex);
}

bool State::Point::operator==(const Point &other)
{
  return same(other, x) && same(other, y);
}
