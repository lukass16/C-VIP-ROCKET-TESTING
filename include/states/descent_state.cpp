#pragma once

#include "Arduino.h"
#include "core/core.cpp"
#include "flight_state.cpp"
#include "communication.h"
#include "buzzer.h"
#include "magnetometer_wrapper.h"
#include "gps_wrapper.h"
#include "barometer_wrapper.h"
#include "arming.h"
#include "flash.h"

class DescentState : public State {
    public:
        void start() override
        {
            Serial.println("DESCENT STATE");

            unsigned long start_time_descent = millis();
            int descent_write_time = 180000; //ms
            int flash_counter = 0;
            bool file_closed = 0;
            sens_data::GpsData gd;
            File file = flash::openFile();

            while (true)
            {
                //activate nihroms
                arming::nihromActivateFirst();
                arming::nihromActivateSecond();
                
                buzzer::signalDescent();

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

                if(millis() - start_time_descent < descent_write_time)
                {
                    if(flash_counter % 2 == 0){flash::writeData(file, gd, md, bd, btd);} //writing data to flash memory every second iteration
                    flash_counter++;
                    if(flash_counter % 100 == 1){flash::closeFile(file);file=flash::openFile();} //close and open the file every 100th iteration
                }
                else if(!file_closed)
                {
                    flash::closeFile(file); //closing flash file
                    Serial.println("Flash data file closed");
                    file_closed = 1;
                }  
            }
        }

        void HandleNextPhase() override
        {
            Serial.println("END of VIP ROCKET CODE");
        }
};
