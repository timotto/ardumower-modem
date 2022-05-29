#pragma once

#include <Arduino.h>
#include "../../src/domain.h"

class MockMowerAdapter
    : public ArduMower::Domain::Robot::StateSource,
      public ArduMower::Domain::Robot::CommandExecutor

{
public:
  ArduMower::Domain::Robot::State::State _state;
  ArduMower::Domain::Robot::Stats::Stats _stats;
  ArduMower::Domain::Robot::Properties _props;
  ArduMower::Domain::Robot::DesiredState _desiredState;
  MockMowerAdapter()
  {
    const uint32_t now = millis();
    _state.timestamp = now;
    _stats.timestamp = now;
    _props.timestamp = now;
  }

  virtual ArduMower::Domain::Robot::State::State state() override { return _state; }
  virtual ArduMower::Domain::Robot::Stats::Stats stats() override { return _stats; }
  virtual ArduMower::Domain::Robot::Properties props() override { return _props; }
  virtual ArduMower::Domain::Robot::DesiredState desiredState() override { return _desiredState; }
  virtual ArduMower::Domain::Robot::State::State *stateP() override { return &_state; }
  virtual ArduMower::Domain::Robot::Stats::Stats *statsP() override { return &_stats; }
  virtual ArduMower::Domain::Robot::Properties *propsP() override { return &_props; }
  virtual ArduMower::Domain::Robot::DesiredState *desiredStateP() override { return &_desiredState; }

  virtual bool start() override { return true; }
  virtual bool stop() override { return true; }
  virtual bool dock() override { return true; }
  virtual bool changeSpeed(float speed) { return true; };
  virtual bool mowerEnabled(bool enabled) { return true; }

  virtual bool requestVersion() override { return true; }
  virtual bool requestStatus() override { return true; }
  virtual bool requestStats() override { return true; }
};
