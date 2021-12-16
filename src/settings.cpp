#include "settings.h"
#include "log.h"
#include "git_version.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <string.h>

using namespace ArduMower::Modem::Settings;

// #define SETTINGS_DUMP_CONTENT

const char * _t_revision = "revision";
const char * _t_general = "general";
const char * _t_web = "web";
const char * _t_wifi = "wifi";
const char * _t_bluetooth = "bluetooth";
const char * _t_relay = "relay";
const char * _t_mqtt = "mqtt";
const char * _t_prometheus = "prometheus";

const char * _t_sta_ssid = "sta_ssid";
const char * _t_sta_psk = "sta_psk";
const char * _t_ap_ssid = "ap_ssid";
const char * _t_ap_psk = "ap_psk";
const char * _t_name = "name";
const char * _t_username = "username";
const char * _t_password = "password";
const char * _t_enabled = "enabled";
const char * _t_mode = "mode";
const char * _t_off = "off";
const char * _t_ap = "ap";
const char * _t_sta = "sta";
const char * _t_encryption = "encryption";
const char * _t_protected = "protected";
const char * _t_pin_enabled = "pin_enabled";
const char * _t_pin = "pin";
const char * _t_prefix = "prefix";
const char * _t_server = "server";
const char * _t_publish_status = "publish_status";
const char * _t_publish_format = "publish_format";
const char * _t_publish_interval = "publish_interval";
const char * _t_ha = "ha";
const char * _t_json = "json";
const char * _t_text = "text";
const char * _t_both = "both";
const char * _t_git_hash = "git_hash";
const char * _t_git_time = "git_time";
const char * _t_git_tag = "git_tag";
const char * _t_uptime = "uptime";
const char * _t_has_sta_psk = "has_sta_psk";
const char * _t_has_ap_psk = "has_ap_psk";
const char * _t_has_password = "has_password";
const char * _t_has_pin = "has_pin";
const char * _t_url = "url";
const char * _t_ping_interval = "ping_interval";

General::General() : name("Ardumower"), encryption(true), password(123456) {}

Settings::Settings(String filename) : _filename(filename), revision(0) {}

#define containsAndHas(o, k, h) (o.containsKey(k) && (!o.containsKey(h) || o[h].as<bool>() == true))

void Settings::begin()
{
  if (!SPIFFS.begin(true))
  {
    Log(ERR, "Settings::begin::spiffs-begin-error");
    return;
  }

  File file = SPIFFS.open(_filename.c_str());
  if (!file || file.isDirectory())
  {
    Log(ERR, "Settings::begin::file-open-error");
    return;
  }

#ifdef SETTINGS_DUMP_CONTENT
  while (file.available())
    Serial.write(file.read());
  Serial.println();
  file.close();
  file = SPIFFS.open(filename.c_str());
#endif

  DynamicJsonDocument doc(1024);
  auto err = deserializeJson(doc, file);

  if (err != DeserializationError::Ok)
  {
    Log(ERR, "Settings::begin::deserializeJson-error(%s)", err.c_str());
  }
  else if (!unmarshal(doc.as<JsonObject>()))
  {
    Log(ERR, "Settings::begin::unmarshal-error");
  }
  else
  {
    Log(INFO, "Settings::begin::success");
  }

  file.close();
}

bool Settings::save()
{
  if (!SPIFFS.begin(true))
  {
    Log(ERR, "Settings::save::spiffs-begin-error");
    return false;
  }

  DynamicJsonDocument doc(1024);
  marshal(doc.to<JsonObject>());

  File file = SPIFFS.open(_filename.c_str(), FILE_WRITE);
  if (!file)
  {
    Log(ERR, "Settings::save::file-open-error");
    return false;
  }

  serializeJson(doc, file);

  file.close();

  Log(INFO, "Settings::save::success");

  return true;
}

bool Settings::valid(String &invalid) const
{
  return general.valid(invalid) && web.valid(invalid) && wifi.valid(invalid) && relay.valid(invalid) && mqtt.valid(invalid);
}

