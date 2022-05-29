#include <AUnit.h>

#include "../src/url.h"

test(url_parse_scheme)
{
  ArduMower::Util::URL url("mqtt://server-hostname.example.com:1883");

  assertEqual(url.scheme(), "mqtt");
}

test(url_parse_hostname)
{
  ArduMower::Util::URL url("mqtt://server-hostname.example.com:1883");

  assertEqual(url.hostname(), "server-hostname.example.com");
}

test(url_parse_hostname_no_scheme)
{
  ArduMower::Util::URL url("server-hostname.example.com:1883");

  assertEqual(url.hostname(), "server-hostname.example.com");
}

test(url_parse_hostname_no_port)
{
  ArduMower::Util::URL url("mqtt://server-hostname.example.com");

  assertEqual(url.hostname(), "server-hostname.example.com");
}

test(url_parse_hostname_no_scheme_no_port)
{
  ArduMower::Util::URL url("server-hostname.example.com");

  assertEqual(url.hostname(), "server-hostname.example.com");
}

test(url_parse_port)
{
  ArduMower::Util::URL url("mqtt://server-hostname.example.com:1883");

  assertEqual(url.port(), 1883);
}

test(url_parse_port_no_scheme)
{
  ArduMower::Util::URL url("server-hostname.example.com:1883");

  assertEqual(url.port(), 1883);
}

test(url_parse_no_port)
{
  ArduMower::Util::URL url("mqtt://server-hostname.example.com");

  assertEqual(url.port(), -1);
}

test(url_parse_with_path)
{
  ArduMower::Util::URL url("mqtt://server-hostname.example.com:1883/some/path");

  assertEqual(url.hostname(), "server-hostname.example.com");
  assertEqual(url.port(), 1883);
  assertEqual(url.path(), "/some/path");
}

test(url_parse_no_port_with_path)
{
  ArduMower::Util::URL url("mqtt://server-hostname.example.com/some/path");

  assertEqual(url.hostname(), "server-hostname.example.com");
  assertEqual(url.port(), -1);
  assertEqual(url.path(), "/some/path");
}
