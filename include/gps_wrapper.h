#pragma once
#include <TinyGPS++.h>
#include "sensor_data.h"
#include "EEPROM.h"


namespace gps {

    TinyGPSPlus gps;
    boolean hasData = false;

    sens_data::GpsData lastData;  //Last data so that values of zero don't get sent when gps doesn't have lock on

    void setup(uint gpsRate = 9600)
    {
        Serial.println("Init GPS: " + String(gpsRate));
    }

    void readGps()
    {
        hasData = false;
        while (Serial.available())
        {
            gps.encode(Serial.read());
            hasData = true;
        }
    }

    double lastLatitude() {
        return gps.location.lat(); 
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

    double getHdop()
    {
        return gps.hdop.hdop();
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
        Serial.print("Second: ");
        Serial.println(second);
        EEPROM.writeFloat(28, second);
        EEPROM.commit();
    }

    void writeMinuteEEPROM()
    {
        Serial.println("Writing Minute to EEPROM");
        float minute = (float) getMinute();
        Serial.print("Minute: ");
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
            gd.sats = getSatellites();
            return gd;
        }
        else
        {
            lastData.sats = getSatellites();
            return lastData;
        } 
    }
    
}