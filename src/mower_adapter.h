#ifndef _MOWER_ADAPTER_H
#define _MOWER_ADAPTER_H

#include "domain.h"
#include "router.h"
#include "encrypt.h"
#include "settings.h"

namespace ArduMower
{

  namespace Modem
  {
    class MowerAdapter : public RxDrain, public TxDrain, public ArduMower::Domain::Robot::StateSource, public ArduMower::Domain::Robot::CommandExecutor
    {
    private:
      Settings::Settings &settings;
      Router &router;

      bool sendIsInitialized;
      Encrypt enc;
      ArduMower::Domain::Robot::Properties _props;
      ArduMower::Domain::Robot::State::State _state;
      ArduMower::Domain::Robot::Stats::Stats _stats;
      ArduMower::Domain::Robot::DesiredState _desiredState;

      void parseArduMowerCommand(String line);
      void parseArduMowerResponse(String line);
      void parseVersionResponse(String line);
      void parseStateResponse(String line);
      void parseStatisticsResponse(String line);
      void parseATCCommand(String line);

      bool sendCommand(String command, bool encrypt = true);
      bool assertSendIsInitialized();

    public:
      MowerAdapter(Settings::Settings &_settings, Router &_router);

      void begin();

      virtual ArduMower::Domain::Robot::Properties props() { return _props; }
      virtual ArduMower::Domain::Robot::State::State state() { return _state; };
      virtual ArduMower::Domain::Robot::Stats::Stats stats() { return _stats; };
      virtual ArduMower::Domain::Robot::DesiredState desiredState() { return _desiredState; }
      virtual ArduMower::Domain::Robot::Properties *propsP() { return &_props; }
      virtual ArduMower::Domain::Robot::State::State *stateP() { return &_state; };
      virtual ArduMower::Domain::Robot::Stats::Stats *statsP() { return &_stats; };
      virtual ArduMower::Domain::Robot::DesiredState *desiredStateP() { return &_desiredState; }

      virtual bool start();
      virtual bool stop();
      virtual bool dock();
      virtual bool changeSpeed(float speed);
      virtual bool setWaypoint(float waypoint);
      virtual bool finishAndRestartEnabled(bool enabled);
      virtual bool mowerEnabled(bool enabled);
      virtual bool requestVersion();
      virtual bool requestStatus();
      virtual bool requestStats();

      virtual void drainRx(String line, bool &stop) override;
      virtual void drainTx(String line, bool &stop) override;
    };
  }
}
#endif
