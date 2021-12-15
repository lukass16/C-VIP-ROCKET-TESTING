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
        int flash_counter = 0;
        sens_data::GpsData gd;
        
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
                magnetometer::startApogeeTimer(15500000); //last ditch effort after restart - if magnetometer fails the timer will deploy
                timerEnabled = 1;
            }
            
            while (!isApogee())
            {
                buzzer::signalFlight();
                //While apogee isn't reached and the timer isn't yet enabled the rocket checks for launch to enable the timer
                if (!timerEnabled)
                {
                    if (magnetometer::launch())  //checks if rocket has been launched
                    {
                        magnetometer::startApogeeTimer(15500000); //code to start timer - time passed in microseconds (15.5s) - prints Timer Enabled if timer enabled
                        magnetometer::arm(true); //magnetometer can detect apogee
                        buzzer::buzz(4000);
                        Serial.println("Launch detected!");
                        timerEnabled = 1;
                        start_writing = 1; //starts writing to flash after launch
                        flash::lock(); //if launch has happened lock the flash - it can't be deleted after
                    }
                }

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

                // BAROMETER
                sens_data::BarometerData bd = barometer::getBarometerState();
                s_data.setBarometerData(bd);

                // BATTERIES
                sens_data::BatteryData btd = arming::getBatteryState();
                s_data.setBatteryData(btd);

                if(start_writing)
                {
                    flash_counter = flash::writeData(file, gd, md, bd, btd); //writing data to flash memory
                    if(flash_counter % 100 == 1){flash::closeFile(file);file=flash::openFile();} //close and open the file every 100th reading
                }
                
            }
            Serial.println("APOGEE DETECTED !!!");
            arming::nihromActivateFirst();
            flash::closeFile(file); //closing flash file

            this->_context->RequestNextPhase();
            this->_context->Start();
        }

        void HandleNextPhase() override
        {
            this->_context->TransitionTo(new DescentState());
        }
};
