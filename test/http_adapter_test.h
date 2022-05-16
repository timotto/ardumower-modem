#include <AUnit.h>
#include <HTTPClient.h>
#include "test_helpers.h"

#include "../src/http_adapter.h"

using namespace aunit;

static const char *corsHeader = "Access-Control-Allow-Origin";
static const char *corsHeader2 = "Access-Control-Allow-Headers";
static const char *interestingHeaders[] = {corsHeader, corsHeader2};

class TestHttpAdapter : public TestOnce
{
private:
protected:
  ArduMower::Modem::HttpAdapter *uut;
  AsyncWebServer *server;
  ArduMower::Modem::Router *router;
  WiFiClient net;
  HTTPClient http;

  void setup() override;
  void teardown() override;
};

testF(TestHttpAdapter, post_body_encoding)
{
  http.POST("AT+@7=9,0xad\n");
  assertEqual(FakeArduMower.testLine, String("AT+@7=9,0xad"));
}

testF(TestHttpAdapter, post_to_root_sends_line_from_body_to_router)
{
  int responseCode = http.POST("AT+V,0x16\n");
  String responseBody = http.getString();
  String responseHeaderACAO = http.header(corsHeader);
  String responseHeaderACAH = http.header(corsHeader2);

  assertEqual(responseCode, 200);
  assertEqual(responseBody, FakeArduMower.atvResponse + "\r\n");
  assertEqual(responseHeaderACAO, String("*"));
  assertEqual(responseHeaderACAH, String("authorization"));
}

testF(TestHttpAdapter, post_to_root_error_has_cors_headers)
{
  int responseCode = http.POST("");
  String responseBody = http.getString();
  String responseHeaderACAO = http.header(corsHeader);
  String responseHeaderACAH = http.header(corsHeader2);

  assertEqual(responseCode, 400);
  assertEqual(responseBody, "empty body");
  assertEqual(responseHeaderACAO, String("*"));
  assertEqual(responseHeaderACAH, String("authorization"));
}

testF(TestHttpAdapter, option_to_root_has_cors_headers)
{
  int responseCode = http.sendRequest("OPTIONS");
  String responseBody = http.getString();
  String responseHeaderACAO = http.header(corsHeader);
  String responseHeaderACAH = http.header(corsHeader2);

  assertEqual(responseCode, 204);
  assertEqual(responseHeaderACAO, String("*"));
  assertEqual(responseHeaderACAH, String("authorization"));
}

// testbed


void TestHttpAdapter::setup()
{
  server = new AsyncWebServer(8080);
  server->begin();

  router = new ArduMower::Modem::Router(Serial2);
  uut = new ArduMower::Modem::HttpAdapter(*router, *server);
  router->begin();
  uut->begin();

  auto loop = [&]()
  {
    router->loop();
    uut->loop();
  };

  AsyncLooper.start(loop);
  FakeArduMower.active = true;

  http.begin(net, "http://localhost:8080/");
  http.collectHeaders(interestingHeaders, 2);
  http.addHeader("Content-type", "text/plain");
}

void TestHttpAdapter::teardown()
{
  http.end();
  FakeArduMower.active = false;
  AsyncLooper.stop();
  delete uut;
  delete router;
  server->end();
  delete server;
}
