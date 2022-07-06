#ifndef _PS4_CONTROLLER_H
#define _PS4_CONTROLLER_H

#include <Arduino.h>
#include "settings.h"
#include "domain.h"

#define PS4_SEND_INTERVAL 500

namespace ArduMower
{
  namespace Modem
  {
    namespace PS4Controller
    {
      class Adapter
      {
      private:
        Settings::Settings &settings;
        ArduMower::Domain::Robot::CommandExecutor &cmd;
        float oldLiniar = 0;
        float oldAngular = 0;
        uint32_t lastSendTime = 0;
        
      public:
        Adapter(
          ArduMower::Modem::Settings::Settings &_settings,
          ArduMower::Domain::Robot::CommandExecutor &_cmd);

        void begin();
        void loop();
      };
    }
  }
}

#endif // _IOBROKER_H