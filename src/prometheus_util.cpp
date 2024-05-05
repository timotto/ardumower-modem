#include "prometheus.h"
#include "prometheus_util.h"
#include "prometheus_impl.h"
#include <Arduino.h>

using namespace ArduMower::Modem::Prometheus::Measurements;

#define addPropMetricA(o, prop, key, fmt, a) \
    new CallbackMeasurement(\
      key,\
      [=](char *buffer, unsigned int size) {\
        return snprintf(buffer, size, fmt, o->prop);\
      },\
      a\
    );
  

#define addPropMetric(o, prop, key, fmt) addPropMetricA(o, prop, key, fmt, Attributes())

#define oneAttr(k,v) Attributes(Attribute(k,v))

#define addFnMetric(key, fn, a) \
  new CallbackMeasurement(key, unsignedLongCallback([](){return fn();}), a);

std::function<unsigned int(char*, unsigned int)> unsignedLongCallback(std::function<uint32_t()> cb)
{
  return [=](char*buffer, unsigned int size)
  {
    return snprintf(buffer, size, "%u", cb());
  };
}

static const char* keyModemHeap = "ardumower_modem_heap";
static const char* keyRobotRuntime = "ardumower_robot_runtime";
static const char* keyRobotMowtime = "ardumower_robot_mowtime";
static const char* keyRobotObstacles = "ardumower_robot_obstacles";

static const char* keyAttrType = "type";
static const char* keyAttrActivity = "activity";
static const char* keyAttrPosition = "position";

namespace ArduMower
{
  namespace Modem
  {
    namespace Prometheus
    {
      void registerSystemProperties()
      {
        addFnMetric("ardumower_modem_total_runtime", millis, Attributes());
        addFnMetric(keyModemHeap, ESP.getHeapSize, Attributes(Attribute(keyAttrType, "total")));
        addFnMetric(keyModemHeap, ESP.getFreeHeap, Attributes(Attribute(keyAttrType, "free")));
        addFnMetric(keyModemHeap, ESP.getMinFreeHeap, Attributes(Attribute(keyAttrType, "min_free")));
        addFnMetric(keyModemHeap, ESP.getMaxAllocHeap, Attributes(Attribute(keyAttrType, "max_alloc")));
      }

      void registerRobotProperties(ArduMower::Domain::Robot::Properties *p)
      {
        // new CallbackMeasurement("")
      }

      void registerRobotState(ArduMower::Domain::Robot::State::State *s)
      {
        new Reference<float>("ardumower_robot_battery_voltage", "%.2f", &s->batteryVoltage);
        new Reference<float>("ardumower_robot_amps", "%.2f", &s->amps);
        new Reference<int>("ardumower_robot_job", "%d", &s->job);
        new Reference<int>("ardumower_robot_sensor", "%d", &s->sensor);

        new Reference<float>("ardumower_robot_gps_age", "%f", &s->position.age);
        new Reference<float>("ardumower_robot_gps_accuracy", "%f", &s->position.accuracy);
        new Reference<int>("ardumower_robot_gps_solution", "%d", &s->position.solution);
        new Reference<int>("ardumower_robot_gps_sat_visible", "%d", &s->position.visibleSatellites);
        new Reference<int>("ardumower_robot_gps_sat_visible_dgps", "%d", &s->position.visibleSatellitesDgps);

        new Reference<float>("ardumower_robot_temperature", "%.2f", &s->temperature);
        new Reference<float>("ardumower_robot_mah", "%.2f", &s->chargingMah, oneAttr(keyAttrType, "charge"));
        new Reference<float>("ardumower_robot_mah", "%.2f", &s->motorMowMah, oneAttr(keyAttrType, "mow"));
        new Reference<float>("ardumower_robot_mah", "%.2f", &s->motorLeftMah, oneAttr(keyAttrType, "left"));
        new Reference<float>("ardumower_robot_mah", "%.2f", &s->motorRightMah, oneAttr(keyAttrType, "right"));
      }

      void registerRobotStatistics(ArduMower::Domain::Robot::Stats::Stats *s)
      {
        new Reference<float>("ardumower_robot_mow_distance", "%.2f", &s->mowDistanceTraveled);
        new Reference<uint32_t>("ardumower_robot_free_memory", "%u", &s->freeMemory);
        new Reference<uint32_t>("ardumower_robot_gps_jumps", "%u", &s->gpsJumps);
        new Reference<uint32_t>("ardumower_robot_gps_checksum_errors", "%u", &s->gpsChecksumErrors);

        new Reference<uint32_t>(keyRobotRuntime, "%u", &s->durations.idle, oneAttr(keyAttrActivity, "idle"));
        new Reference<uint32_t>(keyRobotRuntime, "%u", &s->durations.charge, oneAttr(keyAttrActivity, "charge"));
        new Reference<uint32_t>(keyRobotRuntime, "%u", &s->durations.mow, oneAttr(keyAttrActivity, "mow"));
        
        new Reference<uint32_t>(keyRobotMowtime, "%u", &s->durations.mowFix, oneAttr(keyAttrPosition, "fix"));
        new Reference<uint32_t>(keyRobotMowtime, "%u", &s->durations.mowFloat, oneAttr(keyAttrPosition, "float"));
        new Reference<uint32_t>(keyRobotMowtime, "%u", &s->durations.mowInvalid, oneAttr(keyAttrPosition, "invalid"));

        new Reference<uint32_t>(keyRobotObstacles, "%u", &s->obstacles.count, oneAttr(keyAttrType, "all"));
        new Reference<uint32_t>(keyRobotObstacles, "%u", &s->obstacles.sonar, oneAttr(keyAttrType, "sonar"));
        new Reference<uint32_t>(keyRobotObstacles, "%u", &s->obstacles.bumper, oneAttr(keyAttrType, "bumper"));
        new Reference<uint32_t>(keyRobotObstacles, "%u", &s->obstacles.gpsMotionLow, oneAttr(keyAttrType, "gps_motion_low"));
      }
    }
  }
}
