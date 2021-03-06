#pragma once
#include <Adafruit_BMP280.h>
#include "sensor_data.h"

Adafruit_BMP280 bmp; // I2C Interface

namespace barometer {
    const float AVIATION_PRESSURE = 1013.5;
    float sea_level_read = 0;
    //defining necessary variables for vertical velocity calculation
    float h_now = 0, h_prev = 0, t_now = 0, t_prev = 0, dh = 0, dt = 0, vert_velocity = 0, vert_velocity_prev = 0;

    float getVertVelocity();

    void setup(bool read_sea_level = 0) 
    {
        if (!bmp.begin(0x76)) {
            Serial.println("Could not find a valid BMP280 sensor, check wiring!");
            while (1);
        }

        /* Default settings from datasheet. */
        bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
        if (read_sea_level) {
            for(int i=0; i<5; i++){
                sea_level_read+=(bmp.readPressure()/100);
            }
            sea_level_read = sea_level_read / 5;
        }
        else
        {
            sea_level_read = AVIATION_PRESSURE;
        }
        Serial.println("Barometer ready! Sea level pressure: " + String(sea_level_read));
    }

    sens_data::BarometerData getBarometerState(){
        sens_data::BarometerData bd;
        bd.temperature = bmp.readTemperature();
        bd.pressure = bmp.readPressure() / 100;
        bd.altitude = bmp.readAltitude(sea_level_read);
        bd.vert_velocity = getVertVelocity();
        return bd;
    }

    float getVertVelocity()
    {
        static int counter = 0;
        counter++;
        if(counter % 3 == 0) //every 3rd call
        {
            h_now = bmp.readAltitude(sea_level_read); //in m
            t_now = millis() / 1000.0; //in ms
            dt = t_now - t_prev;
            dh = h_now - h_prev;
            vert_velocity = dh/dt;
            //reverting values
            h_prev = h_now;
            t_prev = t_now;
            vert_velocity_prev = vert_velocity;
            return vert_velocity;
        }
        else {return vert_velocity_prev;}
    }

    void readSensor() {
        // Serial.print("Temperature = ");
        // Serial.print(bmp.readTemperature());
        // Serial.println(" *C");

        // Serial.print("Pressure = ");
        // Serial.print(bmp.readPressure()/100); //displaying the Pressure in hPa, you can change the unit
        // Serial.println(" hPa");

        Serial.print("Approx altitude = ");
        Serial.print(bmp.readAltitude(sea_level_read));
        Serial.println(" m");
        Serial.println();
    }

}