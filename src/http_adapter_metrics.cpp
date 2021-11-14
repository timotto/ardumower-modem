#include "http_adapter.h"
#include <Arduino.h>

using namespace ArduMower::Modem;
using namespace ArduMower::Modem::Prometheus::Measurements;
using namespace ArduMower::Modem::Prometheus;

static const char *metricKey = "ardumower_modem_http_request_count";
static const char *attrKey = "status";

Http::Metrics::Metrics()
    : status200(new Value(metricKey, Attributes(Attribute(attrKey, "200")))),
      status400(new Value(metricKey, Attributes(Attribute(attrKey, "400")))),
      status500(new Value(metricKey, Attributes(Attribute(attrKey, "500"))))

{
}

Http::Metrics::~Metrics()
{
  delete status200;
  delete status400;
  delete status500;
}

void Http::Metrics::countStatusCode(int code)
{
  if (code >= 200 && code < 300)
    status200->inc();
  else if (code >= 400 && code < 500)
    status400->inc();
  else if (code >= 500 && code < 600)
    status500->inc();
}
