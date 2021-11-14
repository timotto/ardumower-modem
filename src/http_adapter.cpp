#include "http_adapter.h"
#include "prometheus.h"
#include <Arduino.h>

using namespace ArduMower::Modem;

void respondWithCors(AsyncWebServerRequest *req, int status, String contentType, String responseBody);

HttpAdapter::HttpAdapter(Router &router, AsyncWebServer &server)
    : _router(router), _server(server), requestId(0)
{
}

HttpAdapter::~HttpAdapter()
{
  delete _metrics;
}

void HttpAdapter::begin()
{
  _metrics = new Http::Metrics();
  _lock = xSemaphoreCreateBinary();
  xSemaphoreGive(_lock);
  _server.on("/", HTTP_POST, std::bind(&HttpAdapter::handleCommandRequest, this, std::placeholders::_1));
  _server.on("/api/modem/reboot", HTTP_POST, std::bind(&HttpAdapter::apiReboot, this, std::placeholders::_1));
}

void HttpAdapter::loop()
{
  processQueue();
}

void HttpAdapter::processQueue()
{
  std::list<Http::CommandRequest *> keep;

  xSemaphoreTake(_lock, portMAX_DELAY);
  for (auto req : _queue)
  {
    if (req->done(millis()))
    {
      delete req;
      continue;
    }

    processRequest(req);
    keep.push_back(req);
  }
  _queue = keep;
  xSemaphoreGive(_lock);
}

size_t HttpAdapter::queueSize()
{
  xSemaphoreTake(_lock, portMAX_DELAY);
  const size_t result = _queue.size();
  xSemaphoreGive(_lock);

  return result;
}

void HttpAdapter::enqueueRequest(Http::CommandRequest *req)
{
  Log(DBG, "HttpAdapter::enqueueRequest");
  if (queueIsFull())
  {
    Log(DBG, "HttpAdapter::enqueueRequest::reject::full");
    req->reject(500, "request queue full");
    delete req;
    return;
  }

  xSemaphoreTake(_lock, portMAX_DELAY);
  _queue.push_back(req);
  xSemaphoreGive(_lock);
}

bool HttpAdapter::queueIsFull()
{
  return queueSize() >= 10;
}

void HttpAdapter::handleCommandRequest(AsyncWebServerRequest *request)
{
  Log(DBG, "HttpAdapter::handleCommandRequest");
  Http::CommandRequest *req = new Http::CommandRequest(requestId++, _metrics, request, millis());
  if (req->done(millis()))
  {
    Log(DBG, "HttpAdapter::handleCommandRequest::fast");
    delete req;
    return;
  }

  enqueueRequest(req);
}

void HttpAdapter::processRequest(Http::CommandRequest *req)
{
  const uint32_t id = req->id;
  switch (req->state)
  {
  case 0:
    Log(DBG, "HttpAdapter::processRequest::send(id=%d)", id);
    // command from http request body has not been sent to the router yet

    // send http request body as command to modem
    if (_router.send(req->httpRequestBody,
                     [=](String res, int err)
                     { handleRouterResponse(id, res); }))
      req->state = 1;
    break;

  case 1:
    // response from the router has not been received yet
    break;
  }
}

void HttpAdapter::handleRouterResponse(const uint32_t id, String res)
{
  Log(DBG, "HttpAdapter::handleRouterResponse");
  // *req might have been deleted already

  bool found = false;
  xSemaphoreTake(_lock, portMAX_DELAY);
  for (auto it : _queue)
  {
    if (it->id != id)
      continue;

    it->onRouterResponse(res);
    found = true;
    Log(DBG, "HttpAdapter::handleRouterResponse::success");
  }
  xSemaphoreGive(_lock);

  if (!found)
    Log(DBG, "HttpAdapter::handleRouterResponse::not-found(id=%d)", id);
}

void HttpAdapter::apiReboot(AsyncWebServerRequest *req)
{
  req->send(200, "text/plain", "rebooting");
  delay(500);
  ESP.restart();
}

void respondWithCors(AsyncWebServerRequest *req, int status, String contentType, String responseBody)
{
  AsyncWebServerResponse *res = req->beginResponse(status, contentType, responseBody);
  res->addHeader("Access-Control-Allow-Origin", "*");
  req->send(res);
}