void Settings::marshal(const JsonObject &o) const
{
  o[_t_revision] = revision;
  general.marshal(o.createNestedObject(_t_general));
  web.marshal(o.createNestedObject(_t_web));
  wifi.marshal(o.createNestedObject(_t_wifi));
  bluetooth.marshal(o.createNestedObject(_t_bluetooth));
  relay.marshal(o.createNestedObject(_t_relay));
  mqtt.marshal(o.createNestedObject(_t_mqtt));
  prometheus.marshal(o.createNestedObject(_t_prometheus));
}

#define mustContain(component, o, prop)                      \
  if (!o.containsKey(prop))                                  \
  {                                                          \
    Log(ERR, "%s::unmarshal::missing(%s)", component, prop); \
    return false;                                            \
  }
#define mustContainAndSucceed(component, o, prop, fn)        \
  if (!o.containsKey(prop))                                  \
  {                                                          \
    Log(ERR, "%s::unmarshal::missing(%s)", component, prop); \
    return false;                                            \
  }                                                          \
  else if (!fn)                                              \
  {                                                          \
    Log(ERR, "%s::unmarshal::error(%s)", component, prop);   \
    return false;                                            \
  }

bool Settings::unmarshal(const JsonObject &o)
{
  mustContain("Settings", o, _t_revision);
  revision = o[_t_revision];

  mustContainAndSucceed("Settings", o, _t_general, general.unmarshal(o[_t_general]));
  mustContainAndSucceed("Settings", o, _t_web, web.unmarshal(o[_t_web]));
  mustContainAndSucceed("Settings", o, _t_wifi, wifi.unmarshal(o[_t_wifi]));
  mustContainAndSucceed("Settings", o, _t_bluetooth, bluetooth.unmarshal(o[_t_bluetooth]));
  mustContainAndSucceed("Settings", o, _t_relay, relay.unmarshal(o[_t_relay]));
  mustContainAndSucceed("Settings", o, _t_mqtt, mqtt.unmarshal(o[_t_mqtt]));
  mustContainAndSucceed("Settings", o, _t_prometheus, prometheus.unmarshal(o[_t_prometheus]));

  return true;
}

void Settings::stripSecrets(const JsonObject &o) const
{
  general.stripSecrets(o[_t_general]);
  web.stripSecrets(o[_t_web]);
  wifi.stripSecrets(o[_t_wifi]);
  bluetooth.stripSecrets(o[_t_bluetooth]);
  relay.stripSecrets(o[_t_relay]);
  mqtt.stripSecrets(o[_t_mqtt]);
  prometheus.stripSecrets(o[_t_prometheus]);
}

static bool validBluetoothAndDnsName(const String &name)
{
  if (name == "")
    return false;

  auto n = name.length();
  if (n > 24)
    return false;

  for (auto i = 0; i < n; i++)
  {
    auto c = name[i];
    // upper & lower case letters are always legal
    if (c >= 'a' && c <= 'z')
      continue;
    if (c >= 'A' && c <= 'Z')
      continue;

    bool first = i == 0;
    // must not start with number
    if (!first && c >= '0' && c <= '9')
      continue;
    bool last = i == n - 1;
    if (!(first || last) && (c == '-' || c == '_'))
      continue;

    return false;
  }

  return true;
}

bool General::valid(String &invalid) const
{
  if (!validBluetoothAndDnsName(name))
    invalid = "general.name";
  else
    return true;

  return false;
}

void General::marshal(const JsonObject &o) const
{
  o[_t_name] = name;
  o[_t_encryption] = encryption;
  o[_t_password] = password;
}

bool General::unmarshal(const JsonObject &o)
{
  mustContain("General", o, _t_name);
  mustContain("General", o, _t_encryption);

  name = o[_t_name].as<String>();
  encryption = o[_t_encryption];
  if (containsAndHas(o, _t_password, _t_has_password))
    password = o[_t_password];

  return true;
}

