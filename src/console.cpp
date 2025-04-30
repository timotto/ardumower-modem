#include <WiFi.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>
#include "console.h"
#include "git_version.h"
#include "../test/helper/fake_ardumower_h.h"

using namespace ArduMower::Modem;

static const char *_prompt = "MODEM> ";
static String chipIdString();

void Console::begin()
{
  while (io.available())
    io.read();
}

void Console::loop()
{
  loopInput();
}

void Console::loopInput()
{
  if (prompt)
  {
    prompt = false;
    if (echo)
      io.printf("\r\n%s", _prompt);
  }

  String line = "";
  bool changes = false;

  while (io.available())
  {
    changes = true;
    char c = io.read();
    if (c == '\n')
      c = '\r';
    line = input.update(c);
    if (line != "")
      break;
  }

  if (line == "")
  {
    if (echo && changes)
      io.printf("\r%s%s", _prompt, input.peek().c_str());
    return;
  }

  io.println();
  prompt = true;
  if (expectSettings)
  {
    processSettings(line);
    return;
  }

  if (line == "?" || line == "help")
    printHelp();
  else if (line == "i" || line == "info")
    printInfo();
  else if (line == "!" || line == "status")
    printStatus();
  else if (line == "restart modem")
    restartModem();
  else if (line == "reset settings")
    resetSettings();
  else if (line == "dump settings")
    dumpSettings();
  else if (line == "load settings")
    loadSettings();
  else if (line == "json")
    setJsonOutput(true);
  else if (line == "text")
    setJsonOutput(false);
  else if (line == "echo on")
    setEcho(true);
  else if (line == "echo off")
    setEcho(false);
  else if (line == "fake on")
    setFakeEnabled(true);
  else if (line == "fake off")
    setFakeEnabled(false);
  else if (line == "fake timeout")
    setFakeTimeout();
  else
    printUnknown(line);
}

void Console::setJsonOutput(bool enabled)
{
  json = enabled;
  if (json)
    printJson(
        [&](const JsonObject &o)
        {
          o["result"] = "ok";
          o["kind"] = "output";
          o["output"] = "json";
        });
  else
    io.println("Switching to text output.");
}

void Console::setEcho(bool enabled)
{
  echo = enabled;
  if (json)
    printJson(
        [&](const JsonObject &o)
        {
          o["result"] = "ok";
          o["kind"] = "echo";
          o["echo"] = enabled;
        });
  else
    io.printf("Echo %s\r\n", enabled ? "ON" : "OFF");
}

void Console::setFakeEnabled(bool enabled) {
  FakeArduMower.active = enabled;
  if (json)
    printJson(
        [&](const JsonObject &o)
        {
          o["result"] = "ok";
          o["kind"] = "fake";
          o["fake"] = enabled;
        });
  else
    io.printf("Fake %s\r\n", enabled ? "ON" : "OFF");
}

void Console::setFakeTimeout()
{
  FakeArduMower.fakeTimeoutNext = true;
  if (json)
    printJson(
        [&](const JsonObject &o)
        {
          o["result"] = "ok";
          o["kind"] = "fake_timeout";
          o["fake_timeout"] = true;
        });
  else
    io.printf("Fake timeout ON\r\n");
}

void Console::printInfo()
{
  String id = chipIdString();
  if (json)
  {
    printJson(
        [&](const JsonObject &o)
        {
          o["result"] = "ok";
          o["kind"] = "info";
          auto info = o.createNestedObject("info");
          info["firmware"] = "ArduMower Modem";
          info["name"] = settings.general.name.c_str();
          auto esp = info.createNestedObject("esp32");
          esp["chip_id"] = id.c_str();
          auto git = info.createNestedObject("git");
          git["hash"] = git_hash;
          git["time"] = git_time;
          git["tag"] = git_tag;
        });
    return;
  }

  title("Modem Info");
  io.printf(
      "Firmware: %s\r\n"
      "Name: %s\r\n"
      "ESP32:\r\n"
      "  ChipId: %s\r\n"
      "Git:\r\n"
      "  Hash: %s\r\n"
      "  Time: %s\r\n"
      "  Tag : %s\r\n",
      "ArduMower Modem",
      id.c_str(),
      settings.general.name.c_str(),
      git_hash,
      git_time,
      git_tag);
}

