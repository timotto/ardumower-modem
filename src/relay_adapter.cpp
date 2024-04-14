#include "relay_adapter.h"
#include "log.h"
#include "trust.h"
extern "C"
{
#include "base64.h"
}

using namespace ArduMower::Modem;
using namespace ArduMower::Util;
using namespace websockets;

// exponential backoff: 1s .. 60s with 1.2X increase per attempt
// reaches 60s max in ~5.5 minutes with ~20 attempts
#define backoff_min 1000
#define backoff_max 60000
#define backoff_factor 1.2

RelayAdapter::RelayAdapter(Settings::Settings &s, Router &r)
    : settings(s), router(r),
      count(0), countMetric(
                    new Prometheus::CallbackMeasurement(
                        "ardumower_modem_relay_request_count",
                        std::bind(
                            &RelayAdapter::writeCount,
                            this,
                            std::placeholders::_1,
                            std::placeholders::_2))),
      connected(false), backoff(Backoff(backoff_min, backoff_max, backoff_factor)),
      lastConnect(0), lastPing(0), lastPong(0),
      rtt(0), _connectionCount(0), _connectionDuration(0)
{
  if (!settings.relay.enabled)
    return;
}

void RelayAdapter::begin()
{
  if (!settings.relay.enabled)
    return;

  client.setCACert(tls_ca_trust);

  client.onMessage(std::bind(&RelayAdapter::onMessageCallback, this, std::placeholders::_1));
  client.onEvent(std::bind(&RelayAdapter::onEventsCallback, this, std::placeholders::_1, std::placeholders::_2));
  setupAuthorizationHeader();
  client.addHeader("Ardumower-Relay-Client-Robot-Timeout", String(1000));
  client.addHeader("Ardumower-Relay-Client-Ping-Interval", String(settings.relay.pingInterval));
}

void RelayAdapter::setupAuthorizationHeader()
{
  if (settings.relay.username == "" || settings.relay.password == "")
    return;

  String plain = settings.relay.username + ":" + settings.relay.password;
  String encoded = base64::encode((const unsigned char *)plain.c_str(), plain.length());
  size_t n = encoded.length();


  // why does base64_encode append a single "\r" / 0x0a character?
  if (n > 0 && encoded[n-1] == '\r')
    encoded = encoded.substring(0,n-1);

  String authValue = "Basic " + encoded;
  client.addHeader("Authorization", authValue);
}

void RelayAdapter::loop()
{
  if (!settings.relay.enabled)
    return;

  client.poll();

  if (!loopConnection())
    return;

  loopTransfer();
  loopPing();
}

bool RelayAdapter::loopConnection()
{
  static bool was_connected = true;

  if (connected)
  {
    if (!was_connected)
    {
      was_connected = true;
      backoff.reset();
    }

    return true;
  }

  static uint32_t next = 0;
  const uint32_t now = millis();

  if (was_connected)
  {
    next = now + backoff.next();
    was_connected = false;

    return false;
  }

  if (now < next)
    return false;
  next = now + backoff.next();

  if (client.connect(settings.relay.url))
  {
    Log(DBG, "RelayAdapter::connect() - success");
  }
  else
  {
    Log(DBG, "RelayAdapter::connect() - failed");
  }

  return false;
}

void RelayAdapter::loopTransfer()
{
  if (sendToMower != "" &&
      router.send(
          sendToMower,
          std::bind(&RelayAdapter::onRouterResponse, this, std::placeholders::_1, std::placeholders::_2)))
    sendToMower = "";

  if (sendToRelay != "" && client.send(sendToRelay))
    sendToRelay = "";
}

void RelayAdapter::onRouterResponse(String res, int err)
{
  switch (err)
  {
  case ArduMower::Modem::XferError::SUCCESS:
    sendToRelay = res + "\r\n";
    break;
  
  case ArduMower::Modem::XferError::TIMEOUT:
    Log(ERR, "RelayAdapter::onRouterResponse() - timeout");
    // no response causes timeout error in relay server
    break;
  
  default:
    Log(ERR, "RelayAdapter::onRouterResponse() - error %d", err);
    // there is no way to signal errors as the transport is transparent
    // fallback to timeout error in relay server
    break;
  }
}

void RelayAdapter::loopPing()
{
  static uint32_t nextPing = 0;
  const uint32_t now = millis();
  if (now < nextPing)
    return;

  if (client.ping())
  {
    lastPing = millis();
    nextPing = lastPing + pingInterval();
  }
}

uint32_t RelayAdapter::pingInterval()
{
  if (settings.relay.pingInterval == 0)
    return 60000;

  return settings.relay.pingInterval;
}

unsigned int RelayAdapter::writeCount(char *buffer, unsigned int size)
{
  return snprintf(buffer, size, "%u", count);
}

void RelayAdapter::onMessageCallback(WebsocketsMessage message)
{
  if (!message.isText())
    return;

  count++;

  String tmp = message.c_str();
  while (tmp.endsWith("\n") || tmp.endsWith("\r"))
    tmp = tmp.substring(0, tmp.length() - 1);

  sendToMower = tmp;
}

void RelayAdapter::onEventsCallback(WebsocketsEvent event, String data)
{
  switch (event)
  {
  case WebsocketsEvent::ConnectionOpened:
    connected = true;
    _connectionCount++;
    lastConnect = millis();
    break;

  case WebsocketsEvent::ConnectionClosed:
    _connectionDuration = (0.5 * _connectionDuration) + (0.5 * (millis() - lastConnect) / 1000.0f);
    connected = false;
    break;

  case WebsocketsEvent::GotPing:
    break;

  case WebsocketsEvent::GotPong:
    lastPong = millis();
    rtt = (0.5f * rtt) + (0.5f * (float)(lastPong - lastPing) / 1000.0f);
    break;
  }
}

bool RelayAdapter::isConnected()
{
  return connected;
}

float RelayAdapter::pingRTT()
{
  return rtt;
}

int RelayAdapter::connectionCount()
{
  return _connectionCount;
}

uint32_t RelayAdapter::connectionTime()
{
  return lastConnect;
}

float RelayAdapter::connectionDuration()
{
  return _connectionDuration;
}