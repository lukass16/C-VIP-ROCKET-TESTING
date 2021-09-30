#pragma once

#include "Arduino.h"
#include "MPU9250.h"
#include "buzzer.h"
#include "sensor_data.h"
#include "EEPROM.h"

//TODO add EEPROM with savedEEPROM() function

namespace magnetometer
{
    // an MPU9250 object with the MPU-9250 sensor on I2C bus 0 with address 0x68
    MPU9250 IMU(Wire, 0x68);
    int status;

    // declaring variables for maximum and minimum values in all of the axes
    float maxx = 71.72;
    float maxy = 69.32;
    float maxz = 7.16;
    float minx = -33.59;
    float miny = -34.93;
    float minz = -98.09;

    // declaring variables for current x y z values in the loop
    float cx = 0;
    float cy = 0;
    float cz = 0;

    // declaring variable for time up to last change and interval
    // since the last change of value, time in loop and time when loop started
    unsigned long time_up_to_change = 0;
    unsigned long interval_since_change = 0;
    unsigned long time_in_loop = 0;
    unsigned long start_time = 0;

    // declaring variable for the detectable interval since last change of max min value in milliseconds
    int interval = 10000;

    // declaring variables for offsets
    float offset_x = 1;
    float offset_y = 2;
    float offset_z = 3;

    // declaring variables for average deltas
    float avg_delta = 0;
    float avg_delta_x = 0;
    float avg_delta_y = 0;
    float avg_delta_z = 0;

    // declaring variables for x y z scale factors
    float scale_x = 4;
    float scale_y = 5;
    float scale_z = 6;

    // declaring variables for corrected x y z values
    float cor_x;
    float cor_y;
    float cor_z = 99;

    //* TIMER FUNCTIONALITY
    //creating variables for timer and launch
    unsigned long periodOfAcc = 0, lastTime = 0, detAccDur = 40; //detAccDur in milliseconds (/?)
    float detAcc = 0;
    int countAcc = 0;

    //creating a variable for timer detection of apogee
    volatile bool timerDetAp = 0;

    //creating a pointer to a variable of type hw_timer_t
    hw_timer_t *timer = NULL;

    //declaring a variable of type portMUX_TYPE, which will be used to take care of the synchronization between the main loop and the ISR, when modifying a shared variable
    portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

    //creating the interrupt handling function - should be as short as possible
    //The interrupt handling routine should have the IRAM_ATTR attribute, in order for the compiler to place the code in IRAM
    void IRAM_ATTR onTimer()
    {
        //since the variable is shared with the main loop it will be modified inside a critical section, declared with the portENTER_CRITICAL_ISR and portEXIT_CRITICAL_ISR macros
        portENTER_CRITICAL_ISR(&timerMux);
        timerDetAp = 1;
        portEXIT_CRITICAL_ISR(&timerMux);
    }

    bool launch()
    {
        if (IMU.getAccelY_mss() > detAcc) //-20
        {
            countAcc++;
        }
        if (countAcc > 50)
        {
            Serial.println("Writing to EEPROM that launch detected");
            EEPROM.writeFloat(36, 1); //*Adding that launch is detected
            //for testing
            countAcc = 0;
            return 1;
        }
        return 0;
    }

    //*Legacy launch() function - undecided yet as to which will be chosen
    // bool launch()
    // {
    //     if (IMU.getAccelZ_mss() > detAcc) //-20
    //     {
    //         lastTime = millis();
    //     }
    //     periodOfAcc = millis() - lastTime;
    //     Serial.print("Period of acceleration: ");
    //     Serial.println(periodOfAcc);
    //     if (periodOfAcc > detAccDur)
    //     {
    //         Serial.println("Writing to EEPROM that launch detected");
    //         EEPROM.writeFloat(36, 1);
    //         return 1;
    //     }
    //     return 0;
    // }

    void startApogeeTimer(int timerLength)
    {
        //initializing timer - setting the number of the timer, the value of the prescaler and stating that the counter should count up (true)
        timer = timerBegin(0, 80, true); //?Is there a possibilty of an error if the same timer is used many times for different applications?

        //binding the timer to a handling function
        timerAttachInterrupt(timer, &onTimer, true);

        //specifying the counter value in which the timer interrupt will be generated and indicating that the timer should not automatically reload (false) upon generating the interrupt
        timerAlarmWrite(timer, timerLength, false);

        //enabling the timer
        timerAlarmEnable(timer);

        Serial.println("TIMER ENABLED");
    }

