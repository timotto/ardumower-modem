#pragma once

#include <WiFi.h>
#include <WiFiAP.h>
#include "helper/async_looper.h"
#include "helper/fake_ardumower.h"
#include "helper/fake_os.h"
#include "helper/fixtures.h"
#include "helper/mock_mower_adapter.h"

// a hardware loopback on the ESP32 between UART1 and UART2 allows testing without mocks

void setup_serial() {
  Serial.begin(115200);
  Serial1.begin(921600, SERIAL_8N1, 23, 22); // loop to Serial2
  Serial2.begin(921600, SERIAL_8N1, 16, 17); // connect Serial2 TXD to GPIO 23 and RXD to GPIO 22

  auto verify = [](Stream & in, Stream & out, String text, char term, String nameIn, String nameOut) {
    while(in.available()) in.read();
    out.print(text);
    out.print(term);
    delay(10);
    String buf = in.readStringUntil(term);
    if (buf == text) return;

    Serial.printf("check the serial loop: the ESP cannot receive on %s what is send from %s\n", nameIn.c_str(), nameOut.c_str());
    Serial.printf("received %d bytes: [%s]\n", buf.length(), buf.c_str());
  };

  verify(Serial1, Serial2, "check it here with a very long string since the tests might do the same", ':', "Serial1", "Serial2");
  verify(Serial2, Serial1, "CHECK2", ':', "Serial2", "Serial1");
}

#define drainSerial(s) while(s.available())s.read()

// WiFi in AP mode allows a local HTTP client to connect to the HTTP server

void setup_network() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("random", "should be random");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}


// allows time to connect serial port monitor before tests are finished

#ifndef START_WAIT_TIMEOUT
#define START_WAIT_TIMEOUT  3000
#endif

void start_wait() {
  Serial.println("press RETURN to start without delay");
  const uint32_t timeout = millis() + START_WAIT_TIMEOUT;

  while(true) {
    delay(1);
    if (millis() >= timeout) break;

    if (Serial.available()) {
      const int c = Serial.read();
      if (c == '\r' || c == '\n') break;
    }

    const uint32_t seconds_left = (timeout - millis()) / 1000;
    static uint32_t last_print = 0;
    if (seconds_left != last_print) {
      last_print = seconds_left;
      Serial.printf("\r%d... ", seconds_left);
    }
  }
  Serial.printf("\r");
}



String readLineFromSerial(Stream & s) {
  const uint32_t timeout = millis() + 100;
  String result;
  while(!result.endsWith("\r\n")) {
    if (millis() >= timeout) return result;
    else if (s.available()) while (s.available()) {
      char c = s.read();
      result += c;
    }
    else delay(1);
  }

  return result;
}

class RecordingPrinter : public Print {
public:
  String result;
  virtual size_t write(uint8_t c) {
    String s = String((char)c);

    result = result + s;
  }
};
