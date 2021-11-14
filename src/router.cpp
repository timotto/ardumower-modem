#include "router.h"
#include "log.h"

using namespace ArduMower::Modem;

// expect an answer from ArduMower within expectDownResponseTimeout ms or respond to caller with "Error::TIMEOUT"
static const uint32_t expectDownResponseTimeout = 1000;

void Router::begin()
{
  while (down.available())
    down.read();
}

void Router::loop()
{
  loopSend();
  loopReceive();
  loopTimeout();
}

void Router::sniffRx(RxDrain *d)
{
  drains.push_back(d);
}

void Router::sniffTx(TxDrain *d)
{
  txDrains.push_back(d);
}

bool Router::send(String _command, responseCb _cb)
{
  // reject until idle
  if (sendCommand || expectResponse)
    return false;

  lastCommand = _command;
  command = _command + "\r\n";
  cb = _cb;
  sendCommand = true;
  expectResponse = true;

  return true;
}

bool Router::sendWithoutResponse(String line)
{
  if (sendCommand || expectResponse)
    return false;

  lastCommand = line;
  command = line + "\r\n";
  cb = [](String r, int err) {};
  sendCommand = true;

  return true;
}

void Router::loopSend()
{
  if (!sendCommand) return;

  auto sent = down.print(command.c_str());
  down.flush();

  if (sent < command.length())
  {
    command = command.substring(sent);
    return;
  }

  sendCommand = false;
  lastTx = _millis();

  bool stop = false;
  for (auto it : txDrains)
  {
    it->drainTx(lastCommand, stop);
    if (stop)
      break;
  }
}

void Router::loopReceive()
{
  while (down.available())
  {
    char c = down.read();
    if (c == -1)
      continue;
    
    lastRx = _millis();

    String line = downRx.update(c);
    if (line == "")
      continue;

    if (expectResponse)
    {
      expectResponse = false;
      cb(line, XferError::SUCCESS);
    }

    if (!drains.empty())
    {
      bool stop = false;
      for (auto it : drains)
      {
        it->drainRx(line, stop);
        if (stop)
          break;
      }
    }

    return;
  }
}

void Router::loopTimeout()
{
  if (isRxTimeout())
  {
    expectResponse = false;
    cb("", XferError::TIMEOUT);
  }
}

bool Router::isRxTimeout()
{
  if (!expectResponse) return false;

  // either time of "send to downstream"
  // or time of last "receive from downstream"
  const uint32_t limit = lastTx > lastRx ? lastTx : lastRx;

  return _millis() >= limit + expectDownResponseTimeout;
}
