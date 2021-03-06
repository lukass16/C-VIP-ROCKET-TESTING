#pragma once

#include "FS.h"
#include <LITTLEFS.h>
#include "sensor_data.h"
#include "EEPROM.h"

#define FORMAT_LITTLEFS_IF_FAILED true

unsigned long flash_time = millis();

//to simplify the usage of the Flash header declared a different function - deleteFile - this serves as it's basis
void delete_File(fs::FS &fs, const char *path)
{
    Serial.printf("Deleting file: %s\r\n", path);
    if (fs.remove(path))
    {
        Serial.println("- file deleted");
    }
    else
    {
        Serial.println("- delete failed");
    }
}

template <unsigned long size>
struct Buffer
{
    int offset = 0;
    uint8_t buf[size];

    void push(uint8_t *value)
    {
        for (int i = 0; i < sizeof(value); i++)
            buf[i + offset] = value[i];
        offset += sizeof(value);
    };

    void clean()
    {
        offset = 0;
        for (int x = 0; x < size; x++)
            buf[x] = 0;
    }
};

template <unsigned long size>
struct StreamPipe
{
    char buf_out[size];
    int offset = 0;

    template <typename T>
    void getValue(T *value_p)
    {
        auto &value = *value_p;
        char char_buf[sizeof(T)];
        for (int i = 0; i < sizeof(T); i++)
            char_buf[i] = buf_out[i + offset];
        offset += sizeof(T);

        value = *(T *)char_buf; // Cast char array to type
    }
};

namespace flash
{

    void setup()
    {
        if (!LITTLEFS.begin(FORMAT_LITTLEFS_IF_FAILED))
        {
            Serial.println("LITTLEFS Mount Failed. Its possible you need to format the partition with LITTLEFS.format() just once");
            //LITTLEFS.format();
            return;
        }
        Serial.println("SPIFFS-like write file to new path and delete it w/folders");
    }

    void deleteFile(const char *path)
    {
        delete_File(LITTLEFS, path);
    }

    File openFile(String filepath = "/test.txt")
    {
        File file = LITTLEFS.open(filepath, FILE_APPEND);
        return file;
    }

    float getTimeElapsed() //*Check overflow
    {
        return millis()-flash_time;
    }

    void testFileIO(File file, int multiplier)
    {
        float x_float = 0.0 + multiplier;
        float y_float = 17.1212332 + multiplier;
        float z_float = 99.9999 + multiplier;

        auto x = (uint8_t *)(&x_float);
        auto y = (uint8_t *)(&y_float);
        auto z = (uint8_t *)(&z_float);

        auto const buf_size = sizeof(x) + sizeof(y) + sizeof(z);
        Buffer<buf_size> buffer;

        buffer.push(x);
        buffer.push(y);
        buffer.push(z);

        if (!file)
        {
            Serial.println("- failed to open file for writing");
            return;
        }
        file.write(buffer.buf, buf_size);
    }

    //should still add a lot of writeable information: https://docs.google.com/document/d/1jWQnLnQJqiII_0ii84CKXIUmW_RAO8ebAFO_XuiE9oU/edit
    int writeData(File file, sens_data::GpsData gpsData, sens_data::MagenetometerData magData, sens_data::BarometerData barData, sens_data::BatteryData batData)
    {
        //counter for closing and opening file
        static int counter;

        //Flash timing
        float _time = flash::getTimeElapsed();
        auto time = (uint8_t *)(&_time);

        //GPS
        float _lat = gpsData.lat; //1.1
        float _lng = gpsData.lng; //1.2
        float _alt = gpsData.alt; //1.3
        float _sats = gpsData.sats; //1.4

        auto lat = (uint8_t *)(&_lat);
        auto lng = (uint8_t *)(&_lng);
        auto alt = (uint8_t *)(&_alt);
        auto sats = (uint8_t *)(&_sats); //1.4

        //Bar
        auto pressure = (uint8_t *)(&barData.pressure); //2.1
        auto altitude = (uint8_t *)(&barData.altitude); //2.2
        auto vert_velocity = (uint8_t *)(&barData.vert_velocity); //2.3
        auto temperature = (uint8_t *)(&barData.temperature); //2.4
       
        //Bat
        auto bat1 = (uint8_t *)(&batData.bat1); //3.1
        auto bat2 = (uint8_t *)(&batData.bat2); //3.2

        //Mag
        auto x = (uint8_t *)(&magData.x); //4.1
        auto y = (uint8_t *)(&magData.y); //4.2
        auto z = (uint8_t *)(&magData.z); //4.3
        auto acc_x = (uint8_t *)(&magData.acc_x); //4.4
        auto acc_y = (uint8_t *)(&magData.acc_y); //4.5
        auto acc_z = (uint8_t *)(&magData.acc_z); //4.6

        auto const buf_size = sizeof(time) + sizeof(lat) + sizeof(lng) + sizeof(alt) + sizeof(sats) + sizeof(pressure) + sizeof(altitude) + sizeof(vert_velocity) + sizeof(temperature) + sizeof(bat1) + sizeof(bat2) + sizeof(x) + sizeof(y) + sizeof(z) + sizeof(acc_x) + sizeof(acc_y) + sizeof(acc_z);
        Buffer<buf_size> buffer;

        buffer.push(time);

        buffer.push(lat);
        buffer.push(lng);
        buffer.push(alt);
        buffer.push(sats);

        buffer.push(pressure);
        buffer.push(altitude);
        buffer.push(vert_velocity);
        buffer.push(temperature);
        
        buffer.push(bat1);
        buffer.push(bat2);
        
        buffer.push(x);
        buffer.push(y);
        buffer.push(z);
        buffer.push(acc_x);
        buffer.push(acc_y);
        buffer.push(acc_z);

        if (!file)
        {
            Serial.println("- failed to open file for writing");
            return 0;
        }
        file.write(buffer.buf, buf_size);

        counter++;
        return counter;
    }


