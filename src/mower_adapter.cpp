#include "mower_adapter.h"
#include "checksum.h"
#include "log.h"
#include "prometheus_util.h"
#include "settings.h"

using namespace ArduMower::Modem;

void processCSVResponse(String res, std::function<void(int, String)> fn);

MowerAdapter::MowerAdapter(Settings::Settings &_settings, Router &_router)
    : settings(_settings), router(_router), sendIsInitialized(false) {}

void MowerAdapter::begin()
{
  enc.setOn(settings.general.encryption);
  enc.setPassword(settings.general.password);
  router.sniffRx(this);
  router.sniffTx(this);
}

void MowerAdapter::drainRx(String line, bool &stop)
{
  parseArduMowerResponse(line);
}

void MowerAdapter::drainTx(String line, bool &stop)
{
  parseArduMowerCommand(line);
}

void MowerAdapter::parseArduMowerResponse(String line)
{
  Log(DBG, "MowerAdapter::parseArduMowerResponse(%s)", line.c_str());

  if (line.length() < 2 + 4)
  // return;
  {
    Log(DBG, "MowerAdapter::parseArduMowerResponse::guard::length(%d)", line.length());
    return;
  }

  if (line[1] != ',')
  // return;
  {
    Log(DBG, "MowerAdapter::parseArduMowerResponse::guard::second-char(%c)", line[1]);
    return;
  }

  char responseTypeCode = line[0];

  String checksumStr = line.substring(line.length() - 5);
  if (!checksumStr.startsWith(",0x"))
  // return;
  {
    Log(DBG, "MowerAdapter::parseArduMowerResponse::guard::checksum-prefix");
    return;
  }
  uint8_t checksum = (checksumStr[3] - '0') << 4 | (checksumStr[4] - '0');

  String payload = line.substring(0, line.length() - 5);

  Checksum chk;
  chk.update(payload);

  // TODO test
  // if (chk.value != checksum) return;

  if (payload.startsWith("S,"))
    parseStateResponse(payload);
  else if (payload.startsWith("V,"))
    parseVersionResponse(payload);
  else if (payload.startsWith("T,"))
    parseStatisticsResponse(payload);
  else
    Log(DBG, "MowerAdapter::parseArduMowerResponse::payload-unknown(%s)", payload.c_str());
}

void MowerAdapter::parseArduMowerCommand(String line)
{
  Log(DBG, "MowerAdapter::parseArduMowerCommand(%s)", line.c_str());

  if (!line.startsWith("AT+"))
  {
    char *buffer = strdup(line.c_str());
    enc.decrypt(buffer, line.length());
    line = buffer;
    free(buffer);
    Log(DBG, "MowerAdapter::parseArduMowerCommand::decrypted(%s)", line.c_str());
  }

  if (line.startsWith("AT+C"))
    parseATCCommand(line);
}

bool MowerAdapter::start()
{
  Log(DBG, "MowerAdapter::command::start");
  return sendCommand("AT+C,-1,1,-1,-1,-1,-1,-1,-1");
}

bool MowerAdapter::stop()
{
  Log(DBG, "MowerAdapter::command::stop");
  return sendCommand("AT+C,-1,0,-1,-1,-1,-1,-1,-1");
}

bool MowerAdapter::dock()
{
  Log(DBG, "MowerAdapter::command::start");
  return sendCommand("AT+C,-1,4,-1,-1,-1,-1,-1,-1");
}

bool MowerAdapter::changeSpeed(float speed)
{
  Log(DBG, "MowerAdapter::command::changeSpeed(%.2f)", speed);
  char buffer[40];
  snprintf(buffer, sizeof(buffer), "AT+C,-1,-1,%.2f,-1,-1,-1,-1,-1", speed);
  String command(buffer);
  return sendCommand(command);
}

