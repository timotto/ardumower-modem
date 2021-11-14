#include "ui_adapter.h"
#include "asset_bundle.h"
#include "log.h"
#include "settings.h"
#include "json.h"
#include <ArduinoJson.h>

using namespace ArduMower::Modem::Http;

UiAdapter::UiAdapter(Api::Api &api,
                     Settings::Settings &settings,
                     AsyncWebServer &server,
                     ArduMower::Domain::Robot::StateSource &source)
    : Common(settings), _api(api), _settings(settings), _server(server), _source(source)
{
}

void UiAdapter::begin()
{
  _server.on("/api/modem/info", HTTP_GET, std::bind(&UiAdapter::handleApiGetModemInfo, this, std::placeholders::_1));

  _server.on("/api/modem/settings", HTTP_GET, std::bind(&UiAdapter::handleApiGetModemSettings, this, std::placeholders::_1));
  _server.on("/api/modem/settings/reset", HTTP_POST, std::bind(&UiAdapter::handleApiResetModemSettings, this, std::placeholders::_1));
  // free happens in ~AsyncWebServer
  auto settingsHandler = new AsyncCallbackJsonWebHandler("/api/modem/settings", std::bind(&UiAdapter::handleApiPostModemSettings, this, std::placeholders::_1, std::placeholders::_2));
  settingsHandler->setMethod(HTTP_POST);
  _server.addHandler(settingsHandler);

  _server.on("/api/modem/bluetooth/reset", HTTP_POST, std::bind(&UiAdapter::handleApiResetModemBluetoothPairings, this, std::placeholders::_1));

  _server.on("/api/robot/desired_state", HTTP_GET, std::bind(&UiAdapter::handleApiGetRobotDesiredState, this, std::placeholders::_1));

  _server.onNotFound(std::bind(&UiAdapter::handleRequest, this, std::placeholders::_1));
}

bool UiAdapter::servePath(AsyncWebServerRequest *request, const String &path)
{
  for (auto i = 0; i < asset_count; i++)
  {
    const asset_t *asset = &assets[i];
    if (path != asset->path)
      continue;

    if (request->hasHeader("If-None-Match"))
    {
      auto h = request->getHeader("If-None-Match");
      if (h != nullptr && h->value() == asset->etag)
      {
        request->send(304);
        Log(DBG, "UiAdapter::servePath::304(path=%s)", path.c_str());
        return true;
      }
    }

    auto *response = request->beginResponse_P(200, asset->mime, asset->data, asset->size);
    response->addHeader("Connection", "close");
    response->addHeader("Content-Encoding", "gzip");
    response->addHeader("ETag", asset->etag);
    response->addHeader("Last-Modified", asset->time);
    request->send(response);
    Log(DBG, "UiAdapter::servePath::200(path=%s size=%d)", path.c_str(), asset->size);
    return true;
  }

  return false;
}

void UiAdapter::handleRequest(AsyncWebServerRequest *request)
{
  if (!auth(request))
    return;

  String name = request->url();
  if (name == "/")
    name = "/index.html";

  if (servePath(request, name))
    return;

  if (name != "/index.html" && servePath(request, "/index.html"))
    return;

  Log(DBG, "UiAdapter::handleRequest::404(path=%s)", name.c_str());
  String err = "not found: " + name;
  request->send(404, "text/plain", err);
}

void UiAdapter::handleApiGetModemInfo(AsyncWebServerRequest *request)
{
  // explicitly allowed without auth
  AsyncJsonResponse *response = new AsyncJsonResponse();
  const JsonObject &root = response->getRoot();
  ArduMower::Modem::Settings::Properties.marshal(root);
  response->setLength();
  request->send(response);
}

void UiAdapter::handleApiGetModemSettings(AsyncWebServerRequest *request)
{
  if (!auth(request))
    return;

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument doc(1024);
  const JsonObject &o = doc.to<JsonObject>();
  _settings.marshal(o);
  _settings.stripSecrets(o);
  serializeJson(doc, *response);
  request->send(response);
}

void UiAdapter::handleApiPostModemSettings(AsyncWebServerRequest *request, JsonVariant &json)
{
  if (!auth(request))
    return;

  Settings::Settings uploaded = _settings;
  if (!uploaded.unmarshal(json.as<JsonObject>()))
  {
    reject(request, 400, "decode", "unmarshal");
    return;
  }

  String invalid;
  if (!uploaded.valid(invalid))
  {
    reject(request, 400, "validate", invalid);
    return;
  }

  if (!uploaded.save())
  {
    Log(ERR, "UiAdapter::handleApiPostModemSettings::500(save)");
    reject(request, 500, "save", "unknown-error");
    return;
  }

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument doc(1024);
  uploaded.marshal(doc.to<JsonObject>());
  serializeJson(doc, *response);
  request->send(response);

  delayedRestart();
}

void UiAdapter::handleApiResetModemSettings(AsyncWebServerRequest *request)
{
  if (!auth(request))
    return;

  Settings::Settings replace(_settings.filename());
  if (!replace.save())
  {
    request->send(500, "text/plain", "save error");
    return;
  }

  AsyncResponseStream *response = request->beginResponseStream("application/json");
  DynamicJsonDocument doc(1024);
  replace.marshal(doc.to<JsonObject>());
  serializeJson(doc, *response);
  request->send(response);

  delayedRestart();
}

void UiAdapter::handleApiGetRobotDesiredState(AsyncWebServerRequest *request)
{
  if (!auth(request))
    return;

  auto desiredState = _source.desiredState();
  auto res = ArduMower::Domain::Json::encode(desiredState);
  request->send(200, "application/json", res);
}

void UiAdapter::handleApiResetModemBluetoothPairings(AsyncWebServerRequest *request)
{
  if (!auth(request))
    return;
  
  _api.ble->clearPairings();
  request->send(200, "application/json", "{\"result\":\"ok\"}");

  delayedRestart();
}

void UiAdapter::delayedRestart()
{
  delay(100);

  _api.os.restart();
}
