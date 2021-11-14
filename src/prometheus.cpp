#include "prometheus.h"
#include <string.h>
#include <stdio.h>

using namespace ArduMower::Modem::Prometheus;

std::list<Measurement *> ArduMower::Modem::Prometheus::_allMeasurements;

Measurement::Measurement(const char *key, Attributes attrs)
    : _key(key), _attrs(attrs)
{
  _allMeasurements.push_back(this);
}

Measurement::~Measurement()
{
  _allMeasurements.remove(this);
}

int Measurement::write(char *buf, int size)
{
  if (size < length() + 1)
    return 0;
  int index = 0;
  index += snprintf(&buf[index], size - index, "%s", _key);
  index += _attrs.write(&buf[index], size - index);
  buf[index++] = ' ';
  index += writeValue(&buf[index], size - index);
  buf[index++] = '\n';
  buf[index] = 0;

  return index;
}

unsigned int Measurement::length()
{
  unsigned int result = 0;

  result += strlen(_key);
  result += _attrs.length();
  result += 1; // ' '
  result += valueLength();
  result += 1; // '\n'

  return result;
}

Attributes &Attributes::add(Attribute a)
{
  attrs.push_back(a);
  return (*this);
}

Attributes Attributes::clone()
{
  Attributes cpy;
  for (auto it : attrs)
    cpy.add(it);
  return cpy;
}

unsigned int Attributes::length() const
{
  if (attrs.empty())
    return 0;

  unsigned int result = 2 + attrs.size() - 1; // 2 for {} and count-1 for ,
  for (auto it : attrs)
    result += it.length();
  return result;
}

int Attributes::write(char *buf, int len)
{
  unsigned int l = length();
  if (len < l)
    return 0;

  if (l == 0)
    return 0;

  unsigned int total = 0;
  buf[total++] = '{';
  int i = 0;
  for (auto it : attrs)
  {
    if (i > 0)
      buf[total++] = ',';
    const int written = it.write(&buf[total], len - total);
    total += written;
  }
  buf[total++] = '}';

  return total;
}

int Attribute::write(char *buf, int len)
{
  const unsigned int l = length();
  if (l == 0)
    return 0;

  if (len < l)
    return 0;

  sprintf(buf, "%s=\"%s\"", key, value);
  return l;
}

unsigned int Attribute::length() const
{
  if (key == nullptr || value == nullptr)
    return 0;

  return strlen(key) + strlen(value) + 3; // =""
}