void Console::printStatus()
{
  String wifiMode = "-";
  bool wifiSta = false;
  bool wifiAp = false;

  const char *modes[] = {"NULL", "STA", "AP", "STA+AP"};

  wifi_mode_t mode;
  esp_wifi_get_mode(&mode);
  wifiSta = mode == 1 || mode == 3;
  wifiAp = mode == 2 || mode == 3;
  if (mode >= 0 && mode <= 3)
    wifiMode = modes[mode];

  uint8_t wifiChannel;
  wifi_second_chan_t secondChan;
  esp_wifi_get_channel(&wifiChannel, &secondChan);

  wifi_config_t confSta, confAp;
  esp_wifi_get_config(WIFI_IF_STA, &confSta);
  esp_wifi_get_config(WIFI_IF_AP, &confAp);

  if (json)
  {
    printJson(
        [&](const JsonObject &o)
        {
          o["result"] = "ok";
          o["kind"] = "status";
          auto status = o.createNestedObject("status");
          status["uptime"] = millis();
          auto heap = status.createNestedObject("heap");
          heap["size"] = ESP.getHeapSize();
          heap["free"] = ESP.getFreeHeap();
          heap["min_free"] = ESP.getMinFreeHeap();
          heap["max_alloc"] = ESP.getMaxAllocHeap();

          auto w = status.createNestedObject("wifi");
          w["mode"] = wifiMode.c_str();
          w["channel"] = wifiChannel;
          auto ap = w.createNestedObject("ap");
          if (!wifiAp)
            ap["enabled"] = false;
          else
          {
            ap["enabled"] = true;
            ap["ssid"] = String(reinterpret_cast<const char *>(confAp.ap.ssid));
            ap["ip"] = String(WiFi.softAPIP().toString().c_str());
          }
          auto sta = w.createNestedObject("sta");
          if (!wifiSta)
            sta["enabled"] = false;
          else
          {
            sta["enabled"] = true;
            sta["ssid"] = String(reinterpret_cast<const char *>(confSta.sta.ssid));
            sta["ip"] = String(WiFi.localIP().toString().c_str());
          }
        });
    return;
  }

  title("Modem Status");
  io.printf(
      "Uptime: %u\r\n"
      "Heap:\r\n"
      " Size    : %u\r\n"
      " Free    : %u\r\n"
      " MinFree : %u\r\n"
      " MaxAlloc: %u\r\n",
      millis(),
      ESP.getHeapSize(),
      ESP.getFreeHeap(),
      ESP.getMinFreeHeap(),
      ESP.getMaxAllocHeap());

  io.printf(
      "WiFi:\r\n"
      " Mode   : %s\r\n"
      " Channel: %d\r\n",
      wifiMode.c_str(),
      (int)wifiChannel);

  if (wifiAp)
  {
    String wifiApSsid = String(reinterpret_cast<const char *>(confAp.ap.ssid));
    String wifiApIp = WiFi.softAPIP().toString();

    io.printf(
        " AP:\r\n"
        "  SSID: %s\r\n"
        "  IP  : %s\r\n",
        wifiApSsid.c_str(), wifiApIp.c_str());
  }

  if (wifiSta)
  {
    String wifiStaSsid = String(reinterpret_cast<const char *>(confSta.sta.ssid));
    String wifiStaIp = WiFi.localIP().toString();

    io.printf(
        " STA:\r\n"
        "  SSID: %s\r\n"
        "  IP  : %s\r\n",
        wifiStaSsid.c_str(), wifiStaIp.c_str());
  }

}

void Console::restartModem()
{
  if (json)
    printJson(
        [&](const JsonObject &o)
        {
          o["result"] = "ok";
          o["kind"] = "restart";
        });
  else
    io.println("Restarting");

  delay(10);
  ESP.restart();
}

