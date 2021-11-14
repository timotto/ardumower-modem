#include "http_common.h"

using namespace ArduMower::Modem::Http;

Common::Common(ArduMower::Modem::Settings::Settings &_settings) : settings(_settings) {}

bool Common::auth(AsyncWebServerRequest *request)
{
  if (!settings.web.use_password)
    return true;

  if (settings.web.username.length() == 0 || settings.web.password.length() == 0)
    return true;

  if (request->authenticate(settings.web.username.c_str(), settings.web.password.c_str()))
    return true;

  request->requestAuthentication();

  return false;
}

void Common::reject(AsyncWebServerRequest *request, int statusCode, const String & action, const String & error)
{
  AsyncResponseStream *response = request->beginResponseStream("application/json");
  response->setCode(statusCode);

  DynamicJsonDocument doc(1024);
  doc["success"] = false; // TODO HTTP OTA uses "success" true/false - get rid of "success" and unify error handling
  doc["error"] = error.c_str();
  doc["action"] = action.c_str();
  serializeJson(doc, *response);

  request->send(response);
}
