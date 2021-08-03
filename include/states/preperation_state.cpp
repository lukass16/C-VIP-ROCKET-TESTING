#pragma once

#include "Arduino.h"
#include "communication.h"
#include "barometer_wrapper.h"
#include "magnetometer_wrapper.h"
#include "buzzer.h"
#include "core/core.cpp"
#include "flight_state.cpp"
#include "test_state.cpp"
#include "arming.h"
#include "EEPROM.h" //*NEW
#include "flash.h"

class PreperationState: public State {
    public: 
        void start () override {
            Serial.println("proof of concept --- PREP STATE");

            arming::setup();
            Wire.begin(12, 13);

            if(magnetometer::hasBeenLaunch())
            {
                this->_context->RequestNextPhase(); //! Transition to flight state
                this->_context->Start();
            }

            buzzer::setup();
            buzzer::test();
            gps::setup(9600);            
            barometer::setup();
            magnetometer::setup();
            flash::setup();
            flash::deleteFile("/test.txt");
            comms::setup(868E6);

            //*Testing
            //magnetometer::calibrate(magnetometer::savedCorToEEPROM());

            // //! Checks second switch with safety against fast pull
            // while(!arming::armingSuccess())
            // {
            //     if(arming::checkSecondSwitch() && arming::timeKeeper && arming::fail == 0)
            //     {
            //         magnetometer::saveCorToEEPROM();
            //         arming::AlreadyCalibrated = 1;  
            //     }
            //     else if (arming::checkSecondSwitch() && !arming::timeKeeper)
            //     {                                                                   
            //         arming::fail = 1;                                                          
            //         Serial.println("CALIBRATION FAILED, AFFIRMED TOO FAST"); 
            //     }   
            // }
            magnetometer::getCorEEPROM();

      
            // //* permanent loop while not successfull arming or not pulled third switch
            // while(!arming::armingSuccess() || !arming::checkThirdSwitch())
            // {
            //     if(arming::armingSuccess() && arming::checkThirdSwitch())
            //     {
            //         this->_context->RequestNextPhase();
            //         this->_context->Start();
            //     }
            // }
            this->_context->RequestNextPhase();
            this->_context->Start();
           
        }

        void HandleNextPhase() override {
            Serial.println("proof of concept --- NEXT STATE for PREP");
            this->_context->TransitionTo(new FlightState);
        }
};