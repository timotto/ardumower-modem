#include "mqtt_iobroker.h"
#include "log.h"
#include <ArduinoJson.h>

using namespace ArduMower::Modem::IOBroker;

float batteryVoltageToLevel2(const float v);

bool Adapter::subscribeTopics()
{

  if (settings.mqtt.iob)
  {
    if (!iobClient->subscribe(topicCreate("/iob/command/set_speed").c_str()))
      return false;
    
    if (!iobClient->subscribe(topicCreate("/iob/command/set_mowMotor_enable").c_str()))
      return false;

    if (!iobClient->subscribe(topicCreate("/iob/command/set_sonar_enable").c_str()))
      return false;

    if (!iobClient->subscribe(topicCreate("/iob/command/set_mowPoint").c_str()))
      return false;

    if (!iobClient->subscribe(topicCreate("/iob/command/set_fixTimeout").c_str()))
      return false;

    if (!iobClient->subscribe(topicCreate("/iob/command/set_finishAndRestart").c_str()))
      return false;

    if (!iobClient->subscribe(topicCreate("/iob/command/set_skipWaypoint").c_str()))
      return false;
    
    if (!iobClient->subscribe(topicCreate("/iob/command/set_mower_start").c_str()))
      return false;

    if (!iobClient->subscribe(topicCreate("/iob/command/set_mower_stop").c_str()))
      return false;
    
    if (!iobClient->subscribe(topicCreate("/iob/command/set_mower_dock").c_str()))
      return false;

    if (!iobClient->subscribe(topicCreate("/iob/error").c_str()))
      return false;
  }
  Log(DBG, "IOBrokerAdapter::subscribeTopics::success");
}

String Adapter::topicCreate(String postfix)
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
    if (!iobClient->publish(topicCreate("/iob/stats/battery_voltage").c_str(), cfloat)) return false;

    dtostrf(batteryVoltageToLevel2(state.batteryVoltage), 3, 1, cfloat); 
    if (!iobClient->publish(topicCreate("/iob/stats/battery_level").c_str(), cfloat)) return false;

    itoa(state.job, cint, 10);
    switch (state.job)
    {
      case 0:
        if (!iobClient->publish(topicCreate("/iob/stats/job").c_str(), "idle")) return false;
        break;
      case 1:
        if (!iobClient->publish(topicCreate("/iob/stats/job").c_str(), "mow")) return false;
        break;
      case 2:
        if (!iobClient->publish(topicCreate("/iob/stats/job").c_str(), "charge")) return false;
        break;
      case 3:
        if (!iobClient->publish(topicCreate("/iob/stats/job").c_str(), "error")) return false;
        break;
      case 4:
        if (!iobClient->publish(topicCreate("/iob/stats/job").c_str(), "dock")) return false;
        break;
      default:
        if (!iobClient->publish(topicCreate("/iob/stats/job").c_str(), cint)) return false;
        break; 
    }    

    itoa(state.sensor, cint, 10);
    if (!iobClient->publish(topicCreate("/iob/stats/sensor").c_str(), cint)) return false;

    dtostrf(state.amps, 3, 2, cfloat); 
    if (!iobClient->publish(topicCreate("/iob/stats/amps").c_str(), cfloat)) return false;

    itoa(state.mapCrc, cint, 10);
    if (!iobClient->publish(topicCreate("/iob/stats/map_crc").c_str(), cint)) return false;

    itoa(state.position.mowPointIndex, cint, 10);
    if (!iobClient->publish(topicCreate("/iob/stats/mow_point_index").c_str(), cint)) return false;

    dtostrf(state.target.x, 3, 2, cfloat); 
    if (!iobClient->publish(topicCreate("/iob/stats/position/x").c_str(), cfloat)) return false;

    dtostrf(state.target.y, 3, 2, cfloat); 
    if (!iobClient->publish(topicCreate("/iob/stats/position/y").c_str(), cfloat)) return false;

    dtostrf(state.position.delta, 3, 2, cfloat); 
    if (!iobClient->publish(topicCreate("/iob/stats/position/delta").c_str(), cfloat)) return false;

    itoa(state.position.solution, cint, 10);
    switch (state.position.solution)
    {
      case 0:
        if (!iobClient->publish(topicCreate("/iob/stats/position/solution").c_str(), "invalid")) return false;
        
      case 1:
        if (!iobClient->publish(topicCreate("/iob/stats/position/solution").c_str(), "float")) return false;
        break;
      case 2:
        if (!iobClient->publish(topicCreate("/iob/stats/position/solution").c_str(), "fix")) return false;
        break;
    }
 
    dtostrf(state.position.age, 3, 2, cfloat); 
    if (!iobClient->publish(topicCreate("/iob/stats/position/age").c_str(), cfloat)) return false;

    dtostrf(state.position.accuracy, 3, 2, cfloat); 
    if (!iobClient->publish(topicCreate("/iob/stats/position/accuracy").c_str(), cfloat)) return false;

    itoa(state.position.visibleSatellites, cint, 10);
    if (!iobClient->publish(topicCreate("/iob/stats/position/visible_satelites").c_str(), cint)) return false;

    itoa(state.position.visibleSatellitesDgps, cint, 10);
    if (!iobClient->publish(topicCreate("/iob/stats/position/visible_satelites_dgps").c_str(), cint)) return false;
    Log(DBG, "IOBrokerAdapter::publishState::success");
}

