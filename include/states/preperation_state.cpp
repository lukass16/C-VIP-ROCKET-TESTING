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

            buzzer::signalStart();

            arming::setup();
            Wire.begin(12, 13);

            buzzer::setup();
            buzzer::test();
            gps::setup(9600);            
            barometer::setup();
            magnetometer::setup();
            flash::setup();
            flash::deleteFile("/test.txt");
            comms::setup(868E6);

            //magnetometer::clearEEPROM();

            if(magnetometer::hasBeenLaunch())
            {
                this->_context->RequestNextPhase(); //! Transition to flight state
                this->_context->Start();
            }

            //*Testing

            //!šis sākotnēji šādi neizskatījās speciāli izmainīju, lai var ielikt buzzer funkciju, patiesībā bija šādi: magnetometer::calibrate(magnetometer::savedCorToEEPROM());
            if(!magnetometer::savedCorToEEPROM())
            {
                buzzer::signalCalibrationStart();
                magnetometer::calibrate(0);
                buzzer::signalCalibrationEnd();
            }
            else
            {
                Serial.println("Magnetometer has already been calibrated - skipping calibration process");
                buzzer::signalCalibrationStart();
                buzzer::signalCalibrationEnd();
            }
            

            //! Checks second switch with safety against fast pull
            while(!arming::armingSuccess())
            {
                // if(arming::checkSecondSwitch() && arming::timeKeeper && arming::fail == 0)
                // {
                //     magnetometer::saveCorToEEPROM();
                //     magnetometer::setAsCalibrated(); //!should include in main code
                //     arming::AlreadyCalibrated = 1;  
                // }
                // else 
                if(arming::checkSecondSwitch() && !arming::timeKeeper)
                {                                                                   
                    arming::fail = 1;                                                          
                    Serial.println("CALIBRATION FAILED, AFFIRMED TOO FAST"); 
                } 
                else if(arming::checkSecondSwitch()) //ja ir izvilkts slēdzis 
                {
                    buzzer::signalSecondSwitch();
                    if((arming::secondSwitchStart - millis()) > 10000) //un ja pagājis vairāk kā noteiktais intervāls
                    {
                        arming::AlreadyCalibrated = 1;
                        magnetometer::saveCorToEEPROM();
                        magnetometer::setAsCalibrated();
                    } 
                }
                else
                {
                    arming::secondSwitchStart = millis(); //resetto izvilkšanas sākuma laiku uz pašreizējo laiku
                }
            }
            magnetometer::getCorEEPROM();
            magnetometer::displayCor(); //!should include in main code

            //* permanent loop while not successfull arming or not pulled third switch
            while(!arming::checkSecondSwitch() || !arming::checkThirdSwitch()) {}
            this->_context->RequestNextPhase();
            this->_context->Start();
           
        }

        void HandleNextPhase() override {
            Serial.println("proof of concept --- NEXT STATE for PREP");
            this->_context->TransitionTo(new FlightState);
        }
};