void Console::resetSettings()
{
  if (!json)
    title("Reset settings");

  settings = Settings::Settings();
  if (!settings.save())
  {
    if (json)
      printJson(
          [&](const JsonObject &o)
          {
            o["result"] = "error";
            o["kind"] = "reset-settings";
            o["error"] = "save-defaults";
          });
    else
      io.println("Reset failed: unable to save default settings.");
    return;
  }

  if (json)
    printJson(
        [&](const JsonObject &o)
        {
          o["result"] = "ok";
          o["kind"] = "reset-settings";
        });
  else
    io.println("Reset successful.");

  restartModem();
}

void Console::dumpSettings()
{
  printJson(
      [&](const JsonObject &o)
      {
        o["result"] = "ok";
        o["kind"] = "dump-settings";
        auto s = o.createNestedObject("settings");
        settings.marshal(s);
      });
}

void Console::loadSettings()
{
  if (json)
    printJson(
        [&](const JsonObject &o)
        {
          o["result"] = "ok";
          o["kind"] = "load-settings";
          o["load"] = "ready";
        });
  else
  {
    title("Load settings");
    io.println("Enter JSON encoded settings as single line and press RETURN");
  }

  prompt = false;
  expectSettings = true;
}

void Console::processSettings(String line)
{
  expectSettings = false;
  io.println();

  DynamicJsonDocument doc(1024);
  auto err = deserializeJson(doc, line);
  if (err != DeserializationError::Ok)
  {
    if (json)
      printJson(
          [&](const JsonObject &o)
          {
            o["result"] = "error";
            o["kind"] = "load-settings";
            o["error"] = err.c_str();
          });
    else
      io.printf("Load failed: unable to parse settings JSON: %s\r\n", err.c_str());
    return;
  }

  Settings::Settings uploaded = settings;
  if (!uploaded.unmarshal(doc.as<JsonObject>()))
  {
    if (json)
      printJson(
          [&](const JsonObject &o)
          {
            o["result"] = "error";
            o["kind"] = "load-settings";
            o["error"] = "unmarshal";
          });
    else
      io.printf("Load failed: unable to unmarshal settings.\r\n");
    return;
  }

  if (!uploaded.save())
  {
    if (json)
      printJson(
          [&](const JsonObject &o)
          {
            o["result"] = "error";
            o["kind"] = "load-settings";
            o["error"] = "save";
          });
    else
      io.printf("Load failed: unable to save settings.\r\n");
    return;
  }

  if (json)
    printJson(
        [&](const JsonObject &o)
        {
          o["result"] = "ok";
          o["kind"] = "load-settings";
          o["load"] = "ready";
        });
  else
    io.println("Settings loaded from console and saved to flash.");

  restartModem();
}

void Console::printHelp()
{
  title("ArduMower Modem Console help");
  io.print(
      "help     list available commands\r\n"
      "info     show hardware info and software revision\r\n"
      "status   show runtime status\r\n"
      "json     output in json\r\n"
      "text     output in text\r\n"
      "fake on|off      enable/disable fake\r\n"
      "fake timeout     fake response timeout to next ArduMower request\r\n"
      "restart modem    restart the modem\r\n"
      "reset settings   reset modem settings to default values\r\n"
      "dump settings    print modem settings to console\r\n"
      "load settings    read modem settings from console\r\n");
}

void Console::printUnknown(String line)
{
  io.printf("Unknown command: %s\r\nEnter ? for a list of commands\r\n", line.c_str());
}

void Console::title(const char *title)
{
  io.print(title);
  io.print("\r\n");
  auto n = strlen(title);
  for (auto i = 0; i < n; i++)
    io.print("=");
  io.print("\r\n\r\n");
}

void Console::printJson(std::function<void(const JsonObject &o)> fn)
{
  DynamicJsonDocument doc(1024);
  const JsonObject &o = doc.to<JsonObject>();
  fn(o);
  serializeJson(doc, io);
  io.println();
}

static String chipIdString()
{
  uint32_t chipId = 0;
  for (auto i = 0; i < 17; i = i + 8)
  {
    chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }

  return String(chipId);
}
