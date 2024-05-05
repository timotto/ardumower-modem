#include <AUnit.h>
#include <inttypes.h>
#include "../src/prometheus_adapter.h"
#include "../src/prometheus_util.h"
#include "test_helpers.h"
#include <HTTPClient.h>
using namespace aunit;

class TestPrometheusAdapter : public TestOnce
{
public:
  ArduMower::Modem::Prometheus::Adapter *uut;
  AsyncWebServer *server;
  ArduMower::Modem::Settings::Settings *_settings;
  MockMowerAdapter mockMower;

  void setup() override;
  void teardown() override;
  
  String getActualMetrics(int &responseCode);
};

#define assertStringContains(hay, needle) assertNotEqual(hay.indexOf(needle), -1)

testF(TestPrometheusAdapter, metrics_endpoint)
{
  mockMower._state.batteryVoltage = 29.4;
  mockMower._state.amps = 8.3;
  mockMower._state.job = 4;
  mockMower._state.sensor = 0;
  mockMower._state.position.accuracy = 0.02;
  mockMower._state.chargingMah = 12043.13;
  mockMower._state.motorMowMah = 8911.13;
  mockMower._state.motorLeftMah = 908.13;
  mockMower._state.motorRightMah = 845.32;
  mockMower._state.temperature = 28.72;
  mockMower._stats.freeMemory = 123458;
  mockMower._stats.mowDistanceTraveled = 12345.6;
  mockMower._stats.gpsJumps = 11;
  mockMower._stats.gpsChecksumErrors = 2;
  mockMower._stats.durations.mow = 1234;
  mockMower._stats.durations.mowFix = 1230;
  mockMower._stats.durations.mowFloat = 4;
  mockMower._stats.obstacles.gpsMotionLow = 3;

  int responseCode;
  String actualMetrics = getActualMetrics(responseCode);

  if (responseCode != 200)
    Serial.println(actualMetrics);
  assertEqual(responseCode, 200);
  assertStringContains(actualMetrics, "ardumower_modem_total_runtime ");
  assertStringContains(actualMetrics, "ardumower_modem_heap{type=\"total\"} ");
  assertStringContains(actualMetrics, "ardumower_modem_heap{type=\"free\"} ");
  assertStringContains(actualMetrics, "ardumower_modem_heap{type=\"min_free\"} ");
  assertStringContains(actualMetrics, "ardumower_modem_heap{type=\"max_alloc\"} ");

  assertStringContains(actualMetrics, "ardumower_robot_battery_voltage 29.4");
  assertStringContains(actualMetrics, "ardumower_robot_amps 8.3");
  assertStringContains(actualMetrics, "ardumower_robot_job 4");
  assertStringContains(actualMetrics, "ardumower_robot_sensor 0");

  assertStringContains(actualMetrics, "ardumower_robot_mow_distance 12345");
  assertStringContains(actualMetrics, "ardumower_robot_free_memory 123458\n");
  assertStringContains(actualMetrics, "ardumower_robot_gps_jumps 11\n");
  assertStringContains(actualMetrics, "ardumower_robot_gps_checksum_errors 2\n");

  assertStringContains(actualMetrics, "ardumower_robot_gps_accuracy 0.02");
  assertStringContains(actualMetrics, "ardumower_robot_runtime{activity=\"mow\"} 1234\n");
  assertStringContains(actualMetrics, "ardumower_robot_mowtime{position=\"fix\"} 1230\n");
  assertStringContains(actualMetrics, "ardumower_robot_mowtime{position=\"float\"} 4\n");
  assertStringContains(actualMetrics, "ardumower_robot_obstacles{type=\"gps_motion_low\"} 3\n");

  assertStringContains(actualMetrics, "ardumower_robot_temperature 28.72\n");
  assertStringContains(actualMetrics, "ardumower_robot_mah{type=\"charge\"} 12043.13\n");
  assertStringContains(actualMetrics, "ardumower_robot_mah{type=\"mow\"} 8911.13\n");
  assertStringContains(actualMetrics, "ardumower_robot_mah{type=\"left\"} 908.13\n");
  assertStringContains(actualMetrics, "ardumower_robot_mah{type=\"right\"} 845.32\n");
}

// testbed

void TestPrometheusAdapter::setup()
{
  server = new AsyncWebServer(8082);
  _settings = new ArduMower::Modem::Settings::Settings("/test/settings");
  uut = new ArduMower::Modem::Prometheus::Adapter(*_settings, *server, mockMower, mockMower);

  _settings->prometheus.enabled = true;

  server->begin();
  uut->begin();
}

void TestPrometheusAdapter::teardown()
{
  server->end();

  delete uut;
  delete _settings;
  delete server;
}

String TestPrometheusAdapter::getActualMetrics(int &responseCode)
{
  WiFiClient net;
  HTTPClient http;
  http.begin(net, "http://localhost:8082/metrics");
  responseCode = http.GET();
  String responseBody;
  responseBody = http.getString();
  http.end();

  return responseBody;
}