    void readFlashVerbose(const char *path)
    {
        File file = LITTLEFS.open(path);
        //This is the size of reading
        auto const buf_size = sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float);   
        while (file.available())
        {
            ;

            StreamPipe<buf_size> stream;
            file.readBytes(stream.buf_out, buf_size);
 

            //Time
            float time = 0;
            stream.getValue<float>(&time);
            Serial.println("ellapsed time: " + String(time, 10));

            //GPS
            float lat = 0;
            stream.getValue<float>(&lat);
            Serial.println("lat: " + String(lat, 10));

            float lng = 0;
            stream.getValue<float>(&lng);
            Serial.println("lng: " + String(lng, 10));

            float alt = 0;
            stream.getValue<float>(&alt);
            Serial.println("alt: " + String(alt, 10));

            float sats = 0;
            stream.getValue<float>(&sats);
            Serial.println("sats: " + String(sats, 10));

            //Bar
            float pressure = 0;
            stream.getValue<float>(&pressure);
            Serial.println("pressure: " + String(pressure, 10));

            float altitude = 0;
            stream.getValue<float>(&altitude);
            Serial.println("altitude: " + String(altitude, 10));

            float vert_velocity = 0;
            stream.getValue<float>(&vert_velocity);
            Serial.println("vert_velocity: " + String(vert_velocity, 10));

            float temperature = 0;
            stream.getValue<float>(&temperature);
            Serial.println("temperature: " + String(temperature, 10));

            //Bat
            float bat1 = 0;
            stream.getValue<float>(&bat1);
            Serial.println("bat1: " + String(bat1, 10));

            float bat2 = 0;
            stream.getValue<float>(&bat2);
            Serial.println("bat2: " + String(bat2, 10));
            
            //Mag
            float x = 0;
            stream.getValue<float>(&x);
            Serial.println("magx: " + String(x, 10));

            float y = 0;
            stream.getValue<float>(&y);
            Serial.println("magy: " + String(y, 10));

            float z = 0;
            stream.getValue<float>(&z);
            Serial.println("magz: " + String(z, 10));

            float acc_x = 0;
            stream.getValue<float>(&acc_x);
            Serial.println("magacc_x: " + String(acc_x, 10));
            
            float acc_y = 0;
            stream.getValue<float>(&acc_y);
            Serial.println("magacc_y: " + String(acc_y, 10));
            
            float acc_z = 0;
            stream.getValue<float>(&acc_z);
            Serial.println("magacc_z: " + String(acc_z, 10));

            Serial.println();

        }
        file.close();
    }

    void readFlash(const char *path)
    {
        File file = LITTLEFS.open(path);
        //This is the size of reading
        auto const buf_size = sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float);   
        while (file.available())
        {
            ;

            StreamPipe<buf_size> stream;
            file.readBytes(stream.buf_out, buf_size);
 

            //Time
            float time = 0;
            stream.getValue<float>(&time);
            Serial.print(String(time, 4) + ",");

            //GPS
            float lat = 0;
            stream.getValue<float>(&lat);
            Serial.print(String(lat, 4) + ",");

            float lng = 0;
            stream.getValue<float>(&lng);
            Serial.print(String(lng, 4) + ",");

            float alt = 0;
            stream.getValue<float>(&alt);
            Serial.print(String(alt, 4) + ",");

            float sats = 0;
            stream.getValue<float>(&sats);
            Serial.print(String(sats, 4) + ",");

            //Bar
            float pressure = 0;
            stream.getValue<float>(&pressure);
            Serial.print(String(pressure, 4) + ",");

            float altitude = 0;
            stream.getValue<float>(&altitude);
            Serial.print(String(altitude, 4) + ",");

            float vert_velocity = 0;
            stream.getValue<float>(&vert_velocity);
            Serial.print(String(vert_velocity, 4) + ",");

            float temperature = 0;
            stream.getValue<float>(&temperature);
            Serial.print(String(temperature, 4) + ",");

            //Bat
            float bat1 = 0;
            stream.getValue<float>(&bat1);
            Serial.print(String(bat1, 4) + ",");

            float bat2 = 0;
            stream.getValue<float>(&bat2);
            Serial.print(String(bat2, 4) + ",");
            
            //Mag
            float x = 0;
            stream.getValue<float>(&x);
            Serial.print(String(x, 4) + ",");

            float y = 0;
            stream.getValue<float>(&y);
            Serial.print(String(y, 4) + ",");

            float z = 0;
            stream.getValue<float>(&z);
            Serial.print(String(z, 4) + ",");

            float acc_x = 0;
            stream.getValue<float>(&acc_x);
            Serial.print(String(acc_x, 4) + ",");
            
            float acc_y = 0;
            stream.getValue<float>(&acc_y);
            Serial.print(String(acc_y, 4) + ",");
            
            float acc_z = 0;
            stream.getValue<float>(&acc_z);
            Serial.println(String(acc_z, 4));


        }
        file.close();
    }

    void lock()
    {
        Serial.println("Flash locked");
        EEPROM.writeFloat(40, 5); //5 chosen as arbitrary value in case this address is used for more fucntionality (file names)
        EEPROM.commit();
    }

    void unlock()
    {
        Serial.println("Flash unlocked");
        EEPROM.writeFloat(40, 0);
        EEPROM.commit();
    }

    bool locked()
    {
        if (EEPROM.readFloat(40) == 5)
        {
            Serial.println("Cannot delete flash - locked");
            return 1;
        }
        else {return 0;}
    }

    void closeFile(File file)
    {
        file.close();
    }
}