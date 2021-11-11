#pragma once

#include "Arduino.h"
#include "core/core.cpp"
#include "descent_state.cpp"
#include "lora_wrapper.h"
#include "gps_wrapper.h"
#include "magnetometer_wrapper.h"
#include "sensor_data.h"
#include "arming.h"


class FlightState : public State {
    private:
        volatile bool timerDetAp = 0;
        bool timerEnabled = 0;

        int magcount = 0;
        boolean isApogee()
        {
            if (magnetometer::isApogee())
            {
                magcount++;
                if (magcount>100)
                {
                    Serial.println("Magnetometer detected apogee");
                    return true;
                }
            }
            else if (magnetometer::timerDetectApogee())
            {
                return true;
                Serial.println("Timer detected apogee");
            }

            return false;
        }

    public:
        void start() override
        {
            Serial.println("FLIGHT STATE");

            File file = flash::openFile(); //opening file for writing during flight
            bool start_writing = 0;
            if(magnetometer::hasBeenLaunch())
            {
                start_writing = 1;
            }
            
            //!There is danger that if launch was detected previously the rocket goes
            //!straight to arming and setting apogee timers, so if launch is detected and during testing 
            //!the launchDetected EEPROM value is not changed it could lead to an inadvertent triggering of the mechanism - should test whether if arming pin is inserted this can happen

            while (!isApogee())
            {
                buzzer::signalThirdSwitch();
                //While apogee isn't reached and the timer isn't yet enabled the rocket checks for launch to enable the timer - the checking of launch has no other functionality
                if (!timerEnabled)
                {
                    if (magnetometer::launch())  //checks if rocket has been launched
                    {
                        magnetometer::startApogeeTimer(14000000); //code to start timer - prints TIMER ENABLED if timer enabled
                        buzzer::buzz(4000);
                        Serial.println("Launch detected!");
                        timerEnabled = 1;
                        start_writing = 1;
                    }
                }

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

                // BATTERIES
                sens_data::BatteryData btd = arming::getBatteryState();
                s_data.setBatteryData(btd);

                if(start_writing)
                {
                    flash::writeData(file, gd, md, bd, btd); //writing data to flash memory
                }
                
            }
            Serial.println("APOGEE DETECTED !!!");
            arming::nihromActivate();

            flash::closeFile(file); //closing flash file

            this->_context->RequestNextPhase();
            this->_context->Start();
        }

        void HandleNextPhase() override
        {
            this->_context->TransitionTo(new DescentState());
        }
};
