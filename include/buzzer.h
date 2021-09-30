#pragma once

#include "Arduino.h"
#define RESONATING_FREQ 3400

namespace buzzer {

    //declaring a variable for the piezo pin
    int piezo_pin = 21;

    //variable for the structure of the third switch beeping
    bool thirdSwitchBeep = false;
    unsigned long previousTime = 0, currentTime = 0;

    //declaring variables for PWM channel attributes
    int freq = 400, channel = 0, resolution = 8;

    void setup() {
        ledcSetup(channel, freq, resolution); //setting up the PWM channel, with a resolution of 8 bits
        ledcAttachPin(piezo_pin, channel); //attaching the piezo_pin to the PWM channel

        Serial.println("Buzzer ready!");
    }

    void buzz(uint freq = freq) {
        ledcWriteTone(channel, freq);
        ledcWrite(channel, 127);
    }

    void buzzCustom(u_int soundLevel, uint freq = freq) {
        ledcWriteTone(channel, freq);
        ledcWrite(channel, soundLevel);
    }

    void buzzEnd()
    {
        buzzer::buzzCustom(0, 0);
    }

    void setPiezoPin(u_int piezo_pin) {
        buzzer::piezo_pin = piezo_pin;
    }

    void test() {
        buzzer::buzzCustom(127, 200);
        delay(500);
        buzzer::buzzCustom(127, 500);
        delay(500);
        buzzer::buzzCustom(127, 800);
        delay(500);
        buzzer::buzzEnd();
    }


    void signalStart()
    {
        buzzer::buzz(450);
        delay(250);
        buzzer::buzz(680);
        delay(250);
        buzzer::buzz(860);
        delay(500);
        buzzer::buzzEnd();
    }

    void signalCalibrationStart()
    {
        buzzer::buzz(900);
        delay(300);
        buzzer::buzz(1080);
        delay(300);
        buzzer::buzzEnd();
    }


    void signalCalibrationEnd()
    {
        buzzer::buzz(1080);
        delay(300);
        buzzer::buzz(900);
        delay(300);
        buzzer::buzzEnd();
    }

    void signalSecondSwitch()
    {
        buzzer::buzz(1080);
        delay(100);
        buzzer::buzzEnd();
        delay(50);
        buzzer::buzz(1080);
        delay(100);
        buzzer::buzzEnd();
    }

    void signalThirdSwitch()
    {
        int interval = 500; //interval time in milliseconds
        currentTime = millis();
        if (currentTime - previousTime >= interval)
        {
            previousTime = currentTime; //save the last time that buzzer was toggled
            if(!thirdSwitchBeep)  //if not buzzing
            {
                buzzer::buzz(3400);
                thirdSwitchBeep = true;
            }
            else
            {
                buzzer::buzzEnd();
                thirdSwitchBeep = false;
            }
        }
    }

    void signalDescent() //identical to previous function
    {
        int interval = 1000; //interval time in milliseconds
        currentTime = millis();
        if (currentTime - previousTime >= interval)
        {
            previousTime = currentTime; //save the last time that buzzer was toggled
            if(!thirdSwitchBeep)  //if not buzzing
            {
                buzzer::buzz(3400);
                thirdSwitchBeep = true;
            }
            else
            {
                buzzer::buzzEnd();
                thirdSwitchBeep = false;
            }
        }
    }

     void signalSavedValues()
    {
        buzzer::buzz(480);
        delay(500);
        buzzer::buzz(860);
        delay(500);
        buzzer::buzz(1080);
        delay(500);
        buzzer::buzzEnd();
    }



}