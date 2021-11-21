#pragma once
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // I2C Interface

namespace barometer {
    const float SEA_LEVEL = 1019.66;
    float sea_level_read = 0;

    unsigned long lt_now = 0, lt_prev = millis(), nowTime = 0, prevTime = 0; //for some reason it VSCode doesn't like declaring this variable as time (I chose t)
    unsigned long prev_time;
    float prev_height, height, vert_velocity, vert_velocity_prev = 0;

    double dt = 0, dh = 0, h_now = 0, h_prev= 0, nowVelocity = 0;
    float t_now = 0, t_prev = 0;

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
            sea_level_read = SEA_LEVEL;
        }
        Serial.println("Barometer ready! Sea level pressure: " + String(sea_level_read));
    }

    float getArtificialHeight(float interval)
    {
        static int counter = 0;
        counter++;
        return counter*interval;
    }

    float getArtificialVertVelocity()
    {
        
        static int counter = 0;
        counter++;
        h_now = getArtificialHeight(0.07); //in m (real: //h_now = bmp.readAltitude(1019.66);) //maybe for real code this can be put inside if statement
        t_now = millis() / 1000.0; //in ms
        //t_now = lt_now / 1000.0; //in s
        if(counter % 1 == 0) //every 7th call
        {
            dt = t_now - t_prev;
            Serial.println("Time: " + String(dt, 5));
            // Serial.println("T_now: " + String(t_now, 5));
            // Serial.println("T_prev: " + String(t_prev, 5));
            dh = h_now - h_prev;
            vert_velocity = dh/dt;
            //reverting values
            h_prev = h_now;
            t_prev = t_now;
            vert_velocity_prev = vert_velocity;
            delay(307);
            return vert_velocity;
        }
        else {delay(300);return vert_velocity_prev;}
    }

    float getVertVelocity()
    {
        static int counter = 0;
        counter++;
        h_now = bmp.readAltitude(sea_level_read); //in m
        t_now = millis() / 1000.0; //in ms
        if(counter % 3 == 0) //every 3rd call
        {
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

    void printVelocity()
    {
        nowTime = millis();
        nowVelocity = getVertVelocity();
        if(prevTime + 1000 < nowTime)
        {
            Serial.println("Height: " + String(bmp.readAltitude(sea_level_read)));
            Serial.println("Velocity: " + String(nowVelocity) + "\n");
            prevTime = nowTime;
        }
        
    }

    void readSensor() {
        // Serial.print("Temperature = ");
        // Serial.print(bmp.readTemperature());
        // Serial.println(" *C");

        // Serial.print("Pressure = ");
        // Serial.print(bmp.readPressure()/100); //displaying the Pressure in hPa, you can change the unit
        // Serial.println(" hPa");

        Serial.print("Approx altitude = ");
        Serial.print(bmp.readAltitude(sea_level_read)); //The "1019.66" is the pressure(hPa) at sea level in my place
        Serial.println(" m");
        Serial.println();
    }

}