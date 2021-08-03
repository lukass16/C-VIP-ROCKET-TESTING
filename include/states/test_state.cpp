#pragma once

#include "Arduino.h"
#include "magnetometer_wrapper.h"
#include "core/core.cpp"

class TestState: public State {
    public: 
        void start () override {
            magnetometer::setup();
            magnetometer::calibrate();
            magnetometer::testCalibratedAxis();            
        }

        void HandleNextPhase() override {
            Serial.println("proof of concept --- NEXT STATE for TEST");
        }
};
