#include <AUnit.h>

#include "../src/checksum.h"

test(checksum_calculate) {
  String givenInput = "V,Ardumower Sunray,1.0.189,1,73";
  unsigned char expectedChecksum = 0x58;

  ArduMower::Checksum uut;
  uut.update(givenInput);
  assertEqual(uut.value(), expectedChecksum);
}