bool MowerAdapter::mowerEnabled(bool enabled)
{
  Log(DBG, "MowerAdapter::command::mowerEnabled(%d)", enabled);
  String command = "AT+C," + String(enabled ? "1" : "0") + ",-1,-1,-1,-1,-1,-1,-1";

  return sendCommand(command);
}

bool MowerAdapter::requestVersion()
{
  Log(DBG, "MowerAdapter::requestVersion");
  return sendCommand("AT+V", false);
}

bool MowerAdapter::requestStatus()
{
  Log(DBG, "MowerAdapter::requestStatus");
  if (!assertSendIsInitialized())
    return false;
  return sendCommand("AT+S", true);
}

bool MowerAdapter::requestStats()
{
  Log(DBG, "MowerAdapter::requestStats");
  if (!assertSendIsInitialized())
    return false;
  return sendCommand("AT+T", true);
}

void MowerAdapter::parseStatisticsResponse(String line)
{
  Log(DBG, "MowerAdapter::parseStatisticsResponse");
  const auto now = millis();

  processCSVResponse(line, [&](int index, String val)
                     {
                       switch (index)
                       {
                       case 1:
                         _stats.durations.idle = val.toInt();
                         break;
                       case 2:
                         _stats.durations.charge = val.toInt();
                         break;
                       case 3:
                         _stats.durations.mow = val.toInt();
                         break;
                       case 4:
                         _stats.durations.mowFloat = val.toInt();
                         break;
                       case 5:
                         _stats.durations.mowFix = val.toInt();
                         break;

                       case 6:
                         _stats.recoveries.mowFloatToFix = val.toInt();
                         break;

                       case 7:
                         _stats.mowDistanceTraveled = val.toFloat();
                         break;
                       case 8:
                         _stats.mowMaxDgpsAge = val.toFloat();
                         break;

                       case 9:
                         _stats.recoveries.imu = val.toInt();
                         break;

                       case 10:
                         _stats.tempMin = val.toFloat();
                         break;
                       case 11:
                         _stats.tempMax = val.toFloat();
                         break;

                       case 12:
                         _stats.gpsChecksumErrors = val.toInt();
                         break;
                       case 13:
                         _stats.dgpsChecksumErrors = val.toInt();
                         break;
                       case 14:
                         _stats.maxMotorControlCycleTime = val.toFloat();
                         break;
                       case 15:
                         _stats.serialBufferSize = val.toInt();
                         break;
                       case 16:
                         _stats.durations.mowInvalid = val.toInt();
                         break;
                       case 17:
                         _stats.recoveries.mowInvalid = val.toInt();
                         break;
                       case 18:
                         _stats.obstacles.count = val.toInt();
                         break;
                       case 19:
                         _stats.freeMemory = val.toInt();
                         break;
                       case 20:
                         _stats.resetCause = val.toInt();
                         break;
                       case 21:
                         _stats.gpsJumps = val.toInt();
                         break;
                       case 22:
                         _stats.obstacles.sonar = val.toInt();
                         break;
                       case 23:
                         _stats.obstacles.bumper = val.toInt();
                         break;
                       case 24:
                         _stats.obstacles.gpsMotionLow = val.toInt();
                         break;
                       }
                     });

  _stats.timestamp = now;
}

void MowerAdapter::parseVersionResponse(String line)
{
  Log(DBG, "MowerAdapter::parseVersionResponse");
  const auto now = millis();

  processCSVResponse(line, [&](int index, String val)
                     {
                       // V,Ardumower Sunray,1.0.189,1,73,0x58\r
                       switch (index)
                       {
                       case 1:
                         _props.firmware = val;
                         break;
                       case 2:
                         _props.version = val;
                         break;
                       case 3:
                         enc.setOn(val.toInt() == 1);
                         break;
                       case 4:
                         enc.setChallenge(val.toInt());
                         break;
                       }
                     });

  _props.timestamp = now;
  sendIsInitialized = true;
}

