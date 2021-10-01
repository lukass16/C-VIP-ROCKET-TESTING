#pragma once
#include <cstdio>
#include <Arduino.h>
#include <ArduinoJson.h>
#include "sensor_data.h"

#include "lora_wrapper.h"
#include "thread_wrapper.h"

using namespace std;

namespace comms {   
    String sensorDataToJson();
    void loop(void *args);

    void setup(double frequency = 868E6) {
        lora::setup(frequency);

        s_thread::setup(loop);
    }

    // This is ran in a seperate thread
    void loop(void *args = NULL) {
        // Send lora every 2 secs
        while (true)
        {
            String serializedJson = sensorDataToJson();
            //String serialized = comms::serializeData();
            lora::sendMessage(serializedJson, s_data.lora_message_id);
            Serial.print("Lora (msg id: ");
            Serial.print(s_data.lora_message_id);
            Serial.print(") sent: ");
            Serial.println(serializedJson);
            s_data.lora_message_id++;
            delay(2000);
        }
        
    }

    String serializeData(){
        char outgoing [100];
        static int counter = 0;
        sens_data::GpsData gps = s_data.getGpsData();
        sens_data::MagenetometerData mag = s_data.getMagnetometerData();
        sens_data::BarometerData bar = s_data.getBarometerData();
        sprintf(outgoing, "%7.4f,%7.4f,%5.0f,%5.2f,%5.2f,%5.2f,%3.1f,%5.1f,%6.1f,%4d", gps.lat, gps.lng, gps.alt, mag.x, mag.y, mag.z, bar.temperature, bar.pressure, bar.altitude, counter);
        counter++;
        return outgoing;
    }

    String sensorDataToJson() {
        StaticJsonDocument<256> document;

        static int counter = 0;
        // document["mode"] = g.current_rocket_state;

        // GPS
        sens_data::GpsData gps = s_data.getGpsData();
        document["lat"] = gps.lat;
        document["lng"] = gps.lng;
        document["alt"] = gps.alt;

        // MAGNETOMETER
        sens_data::MagenetometerData mag = s_data.getMagnetometerData();
        document["x"] = mag.x;
        document["y"] = mag.y;
        document["z"] = mag.z;

        // BAROMETER
        sens_data::BarometerData bar = s_data.getBarometerData();
        document["temp"] = bar.temperature;
        document["pres"] = bar.pressure;
        document["bar_alt"] = bar.altitude;
        document["vert_vel"] = bar.vert_velocity; //*NEW

        // COUNTER
        document["counter"] = counter;

        // @todo Should also send time since lauch
        
        char buffer[256]; //TODO check if with new data buffer size large enough
        
        counter++;
        serializeJson(document, buffer);
        return String(buffer);
        //gittest
    }
}