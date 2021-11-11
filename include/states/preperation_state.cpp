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
#include "flash.h"
#include "EEPROM.h"

class PreperationState: public State {
    public: 

        void extractData() {
            static int executions = 0;

            // GPS
            gps::readGps();
            sens_data::GpsData gd;
            if (gps::hasData)
            {
                gd = gps::getGpsState();
                s_data.setGpsData(gd);
            }
            // MAGNETOMETER
            magnetometer::readMagnetometer();
            sens_data::MagenetometerData md = magnetometer::getMagnetometerState();
            s_data.setMagnetometerData(md);
            // BAROMETER
            sens_data::BarometerData bd = barometer::getBarometerState();
            s_data.setBarometerData(bd);

            if(executions % 700 == 0)
            {
                //Print necessary info during preperation state
                Serial.println("Lopy Battery Voltage: " + String(arming::getLopyBatteryVoltage()) + "V Parachute Battery 1 Voltage: " + String(arming::getBattery1Voltage()) + "V  Parachute Battery 2 Voltage: " + String(arming::getBattery2Voltage()) + "V");
                Serial.println("GPS satellites: " + String(gd.sats));
            }
            executions++;
        }

        void start () override {
            Serial.println("PREP STATE");
            buzzer::signalStart();

            arming::setup();
            Wire.begin(12, 13);

            buzzer::setup();
            buzzer::test();
            flash::setup();
            flash::deleteFile("/test.txt"); //!if is reset mid-flight file gets deleted
            gps::setup(9600);            
            barometer::setup();
            magnetometer::setup();
            comms::setup(868E6);

            if(arming::clearEEPROM()) //checks EEPROM clear jumper
            {
                magnetometer::clearEEPROM();
            }
            magnetometer::getCorEEPROM();
            magnetometer::displayCor();

            if(magnetometer::hasBeenLaunch())
            {
                this->_context->RequestNextPhase(); //! Transition to flight state
                this->_context->Start();
            }

            if(!magnetometer::savedCorToEEPROM())
            {
                buzzer::signalCalibrationStart();
                magnetometer::calibrate();
                buzzer::signalCalibrationEnd();
            }
            else
            {
                Serial.println("Calibration skipped - EEPROM shows as calibrated");
                buzzer::signalCalibrationStart();
                buzzer::signalCalibrationEnd();
            }

            arming::secondSwitchStart = millis();
            while(!arming::armingSuccess() && !magnetometer::savedCorToEEPROM())
            {
                extractData(); //give Data to LoRa for sending and print necessary data in Serial
                if(!arming::checkFirstSwitch() && !arming::firstSwitchHasBeen)
                {
                    buzzer::buzz(1080);
                    delay(1000);
                    buzzer::buzzEnd();
                    arming::firstSwitchHasBeen = 1;
                }
                if(arming::checkSecondSwitch() && !arming::timeKeeper)
                {                                                                   
                    arming::fail = 1;                                                          
                    Serial.println("Calibration failed, affirmed too fast!"); 
                } 
                else if(arming::checkSecondSwitch() && arming::checkThirdSwitch()) //ja ir izvilkts slēdzis 
                {
                    buzzer::signalSecondSwitch();
                    if(millis() - arming::secondSwitchStart > 10000) //un ja pagājis vairāk kā noteiktais intervāls
                    {
                        arming::AlreadyCalibrated = 1;
                        magnetometer::saveCorToEEPROM();
                        magnetometer::setAsCalibrated();
                        Serial.println("EEPROM calibration values saved");
                        buzzer::signalSavedValues();
                    } 
                }
                else
                {
                    arming::secondSwitchStart = millis(); //resetto izvilkšanas sākuma laiku uz pašreizējo laiku
                }
            }
            magnetometer::getCorEEPROM();
            magnetometer::displayCor();

            //permanent loop while not pulled third switch
            while(!arming::checkSecondSwitch() || arming::checkThirdSwitch()) {extractData();}
            this->_context->RequestNextPhase();
            this->_context->Start();
           
        }

        void HandleNextPhase() override {
            Serial.println("proof of concept --- NEXT STATE for PREP");
            this->_context->TransitionTo(new FlightState);
        }
};