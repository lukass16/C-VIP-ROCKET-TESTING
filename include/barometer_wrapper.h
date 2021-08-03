#pragma once
#include <Adafruit_BMP280.h>
#include "sensor_data.h"

Adafruit_BMP280 bmp; // I2C Interface

namespace barometer {
    const float SEA_LEVEL = 1019.66;

    unsigned long t = millis(); //for some reason it VSCode doesn't like declaring this variable as time (I chose t)
    unsigned long prev_time;
    float prev_hight, hight, vert_velocity;

    float getVertVelocity();

    void setup() 
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

        Serial.println("Barometer ready!");
    }

    sens_data::BarometerData getBarometerState(){
        sens_data::BarometerData bd;
        bd.temperature = bmp.readTemperature();
        bd.pressure = bmp.readPressure() / 100;
        bd.altitude = bmp.readAltitude(SEA_LEVEL);
        bd.vert_velocity = getVertVelocity();  //*NEW
        return bd;
    }

    float getVertVelocity()
    {
        t = millis(); //in seconds

        hight = bmp.readAltitude(1019.66);
        vert_velocity = ((hight - prev_hight) * 1000) / (t - prev_time); //we multiply by 1000 because we divide by milliseconds
        prev_hight = hight;
        prev_time = t;

        return vert_velocity;
    }

    void readSensor() {
        Serial.print("Temperature = ");
        Serial.print(bmp.readTemperature());
        Serial.println(" *C");

        Serial.print("Pressure = ");
        Serial.print(bmp.readPressure()/100); //displaying the Pressure in hPa, you can change the unit
        Serial.println(" hPa");

        Serial.print("Approx altitude = ");
        Serial.print(bmp.readAltitude(SEA_LEVEL)); //The "1019.66" is the pressure(hPa) at sea level in my place
        Serial.println(" m");                    //If you don't know it, modify it until you get your current altitude

        Serial.println();
    }

}