#pragma once

#include "Arduino.h"
#include "core/core.cpp"
#include "descent_state.cpp"
#include "lora_wrapper.h"
#include "gps_wrapper.h"
#include "magnetometer_wrapper.h"
#include "sensor_data.h"
#include "flash.h"
#include "arming.h"


class FlightState : public State
{

private:
    volatile bool timerDetAp = 0;
    bool gpsDetAp = 0;
    bool timerEnabled = 0;

    int count = 0;
    boolean isApogee()
    {
        if (magnetometer::isApogee())
        {
            count++; // [old] for testing purposes commented out
            if (count>100)
            {
                Serial.println("Magnetometer detected apogee!");
                return true;
            }
        }
        else if (magnetometer::timerDetectApogee())
        {
            return true;
            Serial.println("Timer detected apogee");
        }
        else if (gpsDetAp)   //*added gpsDetAp functionality
        {
            return false; //! for testing purposes changed from true to false
            Serial.println("GPS detected apogee");
        }

        return false;
    }

public:
    float GPSTimeElapsed = 0;

    void start() override
    {
        Serial.println("proof of concept --- FLIGHT STATE");

        //*test
        unsigned long start_time = millis();

        //File file = flash::openFile(); //opening file for writing during flight

        //!There is danger that if launch was detected previously the rocket goes straight to arming and setting apogee timers 
        //!so if launch is detected and during testing the launchDetected EEPROM value is not changed it could lead to an inadvertent triggering of the mechanism
        /* commented out for different tests - safety mechanism for apogee detection mid-flight
        if(magnetometer::hasBeenLaunch())
        {
            GPSTimeElapsed = gps::getGPSTimeElapsed(gps::getMinute(), gps::getSecond());
            if(GPSTimeElapsed > 14)
            {
                gpsDetAp = 1;
            }
            else
            {
                magnetometer::startApogeeTimer((14 - GPSTimeElapsed) * 1000000);
                timerEnabled = 1;
            }
        }
        */
        
        //while (millis()-start_time < 30000)
        while (1/*!isApogee() && !magnetometer::timerDetectApogee()*/) //!should change to just apogee - Commented out for Radio test
        {
            //Serial.println("Got in loop");
            buzzer::signalThirdSwitch();
            //While apogee isn't reached and the timer isn't yet enabled the rocket checks for launch to enable the timer - the checking of launch has no other functionality
           /* !Commented out for RS test            
            if (!timerEnabled)
            {
                if (magnetometer::launch())  //checks if rocket has been launched
                {
                    magnetometer::startApogeeTimer(14000000); //code to start timer - prints TIMER ENABLED if timer enabled
                    Serial.println("Launch detected!");
                    timerEnabled = 1;
                }
            }
            */ //!Commented out for RS test

            //!Note - the way the code works currently is that even when the rocket is not launched data is being saved to the flash with a speed as if the rocket was mid flight

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
            //barometer::readSensor(); // This is only to display data
            sens_data::BarometerData bd = barometer::getBarometerState();
            s_data.setBarometerData(bd);
            //Serial.println("Looping in flight state!");

            //TODO Flash
            //flash::writeData(file, gd, md, bd);

            // delay(1000);
        }

        Serial.println("APOGEE DETECTED !!!");
        //*NEW
        arming::nihromActivate();
    
        // flash::closeFile(file);
        // delay(10000);
        // Serial.println("[Test] Reading file");
        // flash::readFlash("/test.txt");
        // delay(100000);
        
        this->_context->RequestNextPhase();
        this->_context->Start();
    }

    void HandleNextPhase() override
    {
        Serial.println("proof of concept --- NEXT STATE for Descent");
        this->_context->TransitionTo(new DescentState());
    }
};
