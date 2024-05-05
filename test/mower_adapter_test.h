#include <AUnit.h>
using namespace aunit;

#include "../src/mower_adapter.h"
#include "../src/prometheus.h"

class TestMowerAdapter : public TestOnce
{
protected:
  ArduMower::Modem::MowerAdapter *uut;
  ArduMower::Modem::Settings::Settings settings;
  ArduMower::Modem::Router *router;

  void setup() override;
  void teardown() override;

  void fakeArduMowerOutput(String givenInput);
  String fakeArduMowerInputReadLine();
  static String stripChecksumAndCRLF(String line);
  
  ArduMower::Domain::Robot::Stats::Stats givenAnATTResponseAndTheExpectedStatistics();
  ArduMower::Domain::Robot::State::State givenAnATSResponseAndTheExpectedState();
  ArduMower::Domain::Robot::State::State givenACustomATSResponseAndTheExpectedState();
  ArduMower::Domain::Robot::Properties givenAnATVResponseAndTheExpectedProperties();
};

#define assertEqualProp(a, b, p) assertEqual(a.p, b.p)

testF(TestMowerAdapter, parse_att_response)
{
  auto expected = givenAnATTResponseAndTheExpectedStatistics();

  auto actualResult = uut->stats();

  assertEqualProp(actualResult, expected, durations.idle);
  assertEqualProp(actualResult, expected, durations.charge);
  assertEqualProp(actualResult, expected, durations.mow);
  assertEqualProp(actualResult, expected, durations.mowFix);
  assertEqualProp(actualResult, expected, durations.mowFloat);
  assertEqualProp(actualResult, expected, durations.mowInvalid);

  assertEqualProp(actualResult, expected, recoveries.mowFloatToFix);
  assertEqualProp(actualResult, expected, recoveries.imu);
  assertEqualProp(actualResult, expected, recoveries.mowInvalid);

  assertEqualProp(actualResult, expected, obstacles.count);
  assertEqualProp(actualResult, expected, obstacles.sonar);
  assertEqualProp(actualResult, expected, obstacles.bumper);
  assertEqualProp(actualResult, expected, obstacles.gpsMotionLow);

  assertEqualProp(actualResult, expected, mowDistanceTraveled);
  assertEqualProp(actualResult, expected, mowMaxDgpsAge);
  assertEqualProp(actualResult, expected, tempMin);
  assertEqualProp(actualResult, expected, tempMax);
  assertEqualProp(actualResult, expected, gpsChecksumErrors);
  assertEqualProp(actualResult, expected, dgpsChecksumErrors);
  assertEqualProp(actualResult, expected, maxMotorControlCycleTime);
  assertEqualProp(actualResult, expected, serialBufferSize);
  assertEqualProp(actualResult, expected, freeMemory);
  assertEqualProp(actualResult, expected, resetCause);
  assertEqualProp(actualResult, expected, gpsJumps);

  actualResult.timestamp = expected.timestamp;
  assertTrue(actualResult == expected);
}

testF(TestMowerAdapter, parse_ats_response)
{
  auto expected = givenAnATSResponseAndTheExpectedState();

  auto actualResult = uut->state();

  assertEqualProp(actualResult, expected, batteryVoltage);
  assertEqualProp(actualResult, expected, job);
  assertEqualProp(actualResult, expected, sensor);
  assertEqualProp(actualResult, expected, amps);
  assertEqualProp(actualResult, expected, mapCrc);

  assertEqualProp(actualResult, expected, position.x);
  assertEqualProp(actualResult, expected, position.y);
  assertEqualProp(actualResult, expected, position.delta);
  assertEqualProp(actualResult, expected, position.solution);
  assertEqualProp(actualResult, expected, position.mowPointIndex);
  assertEqualProp(actualResult, expected, position.age);
  assertEqualProp(actualResult, expected, position.accuracy);
  assertEqualProp(actualResult, expected, position.visibleSatellites);
  assertEqualProp(actualResult, expected, position.visibleSatellitesDgps);

  assertEqualProp(actualResult, expected, target.x);
  assertEqualProp(actualResult, expected, target.y);

  actualResult.timestamp = expected.timestamp;
  assertTrue(actualResult == expected);
}

