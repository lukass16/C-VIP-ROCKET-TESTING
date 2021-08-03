#pragma once

#include "Arduino.h"

namespace buzzer {

    //declaring a variable for the piezo pin
    int piezo_pin = 21;

    //declaring variables for PWM channel attributes
    int freq = 400, channel = 0, resolution = 8;

    void setup() {
        ledcSetup(channel, freq, resolution); //setting up the PWM channel
        ledcAttachPin(piezo_pin, channel); //attaching the piezo_pin to the PWM channel

        Serial.println("Buzzer ready!");
    }

    void buzz(u_int soundLevel, uint freq = freq) {
        ledcWriteTone(channel, freq);
        ledcWrite(channel, soundLevel);
    }

    void setPiezoPin(u_int piezo_pin) {
        buzzer::piezo_pin = piezo_pin;
    }

    void test() {
        buzzer::buzz(125, 200);
        delay(500);
        buzzer::buzz(125, 500);
        delay(500);
        buzzer::buzz(125, 800);
        delay(500);
        buzzer::buzz(0, 400);
    }
}