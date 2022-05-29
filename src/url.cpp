#include "url.h"

using namespace ArduMower::Util;

URL::URL(const String & _url)
: _scheme(""), _hostname(""), _port(-1), _path("")
{
  String url = _url;
  
  int index;
  index = url.indexOf("://");
  if (index != -1)
  {
    _scheme = url.substring(0, index);
    url = url.substring(index + 3);
  }

  index = url.indexOf(":");
  if (index != -1)
  {
    _hostname = url.substring(0, index);
    url = url.substring(index + 1);

    index = url.indexOf("/");
    if (index != -1)
    {
      _port = url.substring(0, index).toInt();
      _path = url.substring(index);
    } else {
      _port = url.toInt();
    }
  } else {
    index = url.indexOf("/");
    if (index != -1)
    {
      _hostname = url.substring(0, index);
      _path = url.substring(index);
    } else {
      _hostname = url;
    }
  }
}

String URL::scheme()
{
  return _scheme;
}

String URL::hostname()
{
  return _hostname;
}

int URL::port()
{
  return _port;
}

String URL::path()
{
  return _path;
}
