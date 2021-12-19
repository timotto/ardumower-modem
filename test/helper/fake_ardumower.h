#pragma once

#include "fake_ardumower_h.h"
#include "../../src/reader.h"
#include "../../src/encrypt.h"
#include "../../src/log.h"

FakeArduMowerClass FakeArduMower;

void fakeArduMowerTask(void *p)
{
  while (true)
  {
    FakeArduMower.loop();
    delay(1);
  }
}

void setup_fake_ardumower()
{
  xTaskCreatePinnedToCore(fakeArduMowerTask, "fakeArduMowerTask", 4096, NULL, 1, NULL, 0);
}
