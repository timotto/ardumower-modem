#include "modem_cli.h"
#include "settings.h"

void ArduMower::Modem::Cli::drainRx(String line, bool &stop)
{
  stop = true;
  if (line == "AT")
    router.sendWithoutResponse("OK");
  else if (line == "AT+VERSION")
    respondVersion();
  else if (line.startsWith("AT+NAME="))
    echoResponse(line);
  else if (line.startsWith("AT+WIFI="))
    echoResponse(line);
  else if (line == "AT+RESET")
    echoResponse(line);
  else if (line == "AT+TEST")
    echoResponse(line);
  else
    stop = false;
}

void ArduMower::Modem::Cli::respondVersion()
{
  String response = String("+VERSION=") + ArduMower::Modem::Settings::Properties.version();

  router.sendWithoutResponse(response);
}

void ArduMower::Modem::Cli::echoResponse(String &req)
{
  String response = req.substring(2);

  router.sendWithoutResponse(response);
}
