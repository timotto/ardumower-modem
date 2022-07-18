#include <Arduino.h>

// used by the CMakeLists.txt file to create a different compile target for test execution
#ifdef ESP_MODEM_TEST
#define DEBUG_LEVEL DBG
#include "../test/test_main.h"
#else

#include "git_version.h"

#ifdef ESP_MODEM_SIM
#define DEBUG_LEVEL DBG
#include "log.h"
#include "../test/helper/fake_ardumower.h"
#endif

#include "api.h"
#include "ble_adapter.h"
#include "conn.h"
#include "esp_os.h"
#include "http_adapter.h"
#include "looptime_monitor.h"
#include "modem_cli.h"
#include "mower_adapter.h"
#include "mqtt_adapter.h"
#include "ota.h"
#include "ota_arduinoota.h"
#include "ota_http_server.h"
#include "prometheus_adapter.h"
#include "relay_adapter.h"
#include "router.h"
#include "settings.h"
#include "ui_adapter.h"
#include "web_server.h"
#include "wifi_adapter.h"
#include "ps4_controller.h"

using namespace ArduMower::Modem;

// encapsulate ESP32 APIs like "reboot" to allow testing
auto espOs = ArduinoESP();
Api::Api api(espOs);

// user settings read/update with JSON file on SPIFFS as backend
Settings::Settings settings;
// client or access point based on user settings & connection errors
Wifi::Adapter wifiAdapter(settings);

#ifdef ESP_MODEM_SIM
// console is used during validation tests and enabled on simulator target only
Console con(Serial, api, settings);
#endif

WebServer webServer;

// firmware update via Arduino IDE with buggy WiFiUDP
// Ota::ArduinoOta ota;
// upload firmware as POST multipart body aka. form file upload
Ota::HttpServer otaHttpServer(settings, webServer.server());

// provides request/response communication with the robot
Router router(Serial2);
// Bluetooth LE endpoint for app
BleAdapter bleAdapter(settings, router);
// HTTP endpoint for app
HttpAdapter httpAdapter(router, webServer.server());
// Relay endpoint for app
RelayAdapter relayAdapter(settings, router);
// emulates the behavior of the previous generation Bluetooth UART modules
Cli modemCli(router);
// ArduMower protocol interpreter, keeps state
MowerAdapter mowerAdapter(settings, router);
// serves the web frontend
Http::UiAdapter ui(api, settings, webServer.server(), mowerAdapter);
// publish state on MQTT, receive commands on MQTT
MqttAdapter mqttAdapter(settings, router, mowerAdapter, mowerAdapter);
// metrics available for use with Prometheus
Prometheus::Adapter prometheusAdapter(settings, webServer.server(), mowerAdapter, mowerAdapter);
Prometheus::LooptimeMonitor looptime;

#ifdef ENABLE_PS4_CONTROLLER
PS4controller::Adapter ps4ControllerAdapter(settings, mowerAdapter, mowerAdapter); 
#endif

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

  api.begin(&bleAdapter, &relayAdapter);
  settings.begin();
#ifdef ESP_MODEM_SIM
  con.begin();
#endif
  wifiAdapter.begin();
  router.begin();
  modemCli.begin();
  webServer.begin();
  // ota.begin();
  otaHttpServer.begin();
  bleAdapter.begin();
  httpAdapter.begin();
  relayAdapter.begin();
  mowerAdapter.begin();
  mqttAdapter.begin();
  prometheusAdapter.begin();
  ui.begin();
#ifdef ENABLE_PS4_CONTROLLER
  ps4ControllerAdapter.begin();
#endif

#ifdef ESP_MODEM_SIM
  looptime.add("con", std::bind(&Console::loop, &con));
#endif
  looptime.add("wifi", [&](){wifiAdapter.loop();});
  looptime.add("http", std::bind(&HttpAdapter::loop, &httpAdapter));
  // looptime.add("ota_arduino", std::bind(&Ota::ArduinoOta::loop, &ota));
  looptime.add("ota_http", std::bind(&Ota::HttpServer::loop, &otaHttpServer));
  looptime.add("router", std::bind(&Router::loop, &router));
  looptime.add("ble", std::bind(&BleAdapter::loop, &bleAdapter));
  looptime.add("relay", std::bind(&RelayAdapter::loop, &relayAdapter));
  looptime.add("mqtt", [&](){mqttAdapter.loop(millis());});
#ifdef ENABLE_PS4_CONTROLLER
  looptime.add("ps4_controller", std::bind(&PS4controller::Adapter::loop, &ps4ControllerAdapter));
#endif
  
#ifdef ESP_MODEM_SIM
  Serial1.begin(115200, SERIAL_8N1, 23, 22); // loop to Serial2
  setup_fake_ardumower();
  FakeArduMower.active = true;
#endif
}

void loop() {
  looptime.loop();
  vPortYield();
}

#endif
