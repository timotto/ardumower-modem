// the router tests require Serial2 and Serial1 to be bridged

#include <AUnit.h>
using namespace aunit;

#include "test_helpers.h"

#include "../src/router.h"

class RouterResponseRecorder
{
public:
  bool responded;
  String response;
  ArduMower::Modem::XferError error;

  RouterResponseRecorder();
  void handler(String response, ArduMower::Modem::XferError err);
};

class TestRouter : public TestOnce
{
private:
  uint32_t fakeMillis();

protected:
  ArduMower::Modem::Router *uut;
  RouterResponseRecorder rec;
  bool fakeTimeIsReal;
  uint32_t fakeTime;

  TestRouter() : uut(nullptr), fakeTimeIsReal(true), fakeTime(0) {}

  void setup() override;
  void teardown() override;

  String actualOutput();
  void fakeInput(String input);
};

class TestDrain : public ArduMower::Modem::RxDrain, public ArduMower::Modem::TxDrain
{
public:
  TestDrain();
  int calls;
  bool stop;
  String line;
  void drainRx(String _line, bool &_stop);
  void drainTx(String _line, bool &_stop);
};

testF(TestRouter, sendWithoutResponse_appends_crlf)
{
  uut->sendWithoutResponse("something");
  assertEqual(actualOutput(), "something\r\n");
}

testF(TestRouter, sendWithoutResponse_returns_false_when_busy)
{
  auto first = uut->sendWithoutResponse("first");
  assertTrue( first );

  // no response from robot yet

  auto second = uut->sendWithoutResponse("second");
  assertFalse( second );

  fakeInput("a response\r\n");

  // response from robot causes first to "unblock"

  auto third = uut->sendWithoutResponse("third");
  assertTrue( third );

  assertEqual(actualOutput(), "first\r\nthird\r\n");
}

testF(TestRouter, send_appends_CRLF)
{
  uut->send("something else", [](String res, int err) {});
  assertEqual(actualOutput(), "something else\r\n");
}

testF(TestRouter, send_returns_false_when_busy)
{
  auto first = uut->send("first", [](String res, int err) {});
  assertTrue( first );

  // no response from robot yet

  auto second = uut->send("second", [](String res, int err) {});
  assertFalse( second );

  fakeInput("a response\r\n");

  // response from robot causes first to "unblock"

  auto third = uut->send("third", [](String res, int err) {});
  assertTrue( third );

  assertEqual(actualOutput(), "first\r\nthird\r\n");
}

testF(TestRouter, send_callback_receives_response)
{
  uut->send("something else", std::bind(&RouterResponseRecorder::handler, &rec, std::placeholders::_1, std::placeholders::_2));
  assertEqual(actualOutput(), "something else\r\n");
  assertFalse(rec.responded);

  uut->loop();
  assertFalse(rec.responded);

  fakeInput("a response\r\n");

  assertTrue(rec.responded);
  assertEqual(rec.response, "a response");
  assertEqual(rec.error, ArduMower::Modem::XferError::SUCCESS);
}

testF(TestRouter, send_callback_receives_timeout)
{
  fakeTimeIsReal = false;

  uut->send("something else", std::bind(&RouterResponseRecorder::handler, &rec, std::placeholders::_1, std::placeholders::_2));

  assertEqual(actualOutput(), "something else\r\n");
  assertFalse(rec.responded);

  fakeTime = 1;
  uut->loop();
  assertFalse(rec.responded);

  fakeTime = 500;
  uut->loop();
  assertFalse(rec.responded);

  fakeTime = 999;
  uut->loop();
  assertFalse(rec.responded);

  fakeTime = 1001;
  uut->loop();

  assertTrue(rec.responded);
  assertEqual(rec.response, "");
  assertEqual(rec.error, ArduMower::Modem::XferError::TIMEOUT);
}

testF(TestRouter, sniffRx_receives_all_responses)
{
  TestDrain drain1;
  TestDrain drain2;
  uut->sniffRx(&drain1);
  uut->sniffRx(&drain2);

  fakeInput("a response without request\r\n");

  assertEqual(drain1.line, String("a response without request"));
  assertEqual(drain2.line, String("a response without request"));
}

testF(TestRouter, sniffRx_can_stop_the_chain)
{
  TestDrain drain1;
  TestDrain drainStopTrue;
  TestDrain drain2;
  drainStopTrue.stop = true;

  uut->sniffRx(&drain1);
  uut->sniffRx(&drainStopTrue);
  uut->sniffRx(&drain2);

  fakeInput("a response without request\r\n");

  assertEqual(drain1.line, String("a response without request"));
  assertEqual(drain2.line, String(""));
}

testF(TestRouter, sinffTx_receives_all_commandsWithoutResponse)
{
  TestDrain drain1;
  TestDrain drain2;
  
  // given
  uut->sniffTx(&drain1);
  uut->sniffTx(&drain2);
  
  // when
  uut->sendWithoutResponse("a command without response");
  uut->loop();
  
  // then
  assertEqual(drain1.line, String("a command without response"));
  assertEqual(drain2.line, String("a command without response"));
}

testF(TestRouter, sinffTx_can_stop_chain)
{
  TestDrain drain1;
  TestDrain drainStopTrue;
  TestDrain drain2;
  drainStopTrue.stop = true;
  
  // given
  uut->sniffTx(&drain1);
  uut->sniffTx(&drainStopTrue);
  uut->sniffTx(&drain2);
  
  // when
  uut->sendWithoutResponse("a command without response");
  uut->loop();
  
  // then
  assertEqual(drain1.line, String("a command without response"));
  assertEqual(drain2.line, String(""));
}

testF(TestRouter, sinffTx_receives_all_commands)
{
  TestDrain drain1;
  TestDrain drain2;
  
  // given
  uut->sniffTx(&drain1);
  uut->sniffTx(&drain2);
  
  // when
  uut->send("a command with response", [](String r, int err) {});
  uut->loop();
  
  // then
  assertEqual(drain1.line, String("a command with response"));
  assertEqual(drain2.line, String("a command with response"));
}

// testbed

void TestRouter::setup()
{
  uut = new ArduMower::Modem::Router(
      Serial2,
      [&]()
      { return fakeTimeIsReal ? millis() : fakeTime; });
  uut->begin();
}

void TestRouter::teardown()
{
  while(Serial1.available()) Serial1.read();
  delete uut;
}

String TestRouter::actualOutput()
{
  uut->loop();
  return readLineFromSerial(Serial1);
}

void TestRouter::fakeInput(String input)
{
  Serial1.print(input.c_str());
  Serial1.flush();
  delay(10);
  uut->loop();
}

RouterResponseRecorder::RouterResponseRecorder() : responded(false), response(""), error(ArduMower::Modem::XferError::UNKNOWN) {}

void RouterResponseRecorder::handler(String r, ArduMower::Modem::XferError err)
{
  responded = true;
  response = r;
  error = err;
}

TestDrain::TestDrain() : calls(0), stop(false), line("") {}

void TestDrain::drainRx(String _line, bool &_stop)
{
  calls++;
  line = _line;
  _stop = stop;
};

void TestDrain::drainTx(String _line, bool &_stop)
{
  calls++;
  line = _line;
  _stop = stop;
};
