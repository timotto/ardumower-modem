#include <AUnit.h>

#include "../src/backoff.h"

test(backoff_first_is_min) {
  uint32_t givenMin = 200;
  uint32_t expectedValue = givenMin;

  ArduMower::Util::Backoff uut(givenMin, 10000, 2);
  uint32_t actualResult = uut.next();

  assertEqual(actualResult, expectedValue);
}

test(backoff_grows_by_factor_per_retry) {
  uint32_t givenMin = 100;
  float givenFactor = 1.5;

  ArduMower::Util::Backoff uut(givenMin, 10000, givenFactor);

  assertEqual(uut.next(), (uint32_t)100);
  assertEqual(uut.next(), (uint32_t)150);
  assertEqual(uut.next(), (uint32_t)225);
  assertEqual(uut.next(), (uint32_t)337);
}

test(backoff_does_not_grow_beyond_max) {
  uint32_t givenMin = 100;
  uint32_t givenMax = 1000;
  float givenFactor = 3;

  ArduMower::Util::Backoff uut(givenMin, givenMax, givenFactor);

  assertEqual(uut.next(), (uint32_t)100);
  assertEqual(uut.next(), (uint32_t)300);
  assertEqual(uut.next(), (uint32_t)900);
  assertEqual(uut.next(), (uint32_t)1000);
  assertEqual(uut.next(), (uint32_t)1000);
}
