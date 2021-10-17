#pragma once

#include "FS.h"
#include <LITTLEFS.h>
#include "sensor_data.h"

#define FORMAT_LITTLEFS_IF_FAILED true

//*Fixed error of abort() by esp32

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
    void writeData(File file, sens_data::GpsData gpsData, sens_data::MagenetometerData magData, sens_data::BarometerData barData)
    {
        //GPS
        float _lat = gpsData.lat;
        float _lng = gpsData.lng;
        float _alt = gpsData.alt;

        auto lat = (uint8_t *)(&_lat);
        auto lng = (uint8_t *)(&_lng);
        auto alt = (uint8_t *)(&_alt);

        //Mag
        auto x = (uint8_t *)(&magData.x);
        auto y = (uint8_t *)(&magData.y);
        auto z = (uint8_t *)(&magData.z);
        auto a = (uint8_t *)(&magData.a);

        //Bar
        auto temperature = (uint8_t *)(&barData.temperature);
        auto altitude = (uint8_t *)(&barData.altitude);
        auto pressure = (uint8_t *)(&barData.pressure);
        auto vert_velocity = (uint8_t *)(&barData.vert_velocity);

        auto const buf_size = sizeof(lat) + sizeof(lng) + sizeof(alt) + sizeof(x) + sizeof(y) + sizeof(z) + sizeof(a) + sizeof(temperature) + sizeof(altitude) + sizeof(pressure) + sizeof(vert_velocity);
        Buffer<buf_size> buffer;

        buffer.push(lat);
        buffer.push(lng);
        buffer.push(alt);

        buffer.push(x);
        buffer.push(y);
        buffer.push(z);
        buffer.push(a);

        buffer.push(temperature);
        buffer.push(altitude);
        buffer.push(pressure);
        buffer.push(vert_velocity);

        if (!file)
        {
            Serial.println("- failed to open file for writing");
            return;
        }
        file.write(buffer.buf, buf_size);
    }


    void readFlash(const char *path)
    {
        File file = LITTLEFS.open(path);
        //This is the size of reading
        auto const buf_size = sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float) + sizeof(float);   
        while (file.available())
        {
            ; //! why?

            StreamPipe<buf_size> stream;
            file.readBytes(stream.buf_out, buf_size);
 
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

            float a = 0;
            stream.getValue<float>(&a);
            Serial.println("magacc: " + String(a, 10));

            //Bar
            float temp = 0;
            stream.getValue<float>(&temp);
            Serial.println("Temperature: " + String(temp, 10));

            float altitude = 0;
            stream.getValue<float>(&altitude);
            Serial.println("Altitude: " + String(altitude, 10));

            float pressure = 0;
            stream.getValue<float>(&pressure);
            Serial.println("Pressure: " + String(pressure, 10));

            float vert_velocity = 0;
            stream.getValue<float>(&vert_velocity);
            Serial.println("Vertical velocity: " + String(vert_velocity, 10));
        }
        file.close();
    }

    void closeFile(File file)
    {
        file.close();
    }
}