void General::stripSecrets(const JsonObject &o) const
{
  stripSecret(o, _t_password, _t_has_password);
}

bool Web::valid(String &invalid) const
{
  if (!use_password)
    return true;

  if (username == "")
    invalid = "web.username";
  else if (password == "")
    invalid = "web.password";
  else
    return true;

  return false;
}

void Web::marshal(const JsonObject &o) const
{
  o[_t_protected] = use_password;
  o[_t_username] = username;
  o[_t_password] = password;
}

bool Web::unmarshal(const JsonObject &o)
{
  use_password = o[_t_protected];
  username = o[_t_username].as<String>();
  if (containsAndHas(o, _t_password, _t_has_password))
    password = o[_t_password].as<String>();

  return true;
}

void Web::stripSecrets(const JsonObject &o) const
{
  stripSecret(o, _t_password, _t_has_password);
}

bool WiFi::valid(String &invalid) const
{
  switch (mode)
  {
  case 0:
    // explicit fallthrough
  case 2:
    if (ap_ssid == "")
      invalid = "wifi.ap_ssid";
    else if (ap_psk == "")
      invalid = "wifi.ap_psk";
    else
      return true;

    return false;

  case 1:
    if (sta_ssid == "")
      invalid = "wifi.sta_ssid";
    else if (sta_psk == "")
      invalid = "wifi.sta_psk";
    else
      return true;

    return false;

  default:
    invalid = "wifi.mode";
    return false;
  }
}

void WiFi::marshal(const JsonObject &o) const
{
  switch (mode)
  {
  case 0:
    o[_t_mode] = _t_off;
    break;
  case 1:
    o[_t_mode] = _t_sta;
    break;
  case 2:
    o[_t_mode] = _t_ap;
    break;
  }
  o[_t_sta_ssid] = sta_ssid;
  o[_t_sta_psk] = sta_psk;
  o[_t_ap_ssid] = ap_ssid;
  o[_t_ap_psk] = ap_psk;
}

bool WiFi::unmarshal(const JsonObject &o)
{
  mode = 0;
  if (o[_t_mode] == _t_sta)
    mode = 1;
  else if (o[_t_mode] == _t_ap)
    mode = 2;

  if (o.containsKey(_t_sta_ssid))
    sta_ssid = o[_t_sta_ssid].as<String>();
  if (o.containsKey(_t_ap_ssid))
    ap_ssid = o[_t_ap_ssid].as<String>();
  if (containsAndHas(o, _t_sta_psk, _t_has_sta_psk))
    sta_psk = o[_t_sta_psk].as<String>();
  if (containsAndHas(o, _t_ap_psk, _t_has_ap_psk))
    ap_psk = o[_t_ap_psk].as<String>();

  return true;
}

void WiFi::stripSecrets(const JsonObject &o) const
{
  stripSecret(o, _t_sta_psk, _t_has_sta_psk);
  stripSecret(o, _t_ap_psk, _t_has_ap_psk);
}

void Bluetooth::marshal(const JsonObject &o) const
{
  o[_t_enabled] = enabled;
  o[_t_pin_enabled] = pin_enabled;
  o[_t_pin] = pin;
}

bool Bluetooth::unmarshal(const JsonObject &o)
{
  enabled = o[_t_enabled];
  pin_enabled = o[_t_pin_enabled];
  if (containsAndHas(o, _t_pin, _t_has_pin))
    pin = o[_t_pin].as<uint32_t>();

  return true;
}

void Bluetooth::stripSecrets(const JsonObject &o) const
{
  if (o.containsKey(_t_pin))
  {
    o[_t_has_pin] = true;
    o.remove(_t_pin);
  }
}

