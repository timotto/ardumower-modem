#include <AUnit.h>
#include <HTTPClient.h>
#include "test_helpers.h"

#include "../src/ui_adapter.h"
#include "../src/git_version.h"
#include "../src/asset_bundle.h"

using namespace aunit;

const asset_t *lookupAsset(const char *path);

class TestUiAdapter : public TestOnce, public ArduMower::Modem::Api::Bluetooth
{
public:
  ArduMower::Modem::Http::UiAdapter *uut;

  FakeArduinoESP fakeOs;
  ArduMower::Modem::Api::Api *api;
  ArduMower::Modem::Settings::Settings settings;
  AsyncWebServer *server;
  MockMowerAdapter mockMower;
  ArduMower::Modem::BleAdapter *ble;

  const asset_t *indexHtml;
  WiFiClient net;
  HTTPClient http;

  int bluetoothResetCount;

  void setup() override;
  void teardown() override;

  virtual void clearPairings() override;
};

testF(TestUiAdapter, get_root_returns_index_html_gz)
{
  http.begin(net, "http://localhost:8080/");
  assertTrue(http.GET());
  String responseBody = http.getString();

  unsigned long actualLength = responseBody.length();
  unsigned long expectedLength = indexHtml->size;
  assertEqual(actualLength, expectedLength);

  assertEqual(memcmp(responseBody.c_str(), indexHtml->data, actualLength), (int)0);
}

testF(TestUiAdapter, get_api_robot_desired_state)
{
  mockMower._desiredState.speed = 17;

  http.begin(net, "http://localhost:8080/api/robot/desired_state");
  assertTrue(http.GET());
  String responseBody = http.getString();

  assertNotEqual(-1, responseBody.indexOf("speed"));
  assertNotEqual(-1, responseBody.indexOf("17"));
}

testF(TestUiAdapter, get_api_modem_info)
{
  http.begin(net, "http://localhost:8080/api/modem/info");
  assertTrue(http.GET());
  String responseBody = http.getString();

  assertNotEqual(-1, responseBody.indexOf("uptime"));
  assertNotEqual(-1, responseBody.indexOf("git_hash"));
  assertNotEqual(-1, responseBody.indexOf(git_hash));
}

testF(TestUiAdapter, get_api_modem_settings)
{
  settings.general.name = "a-specific-name";

  http.begin(net, "http://localhost:8080/api/modem/settings");
  assertTrue(http.GET());
  String responseBody = http.getString();

  assertNotEqual(-1, responseBody.indexOf("revision"));
  assertNotEqual(-1, responseBody.indexOf("name"));
  assertNotEqual(-1, responseBody.indexOf("a-specific-name"));
}

testF(TestUiAdapter, post_api_modem_settings_saves_and_restarts)
{
  String expectedName = "name-after-post";
  settings.general.name = "name-before-post";
  assertTrue(settings.save());

  auto different = settings;
  different.general.name = expectedName;
  
  String postRequestBody;
  DynamicJsonDocument doc(1024);
  different.marshal(doc.to<JsonObject>());
  serializeJson(doc, postRequestBody);
  http.begin(net, "http://localhost:8080/api/modem/settings");
  http.addHeader("Content-type", "application/json");

  assertEqual(http.POST(postRequestBody), 200);

  // settings are saved to flash by http post handler, so reload from flash
  settings.begin();
  assertEqual( settings.general.name, expectedName);

  // successful settings upload triggers restart
  // restart happens some time later
  auto timeout = millis()+100;
  while(fakeOs.restarts() < 1)
  {
    assertTrue( millis() < timeout);
    delay(10);
  }
}

testF(TestUiAdapter, post_api_modem_settings_rejects_invalid_settings_with_reason)
{
  String expectedName = "name-after-post";
  settings.general.name = expectedName;
  assertTrue(settings.save());

  auto different = settings;
  different.general.name = "";
  
  String postRequestBody;
  DynamicJsonDocument doc(1024);
  different.marshal(doc.to<JsonObject>());
  serializeJson(doc, postRequestBody);
  http.begin(net, "http://localhost:8080/api/modem/settings");
  http.addHeader("Content-type", "application/json");

  assertEqual(http.POST(postRequestBody), 400);
  assertNotEqual(http.getString().indexOf("general.name"), -1);

  // settings are saved to flash by http post handler, so reload from flash
  settings.begin();
  assertEqual( settings.general.name, expectedName);

  // successful settings upload triggers restart
  // restart happens some time later
  auto timeout = millis()+100;
  while(millis() < timeout)
  {
    assertEqual( fakeOs.restarts(), (int)0 );
    delay(1);
  }
}

testF(TestUiAdapter, post_api_modem_settings_reset_restores_defaults_and_restarts)
{
  ArduMower::Modem::Settings::Settings defaultSettings;
  settings.general.name = "not-default";
  settings.bluetooth.enabled = !defaultSettings.bluetooth.enabled;
  settings.mqtt.enabled = !defaultSettings.mqtt.enabled;
  settings.prometheus.enabled = !defaultSettings.prometheus.enabled;
  settings.wifi.ap_psk = "non-default-psk";
  assertTrue(settings.save());

  http.begin(net, "http://localhost:8080/api/modem/settings/reset");


  assertEqual(http.POST(""), 200);

  // settings are saved to flash by http post handler, so reload from flash
  settings.begin();
  assertEqual( settings.general.name, defaultSettings.general.name);
  assertEqual( settings.bluetooth.enabled, defaultSettings.bluetooth.enabled);
  assertEqual( settings.mqtt.enabled, defaultSettings.mqtt.enabled);
  assertEqual( settings.prometheus.enabled, defaultSettings.prometheus.enabled);
  assertEqual( settings.wifi.ap_psk, defaultSettings.wifi.ap_psk);

  // successful settings upload triggers restart
  // restart happens some time later
  auto timeout = millis()+100;
  while(fakeOs.restarts() < 1)
  {
    assertTrue( millis() < timeout);
    delay(10);
  }
}

testF(TestUiAdapter, post_api_modem_bluetooth_reset_clears_pairings_and_restarts)
{
  http.begin(net, "http://localhost:8080/api/modem/bluetooth/reset");


  assertEqual(http.POST(""), 200);

  assertEqual( bluetoothResetCount , (int)1);

  // clear pairings triggers restart
  // restart happens some time later
  auto timeout = millis()+100;
  while(fakeOs.restarts() < 1)
  {
    assertTrue( millis() < timeout);
    delay(10);
  }
}

// testbed

const asset_t *lookupAsset(const char *path)
{
  for (auto i = 0; i < asset_count; i++)
  {
    const asset_t *asset = &assets[i];
    if (strcmp(asset->path, path) == 0)
      return asset;
  }

  return nullptr;
}

void TestUiAdapter::setup()
{
  bluetoothResetCount = 0;

  indexHtml = lookupAsset("/index.html");
  assertTrue(indexHtml != nullptr);

  api = new ArduMower::Modem::Api::Api(fakeOs);
  api->begin(this);
  settings = testModemSettings();

  server = new AsyncWebServer(8080);
  server->begin();

  uut = new ArduMower::Modem::Http::UiAdapter(*api, settings, *server, mockMower);
  uut->begin();
}

void TestUiAdapter::teardown()
{
  http.end();
  // delay(1);
  server->end();
  // delay(1);
  // delete uut;
  delete server;
  delete api;
}

void TestUiAdapter::clearPairings()
{
  bluetoothResetCount++;
}
