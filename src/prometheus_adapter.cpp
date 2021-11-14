#include "prometheus_adapter.h"
#include "prometheus.h"
#include "log.h"

using namespace ArduMower::Modem::Prometheus;

Adapter::Adapter(Settings::Settings &settings,
                 AsyncWebServer &server,
                 ArduMower::Domain::Robot::StateSource &source,
                 ArduMower::Domain::Robot::CommandExecutor &cmd)
    : _settings(settings), _server(server), _source(source), _cmd(cmd)
{
}

void Adapter::begin()
{
  if (!_settings.prometheus.enabled)
    return;

  auto props = _source.propsP();
  auto state = _source.stateP();
  auto statistics = _source.statsP();

  registerSystemProperties();
  registerRobotProperties(props);
  registerRobotState(state);
  registerRobotStatistics(statistics);

  _server.on("/metrics", HTTP_GET, std::bind(&Adapter::metrics, this, std::placeholders::_1));
}

void Adapter::metrics(AsyncWebServerRequest *request)
{
  auto send = [&](ArduMower::Domain::Robot::Stats::Stats stats,
                  ArduMower::Domain::Robot::State::State status)
  {
    Log(DBG, "Prometheus::Adapter::metrics::send");
    unsigned int size = 1;
    for (auto it : ArduMower::Modem::Prometheus::_allMeasurements)
      size += it->length();
    char *buffer = (char *)malloc(size + 1);
    if (buffer == nullptr)
    {
      request->send(500, "text/plain", "no heap");
      return;
    }
    unsigned int index = 0;
    for (auto it : ArduMower::Modem::Prometheus::_allMeasurements)
    {
      index += it->write(&buffer[index], size - index);
    }
    request->_tempObject = buffer;
    request->send_P(200, "text/plain", (const unsigned char *)buffer, index);
  };

  auto status = _source.state();
  const uint32_t timeout = millis() + 2000;
  while (true)
  {
    const uint32_t now = millis();
    if (now > timeout)
    {
      request->send(504, "text/plain", "refresh timeout");
      return;
    }

    auto stats = _source.stats();
    if (now - stats.timestamp < 15000)
    {
      send(stats, status);
      return;
    }

    static uint32_t next_request = 0;
    if (now < next_request)
    {
      delay(10);
      continue;
    }
    next_request = now + 500;

    Log(DBG, "Prometheus::Adapter::metrics::request-refresh");
    _cmd.requestStats();
  }
}
