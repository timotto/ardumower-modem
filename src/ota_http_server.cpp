#include "ota_http_server.h"
#include "log.h"
#include <Update.h>
#include <AsyncJson.h>

using namespace ArduMower::Modem::Ota;
using namespace std::placeholders;

const char *resultToString(Http::Result r);

HttpServer::HttpServer(ArduMower::Modem::Settings::Settings & settings, AsyncWebServer &server)
    : ArduMower::Modem::Http::Common(settings), _server(server), _active(false), _failed(false), _restart(false), _restartTime(0) {}

void HttpServer::begin()
{
  auto uploadRequestHandler = std::bind(&HttpServer::handleUploadRequest, this, _1);
  auto uploadHandler = std::bind(&HttpServer::handleUpload, this, _1, _2, _3, _4, _5, _6);

  _server.on("/api/modem/ota/upload", HTTP_POST, uploadRequestHandler, uploadHandler);

  auto postRequestHandler = std::bind(&HttpServer::handlePostRequest, this, _1);
  auto bodyHandler = std::bind(&HttpServer::handleBody, this, _1, _2, _3, _4, _5);

  _server.on("/api/modem/ota/post", HTTP_POST, postRequestHandler, uploadHandler, bodyHandler);
}

void HttpServer::loop()
{
  loopRestart();
}

void HttpServer::handleUploadRequest(AsyncWebServerRequest *request)
{
  Http::ServerSession *session = (Http::ServerSession *)request->_tempObject;
  if (session == NULL)
  {
    Log(ERR, "Http::ServerSession::handleRequest::session-null");
    reject(request, 400, "upload", "unknown-request");
    return;
  }

  session->respond(request);
}

void HttpServer::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (index == 0)
    beginUpdate(request, index, data, len, final);
  else
    continueUpdate(request, index, data, len, final);
}

void HttpServer::handlePostRequest(AsyncWebServerRequest *request)
{
  Http::ServerSession *session = (Http::ServerSession *)request->_tempObject;
  if (session == NULL)
  {
    Log(ERR, "Http::ServerSession::handlePostRequest::session-null");
    reject(request, 400, "upload", "unknown-request");
    return;
  }

  session->respond(request);
}

void HttpServer::handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  if (index == 0)
    beginUpdate(request, index, data, len, len == total);
  else
    continueUpdate(request, index, data, len, (index + len) == total);
}

void HttpServer::beginUpdate(AsyncWebServerRequest *request, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!auth(request))
    return;

  auto session = new Http::ServerSession(this);
  request->_tempObject = session;
  session->handle(index, data, len, final);
}

void HttpServer::continueUpdate(AsyncWebServerRequest *request, size_t index, uint8_t *data, size_t len, bool final)
{
  Http::ServerSession *session = (Http::ServerSession *)request->_tempObject;
  if (session == NULL)
    return;

  session->handle(index, data, len, final);
}

void HttpServer::requestRestart()
{
  _restartTime = millis() + 1000;
  _restart = true;
}

void HttpServer::loopRestart()
{
  if (!_restart)
    return;

  if (millis() < _restartTime)
    return;

  ESP.restart();
}

// Http::ServerSession

void Http::ServerSession::respond(AsyncWebServerRequest *request)
{

  auto res = new AsyncJsonResponse();
  auto o = res->getRoot();

  auto success = result == Result::SUCCESS;

  // TODO unify with error handling from http_common.h
  o["success"] = success;
  o["result"] = resultToString(result);

  if (success)
    o["md5"] = Update.md5String();

  Log(INFO, "Ota::Http::ServerSession::respond(%d / %s)", (int)result, resultToString(result));
  res->setLength();
  request->send(res);
  if (success)
    s->requestRestart();
  else
    Update.abort();
}

void Http::ServerSession::handle(size_t index, uint8_t *data, size_t len, bool final)
{
  if (!(result == Result::PENDING || result == Result::STARTED))
    return;

  if (_index != index)
  {
    Log(ERR, "Ota::Http::ServerSession::handle::index-mismatch(expect=%u is=%u)", _index, index);
    result = Result::INDEX_MISMATCH;
    return;
  }

  if (index == 0)
  {
    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
    {
      Log(ERR, "Ota::Http::ServerSession::handle::update-begin-error(%s)", Update.errorString());
      result = Result::UPDATE_BEGIN_FAILED;
      return;
    }

    if (!verifyHeader(data, len))
    {
      result = Result::VERIFY_HEADER_FAILED;
      return;
    }

    Log(INFO, "Ota::Http::ServerSession::handle::update-begin");
    result = Result::STARTED;
  }

  auto n = Update.write(data, len);
  if (n != len)
  {
    Log(ERR, "Ota::Http::ServerSession::handle::update-write-error(len=%d written=%d error=%s)", len, n, Update.errorString());
    result = Result::SHORT_WRITE_ERROR;
    return;
  }
  _index += len;

  if (!final)
    return;

  if (!Update.end(true))
  {
    Log(ERR, "Ota::Http::ServerSession::handle::update-end-error(%s)", Update.errorString());
    result = Result::UPDATE_END_FAILED;
    return;
  }

  Log(INFO, "Ota::Http::ServerSession::handle::update-end");
  result = Result::SUCCESS;
}

bool Http::ServerSession::verifyHeader(uint8_t *data, size_t len)
{
  if (len < 1)
  // return false;
  {
    Log(INFO, "Ota::Http::ServerSession::verifyHeader::error::length");
    return false;
  }
  if (data[0] != 0xe9)
  // return false;
  {
    Log(INFO, "Ota::Http::ServerSession::verifyHeader::error::magic");
    return false;
  }
  // -00000000  e9 06 02 2f f0 4a 08 40  ee 00 00 00 00 00 00 00  |.../.J.@........|
  // +00000000  e9 06 02 2f 04 48 08 40  ee 00 00 00 00 00 00 00  |.../.H.@........|

  // -00000010  00 00 00 00 00 00 00 01  20 00 40 3f e0 59 05 00  |........ .@?.Y..|
  // +00000010  00 00 00 00 00 00 00 01  20 00 40 3f 3c 5f 05 00  |........ .@?<_..|
  // +00000010  00 00 00 00 00 00 00 01  20 00 40 3f f0 52 03 00  |........ .@?.R..|

  // if (len < )
  return true;
}

static const char *result_success = "success";
static const char *result_started = "started";
static const char *result_incomplete = "incomplete";
static const char *result_error = "error";
static const char *result_index_mismatch = "index_mismatch";
static const char *result_update_begin_failed = "update_begin_failed";
static const char *result_verify_header_failed = "verify_header_failed";
static const char *result_short_write_error = "short_write_error";
static const char *result_update_end_failed = "update_end_failed";
static const char *result_unknown = "unknown";

const char *resultToString(Http::Result r)
{
  switch (r)
  {
  case Http::Result::SUCCESS:
    return result_success;

  case Http::Result::STARTED:
    return result_started;

  case Http::Result::INCOMPLETE:
    return result_incomplete;

  case Http::Result::ERROR:
    return result_error;

  case Http::Result::INDEX_MISMATCH:
    return result_index_mismatch;

  case Http::Result::UPDATE_BEGIN_FAILED:
    return result_update_begin_failed;

  case Http::Result::VERIFY_HEADER_FAILED:
    return result_verify_header_failed;

  case Http::Result::SHORT_WRITE_ERROR:
    return result_short_write_error;

  case Http::Result::UPDATE_END_FAILED:
    return result_update_end_failed;

  default:
    return result_unknown;
  }
}