bool Adapter::evaluateMessage(String topic, String payload)
{
  
  // ############# set Waypoint #############

  if (topic.endsWith("set_mowPoint"))
  {    
    float value = payload.toFloat();

    if ( 0 <= value && value <= 1)
    {
        cmd.setWaypoint(value);
        if (!iobClient->publish(topicCreate("/iob/error").c_str(), "set waypoint success")) return false;
    } 
    else
    {
        if (!iobClient->publish(topicCreate("/iob/error").c_str(), "value for waypoint is not between 0 and 1")) return false;
    }
  }

  // ############# set mower start #############

  if (topic.endsWith("set_mower_start") && payload == "true")
  {
      cmd.start();
      if (!iobClient->publish(topicCreate("/iob/command/set_mower_start").c_str(), "false")) return false;
      if (!iobClient->publish(topicCreate("/iob/error").c_str(), "mower start success")) return false;
  }

  // ############# set mower stop #############

  if (topic.endsWith("set_mower_stop") && payload == "true")
  {
      cmd.stop();
      if (!iobClient->publish(topicCreate("/iob/command/set_mower_stop").c_str(), "false")) return false;
      if (!iobClient->publish(topicCreate("/iob/error").c_str(), "mower stop success")) return false;
  }

  // ############# set mower dock #############

  if (topic.endsWith("set_mower_dock") && payload == "true")
  {
      cmd.dock();
      if (!iobClient->publish(topicCreate("/iob/command/set_mower_dock").c_str(), "false")) return false;
      if (!iobClient->publish(topicCreate("/iob/error").c_str(), "mower dock success")) return false;
  }

  // ############# set finish and restart #############

  if (topic.endsWith("set_FinishAndRestart"))
  {
      if (payload == "true") cmd.finishAndRestartEnabled(true);
                        else cmd.finishAndRestartEnabled(false);

      if (!iobClient->publish(topicCreate("/iob/command/set_FinishAndRestart").c_str(), "")) return false;
      if (!iobClient->publish(topicCreate("/iob/error").c_str(), " set finish and restart success")) return false;
  }

  // ############# set fix timeout #############

  if (topic.endsWith("set_fixTimeout"))
  {    
    int value = payload.toInt();

    if ( 0 <= value && value <= 300)
    {
        cmd.setFixTimeout(value);
        if (!iobClient->publish(topicCreate("/iob/error").c_str(), "set fix Timeout success")) return false;
    } 
    else
    {
        if (!iobClient->publish(topicCreate("/iob/error").c_str(), "value for fix timeout is not between 0 and 300")) return false;
    }
  }

  // ############# set mower motor enabled #############

  if (topic.endsWith("set_mowMotor_enable"))
  {
      if (payload == "true") cmd.mowerEnabled(true);
                        else cmd.mowerEnabled(false);

      if (!iobClient->publish(topicCreate("/iob/command/set_mowMotor_enable").c_str(), "")) return false;
      if (!iobClient->publish(topicCreate("/iob/error").c_str(), " set mow motor state success")) return false;
  }

  // ############# set skip Waypoint #############

  if (topic.endsWith("set_skipWaypoint") && payload == "true")
  {
      cmd.skipWaypoint();
      if (!iobClient->publish(topicCreate("/iob/command/set_mower_dock").c_str(), "false")) return false;
      if (!iobClient->publish(topicCreate("/iob/error").c_str(), "skip Waypoint success")) return false;
  }

  // ############# set sonar enabled #############

  if (topic.endsWith("set_sonar_enable"))
  {
      if (payload == "true") cmd.sonarEnabled(true);
                        else cmd.sonarEnabled(false);

      if (!iobClient->publish(topicCreate("/iob/command/set_sonar_enable").c_str(), "")) return false;
      if (!iobClient->publish(topicCreate("/iob/error").c_str(), " set sonar state success")) return false;
  }

  // ############# set speed #############

  if (topic.endsWith("set_speed"))
  {    
    float value = payload.toFloat();

    if ( 0 <= value && value <= 0.6)
    {
        cmd.changeSpeed(value);
        if (!iobClient->publish(topicCreate("/iob/error").c_str(), "set speed success")) return false;
    } 
    else
    {
        if (!iobClient->publish(topicCreate("/iob/error").c_str(), "value for speed is not between 0 and 0.6")) return false;
    }
  }

  //if (!iobClient->publish(topicCreate("/iob/error").c_str(), "no error")) return false;
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