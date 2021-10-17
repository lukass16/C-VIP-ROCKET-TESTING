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

            int ex_been = 0;

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

            magnetometer::clearEEPROM();
            magnetometer::getCorEEPROM();
            magnetometer::displayCor();

            if(magnetometer::hasBeenLaunch())
            {
                this->_context->RequestNextPhase(); //! Transition to flight state
                this->_context->Start();
            }

            //*Testing

            //TODO if magnetometer already calibrated still waiting to retrieve values - sets new

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
            
            arming::secondSwitchStart = millis();
            while(!arming::armingSuccess())
            {
                // if(arming::checkSecondSwitch() && arming::timeKeeper && arming::fail == 0)
                // {
                //     magnetometer::saveCorToEEPROM();
                //     magnetometer::setAsCalibrated(); //!should include in main code
                //     arming::AlreadyCalibrated = 1;  
                // }
                // else 
                if(!arming::checkFirstSwitch() && !ex_been)
                {
                    buzzer::buzz(1080);
                    delay(1000);
                    buzzer::buzzEnd();
                    ex_been = 1;
                }
                if(arming::checkSecondSwitch() && !arming::timeKeeper)
                {                                                                   
                    arming::fail = 1;                                                          
                    Serial.println("CALIBRATION FAILED, AFFIRMED TOO FAST"); 
                } 
                else if(arming::checkSecondSwitch() && arming::checkThirdSwitch()) //ja ir izvilkts slēdzis 
                {
                    buzzer::signalSecondSwitch();
                    if(millis() - arming::secondSwitchStart > 10000) //un ja pagājis vairāk kā noteiktais intervāls
                    {
                        arming::AlreadyCalibrated = 1;
                        magnetometer::saveCorToEEPROM();
                        magnetometer::setAsCalibrated();
                        Serial.println("SAGLABATAS VERTIBAS");
                        buzzer::signalSavedValues();
                    } 
                }
                else
                {
                    //Serial.println("Nav sledzis");
                    arming::secondSwitchStart = millis(); //resetto izvilkšanas sākuma laiku uz pašreizējo laiku
                }
            }
            magnetometer::getCorEEPROM();
            magnetometer::displayCor(); //!should include in main code

            //* permanent loop while not successfull arming or not pulled third switch
            while(!arming::checkSecondSwitch() || arming::checkThirdSwitch())
            {
                //nothing
            }
            this->_context->RequestNextPhase();
            this->_context->Start();
           
        }

        void HandleNextPhase() override {
            Serial.println("proof of concept --- NEXT STATE for PREP");
            this->_context->TransitionTo(new FlightState);
        }
};