#pragma once

#include "Arduino.h"
#include "communication.h"
#include "barometer_wrapper.h"
#include "magnetometer_wrapper.h"
#include "buzzer.h"
#include "core/core.cpp"
#include "flight_state.cpp"
#include "arming.h"
#include "flash.h"
#include "EEPROM.h"

class PreperationState: public State {
    public: 
        sens_data::GpsData gd;
        void extractData() {
            static int executions = 0;
            // GPS
            gps::readGps();
            if (gps::hasData)
            {
                gd = gps::getGpsState();
                s_data.setGpsData(gd);
            }
            // MAGNETOMETER
            magnetometer::readMagnetometer();
            sens_data::MagenetometerData md = magnetometer::getMagnetometerState();
            s_data.setMagnetometerData(md);
            magnetometer::processApogee();
            // BAROMETER
            sens_data::BarometerData bd = barometer::getBarometerState();
            s_data.setBarometerData(bd);

            if(executions % 700 == 0) //happens every 700th loop - corresponds to about once per every 2 seconds
            {
                //Print necessary info during preperation state
                Serial.print("FCB: " + String(arming::getLopyBatteryVoltage()) + "V\tPB 1: " + String(arming::getBattery1Voltage()) + "V\tPB 2: " + String(arming::getBattery2Voltage()) + "V");
                Serial.println("\tGPS sats: " + String(gd.sats));
            }
            executions++;
        }

        void start () override {
            Serial.println("PREP STATE");

            buzzer::setup();
            buzzer::signalStart();
            arming::setup();
            Wire.begin(12, 13);
            flash::setup();
            barometer::setup();
            buzzer::buzzEnd(); //end start signal
            magnetometer::setup();
            comms::setup(868E6);

            if(arming::clearEEPROM()) //checks EEPROM clear jumper
            {
                magnetometer::clearEEPROM();
                flash::unlock();
            }
            if(!flash::locked()){flash::deleteFile("/test.txt");} //if file is not locked - clear it
            magnetometer::getCorEEPROM();

            if(magnetometer::hasBeenLaunch())
            {
                magnetometer::arm();
                this->_context->RequestNextPhase(); //Transition to flight state
                this->_context->Start();
            }
            
            //Check whether the magnetometer has been calibrated, if not - calibrate it
            if(!magnetometer::savedCorToEEPROM())
            {
                buzzer::signalCalibrationStart();
                magnetometer::calibrate();
                buzzer::signalCalibrationEnd();
            }
            else {buzzer::signalCalibrationSkip();}

            arming::secondSwitchStart = millis();
            while(!arming::armingSuccess() && !magnetometer::savedCorToEEPROM())
            {
                extractData(); //report back state of flight computer
                arming::reportFirstSwitch(); //signal if first switch has been pulled
                if(arming::checkSecondSwitch() && arming::checkThirdSwitch()) //ja ir izvilkts slēdzis 
                {
                    buzzer::signalSecondSwitch();
                    if(millis() - arming::secondSwitchStart > 10000) //if 10 seconds have passed
                    {
                        arming::AlreadyCalibrated = 1;
                        magnetometer::saveCorToEEPROM();
                        magnetometer::setAsCalibrated();
                        buzzer::signalSavedValues();
                        Serial.println("EEPROM calibration values saved");
                    } 
                }
                else
                {
                    arming::secondSwitchStart = millis(); //resets pulled start time to current time
                }
            }
            magnetometer::getCorEEPROM();
            magnetometer::displayCor();

            //permanent loop while not pulled third switch and second switch
            while(!arming::checkSecondSwitch() || arming::checkThirdSwitch()) {extractData();} //changed init
            this->_context->RequestNextPhase();
            this->_context->Start();

        }

        void HandleNextPhase() override {
            this->_context->TransitionTo(new FlightState);
        }
};