#include "relay_adapter.h"
#include "log.h"
#include "trust.h"

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
      router.send(sendToMower, [&](String res, int err)
                  { sendToRelay = res + "\r\n"; }))
    sendToMower = "";

  if (sendToRelay != "" && client.send(sendToRelay))
    sendToRelay = "";
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