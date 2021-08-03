#pragma once
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "sensor_data.h"
#include "EEPROM.h"


namespace gps {

    #define RXPIN 15 //P4 on lopy is set to RX pin (important to remember about crossover connection)
    #define TXPIN 4  //P3
    SoftwareSerial gpsSerial;
    SoftwareSerialConfig ssc = SWSERIAL_8N1; // 8bits-no_parity-1_stop_bit  https://github.com/plerup/espsoftwareserial/
    TinyGPSPlus gps;

    boolean hasData = false;

    sens_data::GpsData lastData;  //Last data so that values of zero don't get sent when gps doesn't have lock on


    void setup(uint gpsRate = 9600)
    {
        gpsSerial.begin(gpsRate, ssc, RXPIN, TXPIN);
        Serial.println("Init GPS: " + String(gpsRate));
    }

    void readGps()
    {
        hasData = false;

        while (gpsSerial.available())
        {
            gps.encode(gpsSerial.read());
            hasData = true;
        }

        // Serial.print("LAT=");
        // Serial.println(gps.location.lat(), 6);
        // Serial.print("LONG=");
        // Serial.println(gps.location.lng(), 6);
        // Serial.print("ALT=");
        // Serial.println(gps.altitude.meters());
    }

    double lastLatitude() {
        return gps.location.lat();  //TODO ar 6 skaitljiem?
    }

    double lastLongitude() {
        return gps.location.lng();
    }

    double lastAltitude() {
        return gps.altitude.meters();
    }


    int getSatellites()
    {
        return gps.satellites.value();
    }

    uint8_t getHour()
    {
        return gps.time.hour();
    }

    uint8_t getMinute()
    {
        return gps.time.minute();
    }

    uint8_t getSecond()
    {
        return gps.time.second();
    }

    void writeSecondEEPROM()
    {
        Serial.println("Writing Second to EEPROM");
        float second = (float) getSecond();
        Serial.print("Second!!!!!!:     ");
        Serial.println(second);
        EEPROM.writeFloat(28, second);
        EEPROM.commit();
    }

    void writeMinuteEEPROM()
    {
        Serial.println("Writing Second to EEPROM");
        float minute = (float) getMinute();
        Serial.print("Minute!!!!!!:     ");
        Serial.println(minute);
        EEPROM.writeFloat(32, minute);
        EEPROM.commit();
    }

    int getSecondEEPROM()
    {
        return EEPROM.readFloat(28);
    }

    int getMinuteEEPROM()
    {
        return EEPROM.readFloat(32);
    }

    float getGPSTimeElapsed(int currentMinute, int currentSecond)
    {
        return ((currentSecond + (currentMinute - getMinuteEEPROM()) * 60) - getSecondEEPROM()); //TODO test
    }


    sens_data::GpsData getGpsState()
    {
        sens_data::GpsData gd;
        if(gps.location.isValid())
        {
            //adding last good values
            lastData.lat = lastLatitude();
            lastData.lng = lastLongitude();
            lastData.alt = lastAltitude();
            gd.lat = lastLatitude();
            gd.lng = lastLongitude();
            gd.alt = lastAltitude();
            return gd;
        }
        else
        {
            return lastData;
        } 
    }
    
}