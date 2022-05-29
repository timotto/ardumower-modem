#pragma once

#include "../../src/api.h"

class FakeArduinoESP : public ArduMower::Modem::Api::OS
{
private:
  int restartCount;

public:
  FakeArduinoESP() : restartCount(0) {}

  int restarts() { return restartCount; }

  virtual void restart() override
  {
    restartCount++;
  }
};
