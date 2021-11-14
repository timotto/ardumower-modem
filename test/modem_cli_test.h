#include <AUnit.h>
using namespace aunit;

#include "../src/modem_cli.h"
#include "../src/settings.h"

class TestModemCli : public TestOnce
{
private:
  ArduMower::Modem::Cli *uut;
  ArduMower::Modem::Router *router;

protected:
  void setup() override;
  void teardown() override;

  void givenTheInputExpectTheResponse(const String &givenInput, const String &expectedResponse);
};

testF(TestModemCli, responds_ok_to_at)
{
  String givenInput = "AT\r\n";
  String expectedResponse = "OK\r\n";

  givenTheInputExpectTheResponse(givenInput, expectedResponse);
}

testF(TestModemCli, responds_to_version_request)
{
  String givenInput = "AT+VERSION\r\n";
  String expectedResponse = String("+VERSION=") + ArduMower::Modem::Settings::Properties.version() + "\r\n";

  givenTheInputExpectTheResponse(givenInput, expectedResponse);
}

testF(TestModemCli, responds_to_name_request)
{
  String givenInput = "AT+NAME=some-name\r\n";
  String expectedResponse = "+NAME=some-name\r\n";

  givenTheInputExpectTheResponse(givenInput, expectedResponse);
}

// testbed

void TestModemCli::givenTheInputExpectTheResponse(const String &givenInput, const String &expectedResponse)
{
  Serial1.print(givenInput);
  String actualResponse = readLineFromSerial(Serial1);
  assertEqual(actualResponse, expectedResponse);
}

void TestModemCli::setup()
{
  router = new ArduMower::Modem::Router(Serial2);
  uut = new ArduMower::Modem::Cli(*router);

  router->begin();
  uut->begin();
  AsyncLooper.start(std::bind(&ArduMower::Modem::Router::loop, router));
}

void TestModemCli::teardown()
{
  AsyncLooper.stop();
  delete router;
  delete uut;
}