testF(TestMowerAdapter, parse_ats_custom_response)
{
  auto expected = givenACustomATSResponseAndTheExpectedState();

  auto actualResult = uut->state();

  assertEqualProp(actualResult, expected, batteryVoltage);
  assertEqualProp(actualResult, expected, job);
  assertEqualProp(actualResult, expected, sensor);
  assertEqualProp(actualResult, expected, amps);
  assertEqualProp(actualResult, expected, mapCrc);

  assertEqualProp(actualResult, expected, position.x);
  assertEqualProp(actualResult, expected, position.y);
  assertEqualProp(actualResult, expected, position.delta);
  assertEqualProp(actualResult, expected, position.solution);
  assertEqualProp(actualResult, expected, position.mowPointIndex);
  assertEqualProp(actualResult, expected, position.age);
  assertEqualProp(actualResult, expected, position.accuracy);
  assertEqualProp(actualResult, expected, position.visibleSatellites);
  assertEqualProp(actualResult, expected, position.visibleSatellitesDgps);

  assertEqualProp(actualResult, expected, target.x);
  assertEqualProp(actualResult, expected, target.y);

  assertEqualProp(actualResult, expected, temperature);
  assertEqualProp(actualResult, expected, chargingMah);
  assertEqualProp(actualResult, expected, motorMowMah);
  assertEqualProp(actualResult, expected, motorLeftMah);
  assertEqualProp(actualResult, expected, motorRightMah);

  actualResult.timestamp = expected.timestamp;
  assertTrue(actualResult == expected);
}

testF(TestMowerAdapter, parse_atv_response)
{
  auto expected = givenAnATVResponseAndTheExpectedProperties();

  auto actualResult = uut->props();

  assertEqualProp(actualResult, expected, firmware);
  assertEqualProp(actualResult, expected, version);

  actualResult.timestamp = expected.timestamp;
  assertTrue(actualResult == expected);
}

testF(TestMowerAdapter, mowerEnabled)
{
  assertTrue(uut->mowerEnabled(true));
  assertEqual(stripChecksumAndCRLF(fakeArduMowerInputReadLine()), "AT+C,1,-1,-1,-1,-1,-1,-1,-1");

  assertTrue(uut->mowerEnabled(false));
  assertEqual(stripChecksumAndCRLF(fakeArduMowerInputReadLine()), "AT+C,0,-1,-1,-1,-1,-1,-1,-1");
}

testF(TestMowerAdapter, changeSpeed)
{
  assertTrue(uut->changeSpeed(0.5));
  String actualResult = stripChecksumAndCRLF(fakeArduMowerInputReadLine());
  assertEqual(actualResult, "AT+C,-1,-1,0.50,-1,-1,-1,-1,-1");

  assertTrue(uut->changeSpeed(0.21));
  actualResult = stripChecksumAndCRLF(fakeArduMowerInputReadLine());
  assertEqual(actualResult, "AT+C,-1,-1,0.21,-1,-1,-1,-1,-1");

  assertTrue(uut->changeSpeed(17.3));
  actualResult = stripChecksumAndCRLF(fakeArduMowerInputReadLine());
  assertEqual(actualResult, "AT+C,-1,-1,17.30,-1,-1,-1,-1,-1");
}

testF(TestMowerAdapter, start)
{
  assertTrue(uut->start());
  String actualResult = stripChecksumAndCRLF(fakeArduMowerInputReadLine());
  assertEqual(actualResult, "AT+C,-1,1,-1,-1,-1,-1,-1,-1");
}

testF(TestMowerAdapter, stop)
{
  assertTrue(uut->stop());
  String actualResult = stripChecksumAndCRLF(fakeArduMowerInputReadLine());
  assertEqual(actualResult, "AT+C,-1,0,-1,-1,-1,-1,-1,-1");
}

testF(TestMowerAdapter, dock)
{
  assertTrue(uut->dock());
  String actualResult = stripChecksumAndCRLF(fakeArduMowerInputReadLine());
  assertEqual(actualResult, "AT+C,-1,4,-1,-1,-1,-1,-1,-1");
}

testF(TestMowerAdapter, parseEncryptedCommand)
{
  // initialize encryption in MowerAdapter::Encrypt
  // 123456 / 143
  fakeArduMowerOutput("V,Ardumower Sunray,1.0.219,1,143,0x80\r\n");

  bool tmp;
  // AT+C,-1,1,0.1,100,0,-1,-1,1,0x2f
  // 123456 / 143
  uut->drainTx("p$Zr[\\`[`[_]`[`__[_[\\`[\\`[`[_Ha6", tmp);

  assertEqual(uut->desiredState().op, (int)1);
  assertEqual(uut->desiredState().speed, (float)0.1);
  assertEqual(uut->desiredState().fixTimeout, (int)100);
  assertEqual(uut->desiredState().finishAndRestart, false);
}

