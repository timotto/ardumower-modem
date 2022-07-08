#include "ps4_controller.h"
#include "esp_gap_bt_api.h"

using namespace ArduMower::Modem::PS4controller;

Adapter::Adapter(ArduMower::Modem::Settings::Settings &_settings, ArduMower::Domain::Robot::CommandExecutor &_cmd): 
    settings(_settings), cmd(_cmd)  
{
}

void Adapter::begin()
{
    if (!settings.ps4controller.enabled) return;

    if (ps4 == NULL)
        ps4 = new PS4Controller();

    if (settings.ps4controller.use_ps4_mac) {
        Serial.print("ps4_mac: ");
        Serial.println(settings.ps4controller.ps4_mac);
        ps4->begin(settings.ps4controller.ps4_mac.c_str());
    } else {
        ps4->begin();
    }
}

void Adapter::loop()
{
    if (!settings.ps4controller.enabled) 
        return;

    if (waitForDisconnect != 0) {
        if (millis() < waitForDisconnect)
            return;

        waitForDisconnect = 0;
        begin();
    }

    if (ps4 == NULL)
        return;

    if (ps4->isConnected()) {
        // if (ps4->Right()) Serial.println("Right Button");
        // if (ps4->Down()) Serial.println("Down Button");
        // if (ps4->Up()) Serial.println("Up Button");
        // if (ps4->Left()) Serial.println("Left Button");

        // if (ps4->Square()) Serial.println("Square Button");
        // if (ps4->Cross()) Serial.println("Cross Button");
        // if (ps4->Circle()) Serial.println("Circle Button");
        // if (ps4->Triangle()) Serial.println("Triangle Button");

        // if (ps4->UpRight()) Serial.println("Up Right");
        // if (ps4->DownRight()) Serial.println("Down Right");
        // if (ps4->UpLeft()) Serial.println("Up Left");
        // if (ps4->DownLeft()) Serial.println("Down Left");

        // if (ps4->L1()) Serial.println("L1 Button");
        // if (ps4->R1()) Serial.println("R1 Button");

        // if (ps4->Share()) Serial.println("Share Button");
        // if (ps4->Options()) Serial.println("Options Button");
        // if (ps4->L3()) Serial.println("L3 Button");
        // if (ps4->R3()) Serial.println("R3 Button");

        if (ps4->PSButton())  {
            Serial.println("disconnect PS4 controller");
            waitForDisconnect = millis() + PS4_DISCONNECT_TIMEOUT;
            delete ps4;
            ps4 = NULL;
            return;
        }
        // if (ps4->Touchpad()) Serial.println("Touch Pad Button");

        // if (ps4->L2()) {
        // Serial.printf("L2 button at %d\n", ps4->L2Value());
        // }
        // if (ps4->R2()) {
        // Serial.printf("R2 button at %d\n", ps4->R2Value());
        // }

        float liniar = 0;
        float angular = 0;
        if ((ps4->Left() || ps4->Right() || ps4->Up() || ps4->Down() || ps4->UpLeft() || ps4->UpRight() || ps4->DownLeft() || ps4->DownRight()) && (ps4->R2() > 0)) {
            liniar = (ps4->Up() || ps4->UpLeft() || ps4->UpRight() ? 1 : (ps4->Down() || ps4->DownLeft() || ps4->DownRight() ? -1 : 0)) * ps4->R2Value() * 0.3 / 255;
            angular = (ps4->Right() || ps4->UpRight() || ps4->DownRight() ? -1 : (ps4->Left() || ps4->UpLeft() || ps4->DownLeft() ? 1 : 0)) * ps4->R2Value() * 0.5 / 255;
            
        } else if ((abs(ps4->RStickX()) > 10) || (abs(ps4->RStickY()) > 10)) {
            // Serial.printf("Right Stick x at %d\n", ps4->RStickX());
            // Serial.printf("Right Stick y at %d\n", ps4->RStickY());
            liniar = ps4->RStickY() * 0.1 / 128;
            angular = -ps4->RStickX() * 0.15 / 128;
            
        } else if ((abs(ps4->LStickX()) > 10) || (abs(ps4->LStickY()) > 10)) {
            // Serial.printf("Left Stick x at %d\n", ps4->LStickX());
            // Serial.printf("Left Stick y at %d\n", ps4->LStickY());
            liniar = ps4->LStickY() * 0.33 / 128;
            angular = -ps4->LStickX() * 0.5 / 128;
        }
        
        if ((liniar != 0) || (angular !=0) || (oldLiniar != 0) || (oldAngular != 0)) {
            if ((millis() - lastSendTime < PS4_SEND_INTERVAL) && ((liniar != 0) || (angular !=0))) 
                return;

            cmd.manualDrive(liniar, angular);
            oldLiniar = liniar;
            oldAngular = angular;
            lastSendTime = millis();
        }   

        // if (ps4->Charging()) Serial.println("The controller is charging");
        // if (ps4->Audio()) Serial.println("The controller has headphones attached");
        // if (ps4->Mic()) Serial.println("The controller has a mic attached");

        // Serial.printf("Battery Level : %d\n", ps4->Battery());

        // Serial.println();
        // This delay is to make the output more human readable
        // Remove it when you're not trying to see the output
    }
}