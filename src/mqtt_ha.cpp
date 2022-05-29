#include "mqtt_ha.h"
#include "log.h"
#include <ArduinoJson.h>

using namespace ArduMower::Modem::HomeAssistant;
using namespace ArduMower::Domain::Robot;

float batteryVoltageToLevel(const float v);
String chipIdString();

void Adapter::loop(const uint32_t now)
{
  loopReport(now);
  loopCommand(now);
}

void Adapter::loopCommand(const uint32_t now)
{
  if (commandSpeedPending && cmd.changeSpeed(commandSpeedValue))
    commandSpeedPending = false;
  if (commandMowerEnabledPending && cmd.mowerEnabled(commandMowerEnabledValue))
    commandMowerEnabledPending = false;
}

void Adapter::loopReport(const uint32_t now)
{
  auto *state = source.stateP();
  auto *desiredState = source.desiredStateP();

  bool changes = false;

  if (fabs(state->batteryVoltage - batteryVoltage) > 0.1)
    changes = true;
  else if (state->job != job)
    changes = true;
  else if (desiredState->speed != speed)
    changes = true;
  else if (desiredState->mowerMotorEnabled != mowerEnabled)
    changes = true;

  if (!changes)
    return;

  DynamicJsonDocument o(1024);

  uint8_t roundBatterylevel = (uint8_t)roundf(batteryVoltageToLevel(state->batteryVoltage));
  o["battery_level"] = roundBatterylevel;

  switch (state->job)
  {
  case 0: // idle
    o["state"] = "paused";
    break;
  case 1: // mow
    o["state"] = "cleaning";
    break;
  case 2: // charge
    o["state"] = "docked";
    break;
  case 3: // error
    o["state"] = "error";
    break;
  case 4: // dock
    o["state"] = "returning";
    break;
  }

  if (!desiredState->mowerMotorEnabled)
    o["fan_speed"] = "off";
  else if (desiredState->speed <= 0.25)
    o["fan_speed"] = "min";
  else if (desiredState->speed <= 0.4)
    o["fan_speed"] = "medium";
  else if (desiredState->speed <= 0.50)
    o["fan_speed"] = "high";
  else
    o["fan_speed"] = "max";

  String buffer;
  size_t len = serializeJson(o, buffer);
  if (len < 1)
  {
    Log(ERR, "HomeAssistant::Adapter::loop::serialize-zero");
    return;
  }
  Log(DBG, "HomeAssistant::Adapter::loop::serialize(%u, %s)", len, buffer.c_str());

  if (!tx("/ha/state", buffer))
    return;

  job = state->job;
  batteryVoltage = state->batteryVoltage;
  speed = desiredState->speed;
  mowerEnabled = desiredState->mowerMotorEnabled;
}

void Adapter::onFanSpeedMessage(const String &payload)
{
  if (payload == "off")
  {
    commandMowerEnabledPending = true;
    commandMowerEnabledValue = false;
    return;
  }
  else if (!source.desiredStateP()->mowerMotorEnabled)
  {
    commandMowerEnabledPending = true;
    commandMowerEnabledValue = true;
  }

  float speed;

  if (payload == "min")
    speed = 0.2;
  else if (payload == "medium")
    speed = 0.33;
  else if (payload == "high")
    speed = 0.46;
  else if (payload == "max")
    speed = 0.59;
  else
    return;

  commandSpeedPending = true;
  commandSpeedValue = speed;
}

String DiscoveryDocument::topic()
{
  String result = "homeassistant/vacuum/ardumower-" + chipIdString() + "/config";

  return result;
}

String DiscoveryDocument::toJson(String topicPrefix)
{
  String result;
  DynamicJsonDocument doc(1024);

  String chipIdStr = chipIdString();

  doc["name"] = settings.general.name;
  doc["unique_id"] = chipIdStr;
  doc["~"] = topicPrefix;
  doc["schema"] = "state";
  doc["state_topic"] = "~/ha/state";
  doc["json_attributes_topic"] = "~/props";
  doc["command_topic"] = "~/command";
  doc["set_fan_speed_topic"] = "~/ha/set_fan_speed";
  doc["payload_return_to_base"] = "dock";

  auto features = doc.createNestedArray("supported_features");
  features.add("start");
  features.add("stop");
  features.add("return_home");
  features.add("battery");
  features.add("status");
  features.add("fan_speed");

  auto speeds = doc.createNestedArray("fan_speed_list");
  speeds.add("off");
  speeds.add("min");
  speeds.add("medium");
  speeds.add("high");
  speeds.add("max");

  serializeJson(doc, result);

  return result;
}

float batteryVoltageToLevel(const float v)
{
  const float cell = v / 7;
  const float empty = 3;
  const float full = 4.2;
  if (cell <= empty)
    return 0;
  if (cell >= full)
    return 100;

  const float rel = (cell - empty) / (full - empty);

  return rel * 100.0;
}

String chipIdString()
{
  uint32_t chipId = 0;
  for (auto i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  return String(chipId);
}
