#pragma once

namespace ArduMower
{
  namespace Modem
  {
    class Led
    {
    private:
      int _pin;
      bool _activeHigh;
      bool _on;

      void on();
      void off();
      void update();

    public:
      Led(int pin, bool activeHigh);

      void setup();
      void loop();
    };
  }
}
