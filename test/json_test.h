#include <AUnit.h>

#include "../src/json.h"
#include "../src/domain.h"

test(json_encode_Properties)
{
  // given
  ArduMower::Domain::Robot::Properties givenInput;
  givenInput.timestamp = 82934;
  givenInput.firmware = "expected firmware";
  givenInput.version = "9.4.2-build.84";

  String expectedResult = "{\"firmware\":\"expected firmware\",\"version\":\"9.4.2-build.84\"}";

  // when
  String actualResult = ArduMower::Domain::Json::encode(givenInput);

  // then
  assertEqual(actualResult, expectedResult);
}
