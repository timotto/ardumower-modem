#include "http_adapter.h"

using namespace ArduMower::Modem;

extern void respondWithCors(AsyncWebServerRequest *req, int status, String contentType, String responseBody);

Http::CommandRequest::CommandRequest(
    const uint32_t _id,
    Metrics *metrics,
    AsyncWebServerRequest *_request,
    const uint32_t timeNow)
    : id(_id), _metrics(metrics), request(_request), timeReceiveHttpRequest(timeNow), state(0), _done(false),
      httpRequestBody(""), routerResponse("")
{
  parseHttpRequestBody();
}

bool Http::CommandRequest::done(const uint32_t now)
{
  if (_done)
    return true;

  return timeout(now);
}

bool Http::CommandRequest::timeout(const uint32_t now)
{
  if (now - timeReceiveHttpRequest < 1000)
    return false;

  _done = true;
  reject(504, "timeout");

  return true;
}

void Http::CommandRequest::parseHttpRequestBody()
{
  Log(DBG, "Http::CommandRequest::parseHttpRequestBody");
  readHttpRequestBody();
  trimHttpRequestBody();
  validateHttpRequestBody();
}

void Http::CommandRequest::readHttpRequestBody()
{
  String value = "";
  auto n = request->params();
  for (auto i = 0; i < n; i++)
  {
    auto p = request->getParam(i);
    if (!p->isPost())
      continue;

    if (p->name() == "body")
      continue;

    if (value != "")
      value += "&";
    value += p->name();
    value += "=";
    value += p->value();
  }

  if (request->hasParam("body", true))
  {
    auto body = request->getParam("body", true);
    value = body->value();
    if (value.startsWith("AT "))
    {
      value = "AT+" + value.substring(3);
    }
    httpRequestBody = value;
    return;

    // TODO test first: currently either "body" or "key=val" but what if body is like "a=b&c" or "a&b=c"?
    // if (value != "")
    //   value += "&";
    // value += body->value();
  }

  if (value.startsWith("AT "))
  {
    value = "AT+" + value.substring(3);
  }

  httpRequestBody = value;
}

void Http::CommandRequest::trimHttpRequestBody()
{
  while (httpRequestBody.endsWith("\n") || httpRequestBody.endsWith("\r"))
    httpRequestBody = httpRequestBody.substring(0, httpRequestBody.length() - 1);
}

void Http::CommandRequest::validateHttpRequestBody()
{
  if (httpRequestBody == "")
  {
    Log(DBG, "Http::CommandRequest::validateHttpRequestBody::error::empty-body");
    reject(400, "empty body");
    return;
  }
}

void Http::CommandRequest::reject(int code, String text)
{
  _done = true;
  respondWithCors(request, code, "text/plain", text);
  _metrics->countStatusCode(code);
}

void Http::CommandRequest::onRouterResponse(String response)
{
  Log(DBG, "Http::CommandRequest::onRouterResponse [%s]", response.c_str());

  response += "\r\n";
  _done = true;

  respondWithCors(request, 200, "text/html; charset=UTF-8", response);
  _metrics->countStatusCode(200);
}
