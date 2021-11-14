#pragma once

#include <Arduino.h>

class Reader
{
private:
  String eol;
  String buffer;

public:
  Reader(String _eol) : eol(_eol), buffer("") {}
  void reset();
  String update(char c);
  String peek();
};
