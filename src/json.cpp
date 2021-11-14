#include "json.h"
#include <ArduinoJson.h>

using namespace ArduMower::Domain::Robot;
using namespace ArduMower::Domain;

static void encodeInto(JsonObject& doc, State::Point &p)
{
  doc["x"] = p.x;
  doc["y"] = p.y;
}

static void encodeInto(JsonObject& doc, State::Position &p)
{
  doc["x"] = p.x;
  doc["y"] = p.y;
  doc["delta"] = p.delta;
  doc["solution"] = p.solution;
  doc["age"] = p.age;
  doc["accuracy"] = p.accuracy;
  doc["visible_satellites"] = p.visibleSatellites;
  doc["visible_satellites_dgps"] = p.visibleSatellitesDgps;
  doc["mow_point_index"] = p.mowPointIndex;
}

String Json::encode(Properties &p)
{
  String result;
  DynamicJsonDocument doc(1024);

  doc["firmware"] = p.firmware;
  doc["version"] = p.version;

  serializeJson(doc, result);

  return result;
}

String Json::encode(State::State &s)
{
  String result;
  DynamicJsonDocument doc(1024);

  doc["battery_voltage"] = s.batteryVoltage;
  JsonObject pos = doc.createNestedObject("position");
  encodeInto(pos, s.position);
  JsonObject tgt = doc.createNestedObject("target");
  encodeInto(tgt, s.target);
  doc["job"] = s.job;
  doc["sensor"] = s.sensor;
  doc["amps"] = s.amps;
  doc["map_crc"] = s.mapCrc;

  serializeJson(doc, result);
  
  return result;
}

String Json::encode(Stats::Stats &stats)
{
  String result;
  DynamicJsonDocument doc(1024);

  JsonObject dur = doc.createNestedObject("durations");
  dur["idle"] = stats.durations.idle;
  dur["charge"] = stats.durations.charge;
  dur["mow"] = stats.durations.mow;
  dur["mow_float"] = stats.durations.mowFloat;
  dur["mow_fix"] = stats.durations.mowFix;
  dur["mow_invalid"] = stats.durations.mowInvalid;

  JsonObject obs = doc.createNestedObject("obstacles");
  obs["count"] = stats.obstacles.count;
  obs["sonar"] = stats.obstacles.sonar;
  obs["bumper"] = stats.obstacles.bumper;
  obs["gps_motion_low"] = stats.obstacles.gpsMotionLow;

  serializeJson(doc, result);
  
  return result;
}

String Json::encode(DesiredState &d)
{
  String result;
  DynamicJsonDocument doc(1024);

  doc["speed"] = d.speed;
  doc["mower_motor_enabled"] = d.mowerMotorEnabled;
  doc["finish_and_restart"] = d.finishAndRestart;
  doc["op"] = d.op;
  doc["fix_timeout"] = d.fixTimeout;

  serializeJson(doc, result);

  return result;
}