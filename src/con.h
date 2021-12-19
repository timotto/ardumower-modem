#pragma once

#include <functional>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "reader.h"
#include "settings.h"

namespace ArduMower
{
  namespace Modem
  {
    class Console
    {
    private:
      HardwareSerial &io;
      Settings::Settings &settings;
      Reader input;
      bool echo;
      bool prompt;
      bool expectSettings;
      bool json;

      void loopInput();

      void setJsonOutput(bool enabled);
      void setEcho(bool enabled);
      void setFakeEnabled(bool enabled);
      void setFakeTimeout();

      void printHelp();
      void printInfo();
      void printStatus();
      void printUnknown(String line);

      void title(const char *title);

      void resetSettings();
      void dumpSettings();
      void loadSettings();
      void processSettings(String line);

      void restartModem();

      void printJson(std::function<void(const JsonObject & o)> fn);

    public:
      Console(HardwareSerial &_io, Settings::Settings &_settings)
          : io(_io), settings(_settings), input("\r"), echo(true), prompt(true), expectSettings(false), json(false) {}

      void begin();
      void loop();
    };
  }
}