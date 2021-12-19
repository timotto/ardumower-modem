#pragma once

#include "../../src/reader.h"
#include "../../src/encrypt.h"
#include "../../src/log.h"

class FakeArduMowerClass
{
public:
  Reader rx;
  ArduMower::Encrypt enc;
  bool active;
  bool fakeTimeoutNext;
  String atvResponse;
  String testLine;
  FakeArduMowerClass()
      : rx("\n"), active(false),
        atvResponse("V,Ardumower Sunray,1.0.219,1,52,0x4f")
  {
    enc.setOn(true);
    enc.setPassword(123456);
    enc.setChallenge(52);
  }

  void respond(String res)
  {
    Log(DBG, "FakeArduMowerClass::respond(%s)", res.c_str());
    Serial1.print(res);
    Serial1.println();
  }

  void loop()
  {
    static bool wasActive = false;
    if (wasActive != active)
    {
      wasActive = active;
      rx.reset();
    }
    if (!active)
      return;

    String line;
    while (Serial1.available())
    {
      char c = Serial1.read();
      line = rx.update(c);
      line.trim();
      if (line == "")
        continue;
      break;
    }
    if (line == "")
      return;

    String decrypted = line;
    if (!line.startsWith("AT+"))
    {
      char *buf = strdup(line.c_str());
      enc.decrypt(buf, strlen(buf));
      decrypted = buf;
      free(buf);
      Log(DBG, "FakeArduMowerClass::loop::receive(line=%s,decrypted=%s)", line.c_str(), decrypted.c_str());
    }
    else
    {
      Log(DBG, "FakeArduMowerClass::loop::receive(line=%s)", line.c_str());
    }

    if (fakeTimeoutNext)
    {
      fakeTimeoutNext = false;
      return;
    }

    /*
      AT+V
      V,Ardumower Sunray,1.0.219,1,63,0x51
    ? AT+P,0,0,0,0x..
      I\3X48484848!:<
      P,0x50
      AT+S
      S,28.12,-10.77,-4.55,2.62,1,2,11,0.05,0,-7.47,-6.46,0.02,36,-0.00,29,-26017,0x55
      AT+T
      T,43,2895,242,34,208,1,28.45,0.28,0,998.00,998.00,55,0,0.02,1024,0,1,4,81751,0,5,0,0,1,0x98
      AT+C
      C,0x43

      // manual drive
      I\3U4869;45868>48!9@
      AT+.,0
      */

    if (decrypted.startsWith("AT+V"))
    {
      respond(atvResponse);
    }
    else if (decrypted.startsWith("AT+P"))
    {
      respond("P,0x50");
    }
    else if (decrypted.startsWith("AT+S"))
    {
      respond("S,28.12,-10.77,-4.55,2.62,1,2,11,0.05,0,-7.47,-6.46,0.02,36,-0.00,29,-26017,0x55");
    }
    else if (decrypted.startsWith("AT+C,"))
    {
      respond("C,0x43");
    }
    else if (decrypted.startsWith("AT+T"))
    {
      respond("T,43,2895,242,34,208,1,28.45,0.28,0,998.00,998.00,55,0,0.02,1024,0,1,4,81751,0,5,0,0,1,0x98");
    }
    else if (decrypted.startsWith("AT+M"))
    {
      // AT+M,0.23,-0.07,0x1a
      respond("M,0x3D");
    }
    else if (decrypted.startsWith("AT+W"))
    {
      // AT+W,0,-7.47,-6.46,-10.77,-4.55,0x3e
      respond("W,0x57");
    }
    else if (decrypted.startsWith("AT+N"))
    {
      // AT+N,0,0,2,40,0,0x10
      respond("N,0x4E");
    }
    else if (decrypted.startsWith("AT+X"))
    {
      // AT+X,0,0x74
      respond("X,0x58");
    }
    else if (decrypted.startsWith("AT+@"))
    {
      testLine = decrypted;
      respond("@,0x40");
    }
    else
    {
      respond("CRC ERROR ;-)");
    }
  }
};

extern FakeArduMowerClass FakeArduMower;
