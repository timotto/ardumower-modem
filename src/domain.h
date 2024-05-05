#pragma once

#include <Arduino.h>
#include <inttypes.h>

namespace ArduMower
{
  namespace Domain
  {
    namespace Robot
    {
      class Properties
      {
      public:
        uint32_t timestamp;
        String firmware;
        String version;

        bool operator==(const Properties &other);
        bool operator!=(const Properties &other) { return !(*this == other); }
      };

      namespace Stats
      {
        class Durations
        {
        public:
          uint32_t idle, charge, mow, mowFloat, mowFix, mowInvalid;

          bool operator==(const Durations &other);
          bool operator!=(const Durations &other) { return !(*this == other); }
        };

        class Recoveries
        {
        public:
          uint32_t mowFloatToFix;
          uint32_t imu;
          uint32_t mowInvalid;

          bool operator==(const Recoveries &other);
          bool operator!=(const Recoveries &other) { return !(*this == other); }
        };

        class Obstacles
        {
        public:
          uint32_t count;
          uint32_t sonar;
          uint32_t bumper;
          uint32_t gpsMotionLow;

          bool operator==(const Obstacles &other);
          bool operator!=(const Obstacles &other) { return !(*this == other); }
        };

        class Stats
        {
        public:
          uint32_t timestamp;
          Durations durations;
          Recoveries recoveries;
          Obstacles obstacles;
          float mowDistanceTraveled;
          float mowMaxDgpsAge;
          float tempMin, tempMax;
          uint32_t gpsChecksumErrors;
          uint32_t dgpsChecksumErrors;
          float maxMotorControlCycleTime;
          uint32_t serialBufferSize;
          uint32_t freeMemory;
          int resetCause;

          uint32_t gpsJumps;

          bool operator==(const Stats &other);
          bool operator!=(const Stats &other) { return !(*this == other); }
        };
      }

      namespace State
      {
        class Point
        {
        public:
          float x, y;

          Point()
              : x(0), y(0){};

          bool operator==(const Point &other);
          bool operator!=(const Point &other) { return !(*this == other); }
        };

        class Position : public Point
        {
        public:
          float delta;
          int solution;
          float age;
          float accuracy;
          int visibleSatellites, visibleSatellitesDgps;
          int mowPointIndex;

          Position()
              : delta(0),
                solution(0), age(0), accuracy(0), visibleSatellites(0), visibleSatellitesDgps(0),
                mowPointIndex(0){};

          bool operator==(const Position &other);
          bool operator!=(const Position &other) { return !(*this == other); }
        };

        class State
        {
        public:
          uint32_t timestamp;
          float batteryVoltage;
          Position position;
          Point target;
          int job;
          int sensor;
          float amps;
          int mapCrc;
          float temperature;
          float chargingMah;
          float motorMowMah;
          float motorLeftMah;
          float motorRightMah;

          State()
              : batteryVoltage(0), job(0), sensor(0), amps(0), mapCrc(0), temperature(0), chargingMah(0), motorMowMah(0), motorLeftMah(0), motorRightMah(0)
          {
          }

          bool operator==(const State &other);
          bool operator!=(const State &other) { return !(*this == other); }
        };
      }

      class DesiredState
      {
      public:
        float speed;
        bool mowerMotorEnabled;
        bool finishAndRestart;
        int op;
        int fixTimeout;

        DesiredState() : speed(0), mowerMotorEnabled(false), finishAndRestart(false), op(-1), fixTimeout(-1){};
      };

      class StateSource
      {
      public:
        virtual ArduMower::Domain::Robot::State::State state() = 0;
        virtual ArduMower::Domain::Robot::Stats::Stats stats() = 0;
        virtual ArduMower::Domain::Robot::Properties props() = 0;
        virtual DesiredState desiredState() = 0;

        virtual ArduMower::Domain::Robot::State::State *stateP() = 0;
        virtual ArduMower::Domain::Robot::Stats::Stats *statsP() = 0;
        virtual ArduMower::Domain::Robot::Properties *propsP() = 0;
        virtual DesiredState *desiredStateP() = 0;
      };

      class CommandExecutor
      {
      public:
        virtual bool start() = 0;
        virtual bool stop() = 0;
        virtual bool dock() = 0;
        virtual bool changeSpeed(float speed) = 0;
        virtual bool mowerEnabled(bool enabled) = 0;

        virtual bool requestVersion() = 0;
        virtual bool requestStatus() = 0;
        virtual bool requestStats() = 0;
      };
    }
  }
}