static bool validDnsName(const String &name)
{
  if (name == "")
    return false;

  auto n = name.length();
  for (auto i = 0; i < n; i++)
  {
    auto c = name[i];
    // upper & lower case letters are always legal
    if (c >= 'a' && c <= 'z')
      continue;
    if (c >= 'A' && c <= 'Z')
      continue;

    auto first = i == 0;
    // must not start with number or special characters
    if (!first && c >= '0' && c <= '9')
      continue;

    auto last = i == n - 1;
    // must not start or end with number or special characters
    if (!first && !last && (c == '-' || c == '_' || c == '.'))
      continue;

    return false;
  }

  return true;
}

bool Relay::valid(String &invalid) const
{
  // if (!enabled)
    return true;

  // if (!validUrl(url))
  //   invalid = "relay.url";
  // else
  //   return true;

  // return false;
}

void Relay::marshal(const JsonObject &o) const
{
  o[_t_enabled] = enabled;
  o[_t_url] = url;
  o[_t_username] = username;
  o[_t_password] = password;
  o[_t_ping_interval] = pingInterval;
}

bool Relay::unmarshal(const JsonObject &o)
{
  enabled = o[_t_enabled];
  url = o[_t_url].as<String>();
  username = o[_t_username].as<String>();
  if (containsAndHas(o, _t_password, _t_has_password))
    password = o[_t_password].as<String>();
  pingInterval = o[_t_ping_interval];

  return true;
}

void Relay::stripSecrets(const JsonObject &o) const
{
  stripSecret(o, _t_password, _t_has_password);
}

bool MQTT::valid(String &invalid) const
{
  if (!enabled)
    return true;

  if (!validDnsName(server))
    invalid = "mqtt.server";
  else
    return true;

  return false;
}

void MQTT::marshal(const JsonObject &o) const
{
  o[_t_enabled] = enabled;

  o[_t_prefix] = prefix;
  o[_t_server] = server;
  o[_t_username] = username;
  o[_t_password] = password;

  o[_t_publish_status] = publishStatus;
  o[_t_publish_format] = publishFormat;
  o[_t_publish_interval] = publishInterval;

  o[_t_ha] = ha;
}

bool MQTT::unmarshal(const JsonObject &o)
{
  enabled = o[_t_enabled];
  prefix = o[_t_prefix].as<String>();
  server = o[_t_server].as<String>();
  username = o[_t_username].as<String>();
  if (containsAndHas(o, _t_password, _t_has_password))
    password = o[_t_password].as<String>();

  publishStatus = o[_t_publish_status];
  if (o[_t_publish_format] == _t_json)
    publishFormat = 1;
  else if (o[_t_publish_format] == _t_text)
    publishFormat = 2;
  else if (o[_t_publish_format] == _t_both)
    publishFormat = 3;
  else
    publishFormat = 3;
  publishInterval = o[_t_publish_interval];

  ha = o[_t_ha];

  return true;
}

void MQTT::stripSecrets(const JsonObject &o) const
{
  stripSecret(o, _t_password, _t_has_password);
}

void Prometheus::marshal(const JsonObject &o) const
{
  o[_t_enabled] = enabled;
}

bool Prometheus::unmarshal(const JsonObject &o)
{
  enabled = o[_t_enabled];

  return true;
}

void Prometheus::stripSecrets(const JsonObject &o) const {}

void Group::stripSecret(const JsonObject &o, const char *key, const char *hasKey) const
{
  if (!o.containsKey(key))
    return;

  const String &password = o[key].as<String>();
  const bool present = password != "";

  o[hasKey] = present;
  o.remove(key);
}

PropertiesClass::PropertiesClass() {}

const char *PropertiesClass::version() const
{
  if (strlen(git_tag) > 0)
    return git_tag;

  return git_hash;
}

void PropertiesClass::marshal(const JsonObject &o) const
{
  o[_t_git_hash] = git_hash;
  o[_t_git_time] = git_time;
  o[_t_git_tag] = git_tag;
  o[_t_uptime] = millis();
}

PropertiesClass ArduMower::Modem::Settings::Properties;
