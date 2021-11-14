#include "led.h"
#include <Arduino.h>

using namespace ArduMower::Modem;

Led::Led(int pin, bool activeHigh) : _pin(pin), _activeHigh(activeHigh) 
{

}

void Led::setup()
{
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, _activeHigh ? LOW : HIGH);
}

void Led::loop()
{
}

void Led::on()
{
  if (_on) return;
  _on = true;
  digitalWrite(_pin, _activeHigh ? HIGH : LOW);
}

void Led::off()
{
  if (!_on) return;
  _on = false;
  digitalWrite(_pin, _activeHigh ? LOW : HIGH);
}