testF(TestMowerAdapter, parsePlaintextCommand)
{
  // initialize encryption in MowerAdapter::Encrypt
  // 123456 / 143
  fakeArduMowerOutput("V,Ardumower Sunray,1.0.219,1,143,0x80\r\n");

  bool tmp;
  uut->drainTx("AT+C,1,2,0.9,101,1,-1,-1,1,0x0c", tmp);

  assertEqual(uut->desiredState().mowerMotorEnabled, true);
  assertEqual(uut->desiredState().op, (int)2);
  assertEqual(uut->desiredState().speed, (float)0.9);
  assertEqual(uut->desiredState().fixTimeout, (int)101);
  assertEqual(uut->desiredState().finishAndRestart, true);
}

// testbed

void TestMowerAdapter::setup()
{
  drainSerial(Serial1);
  drainSerial(Serial2);

  router = new ArduMower::Modem::Router(Serial2);
  settings = testModemSettings();
  uut = new ArduMower::Modem::MowerAdapter(settings, *router);
  router->begin();
  uut->begin();

  AsyncLooper.start(std::bind(&ArduMower::Modem::Router::loop, router));
}

void TestMowerAdapter::teardown()
{
  AsyncLooper.stop();
  delete uut;
  delete router;
}

void TestMowerAdapter::fakeArduMowerOutput(String givenInput)
{
  Serial1.print(givenInput.c_str());
  Serial1.flush();
  while (Serial2.available())
    delay(100);
  delay(100);
}

String TestMowerAdapter::fakeArduMowerInputReadLine()
{
  const uint32_t timeout = millis() + 1000;
  String line = "";
  while (millis() < timeout)
  {
    while (Serial1.available())
    {
      const char c = Serial1.read();
      line = line + String(c);
      if (!line.endsWith("\r\n")) continue;
      
      return line;
    }
    delay(10);
  }

  return "timeout";
}

String TestMowerAdapter::stripChecksumAndCRLF(String line) 
{
  if (line.endsWith("\r\n")) line = line.substring(0, line.length() - 2);
  if (line.length() >= 5 && line[line.length()-5] == ',') line = line.substring(0, line.length() - 5);

  return line;
}


ArduMower::Domain::Robot::Stats::Stats TestMowerAdapter::givenAnATTResponseAndTheExpectedStatistics()
{
  String response = "T,43,2895,242,34,208,1,28.45,0.28,0,998.00,998.00,55,0,0.02,1024,0,1,4,81751,0,5,0,0,1,0x98\r\n";
  fakeArduMowerOutput(response);

  ArduMower::Domain::Robot::Stats::Stats expected;
  // s += statIdleDuration;
  expected.durations.idle = 43;
  // s += statChargeDuration;
  expected.durations.charge = 2895;
  // s += statMowDuration;
  expected.durations.mow = 242;
  // s += statMowDurationFloat;
  expected.durations.mowFloat = 34;
  // s += statMowDurationFix;
  expected.durations.mowFix = 208;
  // s += statMowFloatToFixRecoveries;
  expected.recoveries.mowFloatToFix = 1;
  // s += statMowDistanceTraveled;
  expected.mowDistanceTraveled = 28.45;
  // s += statMowMaxDgpsAge;
  expected.mowMaxDgpsAge = 0.28;
  // s += statImuRecoveries;
  expected.recoveries.imu = 0;
  // s += statTempMin;
  expected.tempMin = 998.0;
  // s += statTempMax;
  expected.tempMax = 998.0;
  // s += gps.chksumErrorCounter;
  expected.gpsChecksumErrors = 55;
  // s += gps.dgpsChecksumErrorCounter;
  expected.dgpsChecksumErrors = 0;
  // s += statMaxControlCycleTime;
  expected.maxMotorControlCycleTime = 0.02;
  // s += SERIAL_BUFFER_SIZE;
  expected.serialBufferSize = 1024;
  // s += statMowDurationInvalid;
  expected.durations.mowInvalid = 0;
  // s += statMowInvalidRecoveries;
  expected.recoveries.mowInvalid = 1;
  // s += statMowObstacles;
  expected.obstacles.count = 4;
  // s += freeMemory();
  expected.freeMemory = 81751;
  // s += getResetCause();
  expected.resetCause = 0;
  // s += statGPSJumps;
  expected.gpsJumps = 5;
  // s += statMowSonarCounter;
  expected.obstacles.sonar = 0;
  // s += statMowBumperCounter;
  expected.obstacles.bumper = 0;
  // s += statMowGPSMotionTimeoutCounter;
  expected.obstacles.gpsMotionLow = 1;

  return expected;
}

