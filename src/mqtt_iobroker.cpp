#include "mqtt_iobroker.h"
#include "log.h"
#include <ArduinoJson.h>

using namespace ArduMower::Modem::IOBroker;

float batteryVoltageToLevel2(const float v);

bool Adapter::subscribeTopics()
{

  if (settings.mqtt.iob)
  {
    if (!iobClient->subscribe(topic("/iob/command/set_speed").c_str()))
      return false;
    
    if (!iobClient->subscribe(topic("/iob/command/set_mowMotor_enable").c_str()))
      return false;

    if (!iobClient->subscribe(topic("/iob/command/set_sonar_enable").c_str()))
      return false;

    if (!iobClient->subscribe(topic("/iob/command/set_mowPoint").c_str()))
      return false;

    if (!iobClient->subscribe(topic("/iob/command/set_fixTimeout").c_str()))
      return false;

    if (!iobClient->subscribe(topic("/iob/command/set_finishAndRestart").c_str()))
      return false;

    if (!iobClient->subscribe(topic("/iob/command/set_skipWaypoint").c_str()))
      return false;

    if (!iobClient->subscribe(topic("/iob/error").c_str()))
      return false;
  }
  Log(DBG, "IOBrokerAdapter::subscribeTopics::success");
}

String Adapter::topic(String postfix)
{
  String result = settings.mqtt.prefix;
  result += settings.general.name;
  result += postfix;

  Log(DBG, "IOBrokerAdapter::topic::success");
  return result;
}

bool Adapter::publishState(ArduMower::Domain::Robot::State::State state)
{
    
    char cfloat[12];
    char cint[12];
    dtostrf(state.batteryVoltage, 3, 2, cfloat); 
    if (!iobClient->publish(topic("/iob/stats/battery_voltage").c_str(), cfloat)) return false;

    dtostrf(batteryVoltageToLevel2(state.batteryVoltage), 3, 1, cfloat); 
    if (!iobClient->publish(topic("/iob/stats/battery_level").c_str(), cfloat)) return false;

    itoa(state.job, cint, 10);
    if (!iobClient->publish(topic("/iob/stats/job").c_str(), cint)) return false;

    itoa(state.sensor, cint, 10);
    if (!iobClient->publish(topic("/iob/stats/sensor").c_str(), cint)) return false;

    dtostrf(state.amps, 3, 2, cfloat); 
    if (!iobClient->publish(topic("/iob/stats/amps").c_str(), cfloat)) return false;

    itoa(state.mapCrc, cint, 10);
    if (!iobClient->publish(topic("/iob/stats/map_crc").c_str(), cint)) return false;

    itoa(state.position.mowPointIndex, cint, 10);
    if (!iobClient->publish(topic("/iob/stats/mow_point_index").c_str(), cint)) return false;

    dtostrf(state.target.x, 3, 2, cfloat); 
    if (!iobClient->publish(topic("/iob/stats/position/x").c_str(), cfloat)) return false;

    dtostrf(state.target.y, 3, 2, cfloat); 
    if (!iobClient->publish(topic("/iob/stats/position/y").c_str(), cfloat)) return false;

    dtostrf(state.position.delta, 3, 2, cfloat); 
    if (!iobClient->publish(topic("/iob/stats/position/delta").c_str(), cfloat)) return false;

    itoa(state.position.solution, cint, 10);
    if (!iobClient->publish(topic("/iob/stats/position/solution").c_str(), cint)) return false;

    dtostrf(state.position.age, 3, 2, cfloat); 
    if (!iobClient->publish(topic("/iob/stats/position/age").c_str(), cfloat)) return false;

    dtostrf(state.position.accuracy, 3, 2, cfloat); 
    if (!iobClient->publish(topic("/iob/stats/position/accuracy").c_str(), cfloat)) return false;

    itoa(state.position.visibleSatellites, cint, 10);
    if (!iobClient->publish(topic("/iob/stats/position/visible_satelites").c_str(), cint)) return false;

    itoa(state.position.visibleSatellitesDgps, cint, 10);
    if (!iobClient->publish(topic("/iob/stats/position/visible_satelites_dgps").c_str(), cint)) return false;
    Log(DBG, "IOBrokerAdapter::publishState::success");
}

float batteryVoltageToLevel2(const float v)
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