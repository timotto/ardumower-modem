#include "ps4_controller.h"
#include "esp_gap_bt_api.h"
#include <PS4Controller.h>

using namespace ArduMower::Modem::PS4Controller;

Adapter::Adapter(ArduMower::Modem::Settings::Settings &_settings, ArduMower::Domain::Robot::CommandExecutor &_cmd): 
    settings(_settings), cmd(_cmd)  
{
}

void Adapter::begin()
{
    PS4.begin("a8:47:4a:78:c5:e6");
    Serial.println("Ready.");
}

void Adapter::loop()
{
    if (PS4.isConnected()) {
        if (PS4.Right()) Serial.println("Right Button");
        if (PS4.Down()) Serial.println("Down Button");
        if (PS4.Up()) Serial.println("Up Button");
        if (PS4.Left()) Serial.println("Left Button");

        // if (PS4.Square()) Serial.println("Square Button");
        // if (PS4.Cross()) Serial.println("Cross Button");
        // if (PS4.Circle()) Serial.println("Circle Button");
        // if (PS4.Triangle()) Serial.println("Triangle Button");

        // if (PS4.UpRight()) Serial.println("Up Right");
        // if (PS4.DownRight()) Serial.println("Down Right");
        // if (PS4.UpLeft()) Serial.println("Up Left");
        // if (PS4.DownLeft()) Serial.println("Down Left");

        // if (PS4.L1()) Serial.println("L1 Button");
        // if (PS4.R1()) Serial.println("R1 Button");

        // if (PS4.Share()) Serial.println("Share Button");
        // if (PS4.Options()) Serial.println("Options Button");
        // if (PS4.L3()) Serial.println("L3 Button");
        // if (PS4.R3()) Serial.println("R3 Button");

        // if (PS4.PSButton()) Serial.println("PS Button");
        // if (PS4.Touchpad()) Serial.println("Touch Pad Button");

        if (PS4.L2()) {
        Serial.printf("L2 button at %d\n", PS4.L2Value());
        }
        if (PS4.R2()) {
        Serial.printf("R2 button at %d\n", PS4.R2Value());
        }

        float liniar = 0;
        float angular = 0;
        if ((PS4.Left() || PS4.Right() || PS4.Up() || PS4.Down() || PS4.UpLeft() || PS4.UpRight() || PS4.DownLeft() || PS4.DownRight()) && (PS4.R2() > 0)) {
            liniar = (PS4.Up() || PS4.UpLeft() || PS4.UpRight() ? 1 : (PS4.Down() || PS4.DownLeft() || PS4.DownRight() ? -1 : 0)) * PS4.R2Value() * 0.3 / 255;
            angular = (PS4.Right() || PS4.UpRight() || PS4.DownRight() ? -1 : (PS4.Left() || PS4.UpLeft() || PS4.DownLeft() ? 1 : 0)) * PS4.R2Value() * 0.5 / 255;
            
        } else if ((abs(PS4.RStickX()) > 10) || (abs(PS4.RStickY()) > 10)) {
            Serial.printf("Right Stick x at %d\n", PS4.RStickX());
            Serial.printf("Right Stick y at %d\n", PS4.RStickY());
            liniar = PS4.RStickY() * 0.1 / 128;
            angular = -PS4.RStickX() * 0.15 / 128;
            
        } else if ((abs(PS4.LStickX()) > 10) || (abs(PS4.LStickY()) > 10)) {
            Serial.printf("Left Stick x at %d\n", PS4.LStickX());
            Serial.printf("Left Stick y at %d\n", PS4.LStickY());

            liniar = PS4.LStickY() * 0.33 / 128;
            angular = -PS4.LStickX() * 0.5 / 128;
        }
        
        if ((liniar != 0) || (angular !=0) || (oldLiniar != 0) || (oldAngular != 0)) {
            if ((millis() - lastSendTime < PS4_SEND_INTERVAL) && ((liniar != 0) || (angular !=0))) 
                return;

            cmd.manualDrive(liniar, angular);
            oldLiniar = liniar;
            oldAngular = angular;
            lastSendTime = millis();
        }   

        // if (PS4.Charging()) Serial.println("The controller is charging");
        // if (PS4.Audio()) Serial.println("The controller has headphones attached");
        // if (PS4.Mic()) Serial.println("The controller has a mic attached");

        // Serial.printf("Battery Level : %d\n", PS4.Battery());

        // Serial.println();
        // This delay is to make the output more human readable
        // Remove it when you're not trying to see the output
    }
}