ArduMower::Domain::Robot::State::State TestMowerAdapter::givenAnATSResponseAndTheExpectedState()
{
  String response = "S,28.12,-10.77,-4.55,2.62,1,2,11,0.05,0,-7.47,-6.46,0.02,36,-0.01,29,-26017,0x56\r\n";
  fakeArduMowerOutput(response);

  ArduMower::Domain::Robot::State::State expected;
  // battery.batteryVoltage;
  expected.batteryVoltage = 28.12;
  // s += stateX;
  expected.position.x = -10.77;
  // s += stateY;
  expected.position.y = -4.55;
  // s += stateDelta;
  expected.position.delta = 2.62;
  // s += gps.solution;
  expected.position.solution = 1;
  // s += stateOp;
  expected.job = 2;
  // s += maps.mowPointsIdx;
  expected.position.mowPointIndex = 11;
  // s += (millis() - gps.dgpsAge)/1000.0;
  expected.position.age = 0.05;
  // s += stateSensor;
  expected.sensor = 0;
  // s += maps.targetPoint.x();
  expected.target.x = -7.47;
  // s += maps.targetPoint.y();
  expected.target.y = -6.46;
  // s += gps.accuracy;
  expected.position.accuracy = 0.02;
  // s += gps.numSV;
  expected.position.visibleSatellites = 36;
  // if (stateOp == OP_CHARGE)
  // s += battery.chargingCurrent;
  // else
  // s += motor.motorsSenseLP;
  expected.amps = -0.01;
  // s += gps.numSVdgps;
  expected.position.visibleSatellitesDgps = 29;
  // s += maps.mapCRC;
  expected.mapCrc = -26017;

  return expected;
}

ArduMower::Domain::Robot::State::State TestMowerAdapter::givenACustomATSResponseAndTheExpectedState()
{
  String response = "S,28.12,-10.77,-4.55,2.62,1,2,11,0.05,0,-7.47,-6.46,0.02,36,-0.01,29,-26017,0,-1,0,12043.12,908.12,845.33,8911.12,28.73,0xcc\r\n";
  fakeArduMowerOutput(response);

  ArduMower::Domain::Robot::State::State expected;
  // battery.batteryVoltage;
  expected.batteryVoltage = 28.12;
  // s += stateX;
  expected.position.x = -10.77;
  // s += stateY;
  expected.position.y = -4.55;
  // s += stateDelta;
  expected.position.delta = 2.62;
  // s += gps.solution;
  expected.position.solution = 1;
  // s += stateOp;
  expected.job = 2;
  // s += maps.mowPointsIdx;
  expected.position.mowPointIndex = 11;
  // s += (millis() - gps.dgpsAge)/1000.0;
  expected.position.age = 0.05;
  // s += stateSensor;
  expected.sensor = 0;
  // s += maps.targetPoint.x();
  expected.target.x = -7.47;
  // s += maps.targetPoint.y();
  expected.target.y = -6.46;
  // s += gps.accuracy;
  expected.position.accuracy = 0.02;
  // s += gps.numSV;
  expected.position.visibleSatellites = 36;
  // if (stateOp == OP_CHARGE)
  // s += battery.chargingCurrent;
  // else
  // s += motor.motorsSenseLP;
  expected.amps = -0.01;
  // s += gps.numSVdgps;
  expected.position.visibleSatellitesDgps = 29;
  // s += maps.mapCRC;
  expected.mapCrc = -26017;
  // lateralError
  // s += "0,";
  // timetable
  // s += "-1,0";
  // incompatible to upstream Synray:
  // s += battery.metricChargingMah;
  expected.chargingMah = 12043.12;
  // s += motor.metricMotorLeftMah;
  expected.motorLeftMah = 908.12;
  // s += motor.metricMotorRightMah;
  expected.motorRightMah = 845.33;
  // s += motor.metricMotorMowMah;
  expected.motorMowMah = 8911.12;
  // s += stateTemp;
  expected.temperature = 28.73;


  return expected;
}

ArduMower::Domain::Robot::Properties TestMowerAdapter::givenAnATVResponseAndTheExpectedProperties()
{
  String response = "V,Ardumower Sunray,1.0.219,1,63,0x51\r\n";
  fakeArduMowerOutput(response);

  ArduMower::Domain::Robot::Properties expected;
  expected.firmware = "Ardumower Sunray";
  expected.version = "1.0.219";

  return expected;
}