void MowerAdapter::parseStateResponse(String line)
{
  Log(DBG, "MowerAdapter::parseStateResponse");
  const auto now = millis();

  processCSVResponse(line, [&](int index, String val)
                     {
                       switch (index)
                       {
                       case 1:
                         _state.batteryVoltage = val.toFloat();
                         break;
                       case 2:
                         _state.position.x = val.toFloat();
                         break;
                       case 3:
                         _state.position.y = val.toFloat();
                         break;
                       case 4:
                         _state.position.delta = val.toFloat();
                         break;
                       case 5:
                         _state.position.solution = val.toInt();
                         break;
                       case 6:
                         _state.job = val.toInt();
                         break;
                       case 7:
                         _state.position.mowPointIndex = val.toInt();
                         break;
                       case 8:
                         _state.position.age = val.toFloat();
                         break;
                       case 9:
                         _state.sensor = val.toInt();
                         break;

                       case 10:
                         _state.target.x = val.toFloat();
                         break;
                       case 11:
                         _state.target.y = val.toFloat();
                         break;

                       case 12:
                         _state.position.accuracy = val.toFloat();
                         break;
                       case 13:
                         _state.position.visibleSatellites = val.toInt();
                         break;
                       case 14:
                         _state.amps = val.toFloat();
                         break;
                       case 15:
                         _state.position.visibleSatellitesDgps = val.toInt();
                         break;
                       case 16:
                         _state.mapCrc = val.toInt();
                         break;
                       
                       // incompatible to sunray upstream
                       case 19:
                         _state.chargingMah = val.toFloat();
                         break;
                       case 20:
                         _state.motorLeftMah = val.toFloat();
                         break;
                       case 21:
                         _state.motorRightMah = val.toFloat();
                         break;
                       case 22:
                         _state.motorMowMah = val.toFloat();
                         break;
                       case 23:
                         _state.temperature = val.toFloat();
                         break;
                       }
                     });

  _state.timestamp = now;
}

void MowerAdapter::parseATCCommand(String line)
{
  Log(DBG, "MowerAdapter::parseATCCommand");

  processCSVResponse(
      line,
      [&](int index, String val)
      {
        if (val.toInt() == -1)
          return;

        switch (index)
        {
        case 1:
          _desiredState.mowerMotorEnabled = val.toInt() == 1;
          break;
        case 2:
          _desiredState.op = val.toInt();
          break;
        case 3:
          _desiredState.speed = val.toFloat();
          break;
        case 4:
          _desiredState.fixTimeout = val.toInt();
          break;
        case 5:
          _desiredState.finishAndRestart = val.toInt() == 1;
          break;
        case 6:
          // setMowingPointPercent
          break;
        case 7:
          // skipNextMowingPoint
          break;
        case 8:
          // sonarEnabled
          break;
        }
      });
}

bool MowerAdapter::assertSendIsInitialized()
{
  if (sendIsInitialized)
    return true;

  const uint32_t now = millis();
  static uint32_t next_time = 0;
  if (now < next_time)
    return false;
  next_time = now + 1000;

  Log(DBG, "MowerAdapter::assertSendIsInitialized::request-version");

  if (!requestVersion())
    return false;

  Log(DBG, "MowerAdapter::assertSendIsInitialized::version-requested");

  return false;
}

bool MowerAdapter::sendCommand(String command, bool encrypt)
{
  Checksum chk;
  chk.update(command);

  char *buffer;
  asprintf(&buffer, "%s,0x%02x", command.c_str(), chk.value());

  if (encrypt)
    enc.encrypt(buffer, strlen(buffer));

  auto result = router.sendWithoutResponse(buffer);
  free(buffer);

  return result;
}

void processCSVResponse(String res, std::function<void(int, String)> fn)
{
  int index = -1;

  while (res.length() > 0)
  {
    index++;

    const auto delimiter = res.indexOf(",");
    String val = delimiter == -1 ? res : res.substring(0, delimiter);

    fn(index, val);

    if (delimiter != -1)
      res = res.substring(delimiter + 1);
    else
      res = "";
  }
}