    bool timerDetectApogee()
    {
        if (timerDetAp)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    //*TIMER FUNCTIONALITY END

    //*ACCELEROMETER FUNCTIONALITY
    // declaring variables for current accelerometer x y z values in the loop
    float acc_x = 0, acc_y = 0, acc_z = 0;

    float getAccX()
    {
        acc_x = IMU.getAccelX_mss();
        return acc_x;
    }

    float getAccY()
    {
        acc_y = IMU.getAccelY_mss();
        return acc_y;
    }

    float getAccZ()
    {
        acc_z = IMU.getAccelZ_mss();
        return acc_z;
    }

    void getAccelValues()
    {
        getAccX();
        getAccY();
        getAccZ();
    }

    void displayAcceleration()
    {
        Serial.println("Accel_X: " + String(acc_x));
        Serial.println("Accel_Y: " + String(acc_y));
        Serial.println("Accel_Z: " + String(acc_z));
        Serial.println("");
    }

    int calibrateAccel()
    {
        return IMU.calibrateAccel();
    }
    //*ACCELEROMETER FUNCTIONALITY END

    //*EEPROM FUNCTIONALITY
    void saveCorToEEPROM()
    {
        //Trying to add the offset and scale factor values to EEPROM
        Serial.println("Trying to add the offset and scale factor values to EEPROM");
        EEPROM.writeFloat(4, offset_x);
        EEPROM.writeFloat(8, scale_x);
        EEPROM.writeFloat(12, offset_y);
        EEPROM.writeFloat(16, scale_y);
        EEPROM.writeFloat(20, offset_z);
        EEPROM.writeFloat(24, scale_z);

        EEPROM.commit();

        Serial.println("Successfully added offset and scale factors to EEPROM");
    }

    void setAsCalibrated()
    {
        Serial.println("Set magnetometer as calibrated");
        EEPROM.writeFloat(0, 1); //1 for true
        EEPROM.commit();
    }

    void setAsNotCalibrated()
    {
        Serial.println("Set magnetometer as not calibrated");
        EEPROM.writeFloat(0, 0); //1 for true
        EEPROM.commit();
    }

    bool savedCorToEEPROM()
    {
        if (EEPROM.readFloat(0) == 1.)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    void getCorEEPROM()
    {
        offset_x = EEPROM.readFloat(4);
        scale_x = EEPROM.readFloat(8);
        offset_y = EEPROM.readFloat(12);
        scale_y = EEPROM.readFloat(16);
        offset_z = EEPROM.readFloat(20);
        scale_z = EEPROM.readFloat(24);
        Serial.println("Retrieved magnetometer calibration values from EEPROM");
    }

    void clearEEPROM()
    {
        for (int i = 0; i <= 36; i = i + 4)
        {
            EEPROM.writeFloat(i, 0.0);
        }
        Serial.println("Cleared EEPROM");
        EEPROM.commit();
    }

    //TODO test
    bool hasBeenLaunch()
    {
        if (EEPROM.readFloat(36) == 1)
        {
            Serial.println("Previous launch detected!");
            return 1;
        }
        else
        {
            Serial.println("Has not yet been launched!");
            return 0;
        }
    }

    void displayCor()
    {
        Serial.print("offset_x ");
        Serial.println(offset_x);
        Serial.print("scale_x ");
        Serial.println(scale_x);
        Serial.print("offset_y ");
        Serial.println(offset_y);
        Serial.print("scale_y ");
        Serial.println(scale_y);
        Serial.print("offset_z ");
        Serial.println(offset_z);
        Serial.print("scale_z ");
        Serial.println(scale_z);
    }
    //*EEPROM FUNCTIONALITY END

    void getMagValues()
    {
        cor_x = (IMU.getMagX_uT() - offset_x) * scale_x;
        cor_y = (IMU.getMagY_uT() - offset_y) * scale_y;
        cor_z = (IMU.getMagZ_uT() - offset_z) * scale_z;
    }

    void printCalibratingValues()
    {
        Serial.print("\t max X: ");
        Serial.print(maxx);
        Serial.print("\t max Y: ");
        Serial.print(maxy);
        Serial.print("\t max Z: ");
        Serial.print(maxz);
        Serial.println("");
        Serial.print("\t min X: ");
        Serial.print(minx);
        Serial.print("\t min Y: ");
        Serial.print(miny);
        Serial.print("\t min Z: ");
        Serial.println(minz);
    }

    void setup()
    {
        status = IMU.begin();
        if (status < 0)
        {
            Serial.println("IMU initialization unsuccessful");
            Serial.println("Check IMU wiring or try cycling power");
            Serial.print("Status: ");
            Serial.println(status);
            while (true)
                ;
        }

        Serial.println("Magnetometer ready!");
    }

    boolean isApogee(float angle = cor_z)
    {
        return angle <= 17;
    }

    void calibrate(boolean skip = false)
    {

        if (skip)
        {
            Serial.println("Calibration SKIPPED!!!");
            return;
        }

        Serial.println("Calibrate... NOW...");

        //finding max and min values in all of the axes
        //assigning start time for the loop
        start_time = millis();
        //the loop will end after an interval of 30 seconds in which the min or max value hasn't changed in any of the axes
        while (interval_since_change < interval)
        {
            IMU.readSensor();
            // get current mag values

            cx = IMU.getMagX_uT();
            cy = IMU.getMagY_uT();
            cz = IMU.getMagZ_uT();

            //updating time in loop
            time_in_loop = millis() - start_time;

            //get min max x values
            if (cx > maxx)
            {
                maxx = cx;
                time_up_to_change = time_in_loop;
            }
            else if (cx < minx)
            {
                minx = cx;
                time_up_to_change = time_in_loop;
            }

            //get min max y values
            if (cy > maxy)
            {
                maxy = cy;
                time_up_to_change = time_in_loop;
            }
            else if (cy < miny)
            {
                miny = cy;
                time_up_to_change = time_in_loop;
            }

            //get min max z values
            if (cz > maxz)
            {
                maxz = cz;
                time_up_to_change = time_in_loop;
            }
            else if (cz < minz)
            {
                minz = cz;
                time_up_to_change = time_in_loop;
            }

            // // Output the data
            printCalibratingValues();

            //updating the time interval since last change of value
            interval_since_change = time_in_loop - time_up_to_change;

            Serial.println(interval_since_change);
            //Serial.println("Calibrate... END !!!");
        }

        //signaling the completion of calibration using the piezo
        // buzzer::buzz(200);
        // delay(500);
        // buzzer::buzz(500);
        // delay(500);
        // buzzer::buzz(800);
        // delay(500);
        // buzzer::buzz(200);

        //getting offset values
        offset_x = (maxx + minx) / 2;
        offset_y = (maxy + miny) / 2;
        offset_z = (maxz + minz) / 2;

        //getting average deltas for x y z axes
        avg_delta_x = (maxx - minx) / 2;
        avg_delta_y = (maxy - miny) / 2;
        avg_delta_z = (maxz - minz) / 2;
        //getting the average delta
        avg_delta = (avg_delta_x + avg_delta_y + avg_delta_z) / 3;

        //getting the scale factors
        scale_x = avg_delta / avg_delta_x;
        scale_y = avg_delta / avg_delta_y;
        scale_z = avg_delta / avg_delta_z;

        Serial.println("Calibrate... END !!!");
    }

    void displayData()
    {
        Serial.print("\t");
        Serial.print(cor_x);
        Serial.print("\t");
        Serial.print(cor_y);
        Serial.print("\t");
        Serial.print(cor_z);
        Serial.println("");
    }

    sens_data::MagenetometerData getMagnetometerState()
    {
        sens_data::MagenetometerData MDat;
        MDat.x = cor_x;
        MDat.y = cor_y;
        MDat.z = cor_z;
        return MDat;
    }

    void processApogee()
    {
        if (isApogee(cor_z))
        {
            buzzer::buzz(125);
        }
        else
        {
            buzzer::buzz(0); //*Could this perhaps be incorect?
        }
    }

    void readMagnetometer()
    {
        // read the sensor
        IMU.readSensor();

        getMagValues();
        getAccelValues();
        // displayData();
        //processApogee();
    }

    void testCalibratedAxis(boolean isForced = false) //*Should change
    {
        int i = 0;
        while (i <= 100 || isForced)
        {
            readMagnetometer();

            if (!isApogee())
            {

                Serial.println("waiting...");
            }
            else
            {
                i++;
                Serial.println("APOGEE !!!");
                printCalibratingValues();
            }
        }
        buzzer::buzz(0);
    }

}