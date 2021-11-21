#pragma once
#include <Arduino.h>
#include "EEPROM.h"
#include "buzzer.h"
#include "sensor_data.h"

#define EEPROM_SIZE 255

namespace arming {

    volatile bool timeKeeper = 0;
    hw_timer_t *timer = NULL;
    portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

    void IRAM_ATTR onTimer()
    {
        portENTER_CRITICAL_ISR(&timerMux);
        timeKeeper = 1;
        portEXIT_CRITICAL_ISR(&timerMux);
    }

    //pin definitions
    int nihrom = 33;            //p20 OUTPUT
    int nihrom2 = 25;           //p19 OUTPUT
    int ParachuteBattery1 = 35; //p17 INPUT
    int ParachuteBattery2 = 34; //p18 INPUT

    int FirstSwitch = 39;     //p14 INPUT
    int SecondSwitch = 38; //p16 INPUT
    int ThirdSwitch = 37;  //p15 INPUT

    const int LopyPower = 0;      //!JĀDEFINĒ!
    int out = 26;                 //p21 lampiņa/buzzer
    int EEPROMclear = 2;       //p8 INPUT

    bool fail = 0;
    bool AlreadyCalibrated = 0;
    bool firstSwitchHasBeen = 0;

    unsigned long secondSwitchStart = 0;
    
    //variables for Parachute battery voltage calculation
    float rawVoltage = 0;
    int rawReading = 0;
    float sumVoltage1 = 0;
    float sumVoltage2 = 0;
    float voltage1 = 0;
    float voltage2 = 0;
    float filteredVoltage = 0;

    //variables for Lopy battery voltage calculation
    int FirstSwitchReading = 0;
	int SecondSwitchReading = 0;
	int ThirdSwitchReading = 0;
    int rawReadingLopy = 0;


    void setup()
    {
        pinMode(nihrom, OUTPUT);           //1. nihroma
        pinMode(nihrom2, OUTPUT);          // 2. nihromam
        pinMode(ParachuteBattery1, INPUT); //MOSFET shēmas baterijai
        pinMode(ParachuteBattery2, INPUT); //MOSFET shēmas baterijai

        pinMode(ThirdSwitch, INPUT);
        pinMode(SecondSwitch, INPUT);
        pinMode(FirstSwitch, INPUT);

        pinMode(out, OUTPUT); //? buzzer
        pinMode(EEPROMclear, INPUT_PULLDOWN);

        //sak timeri
        timer = timerBegin(0, 80, true);
        timerAttachInterrupt(timer, &onTimer, true);
        timerAlarmWrite(timer, 5000000, false); //! 5sek - safety Note: code goes through all sensor initializations and calibration while timer is going, should create seperate timer start function
        timerAlarmEnable(timer);

        //EEPROM setup
        EEPROM.begin(EEPROM_SIZE);
        Serial.println("Arming setup complete!");
    }

    float getBattery1Voltage()
    {
        //static int readings = 0;
        //readings++;
        rawReading = analogRead(ParachuteBattery1);
        rawVoltage = (rawReading/320.0);
        //sumVoltage1 += rawVoltage;
        //voltage1 = sumVoltage1/readings;
        //return voltage1;
        return rawVoltage;
    }

    float getBattery2Voltage()
    {
        // static int readings = 0;
        // readings++;
        rawReading = analogRead(ParachuteBattery2);
        rawVoltage = (rawReading/320.0); //!fix int/int
        // sumVoltage2 += rawVoltage;
        // voltage2 = sumVoltage2/readings;
        // return voltage2;
        return rawVoltage;
    }

    float getLopyBatteryVoltage()
    {
        FirstSwitchReading = analogRead(FirstSwitch);
	    SecondSwitchReading = analogRead(SecondSwitch);
	    ThirdSwitchReading = analogRead(ThirdSwitch);
        if(SecondSwitchReading != 0)
	    {
		    rawReadingLopy = SecondSwitchReading;
	    }
	    else
	    {
		    rawReadingLopy = ThirdSwitchReading;
	    }

        return rawReadingLopy / 620.0;
    }

    bool getParachuteBatteryStatus()
    {
        if(voltage1 > 8.1 && voltage2 > 8.1)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    bool checkFirstSwitch()
    {
        if (analogRead(FirstSwitch) > 700)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    bool checkSecondSwitch()
    {
        if (analogRead(SecondSwitch) > 1000)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    bool checkThirdSwitch() //!changed
    {
        if (analogRead(ThirdSwitch) > 1000)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    bool armingSuccess()
    {
        if (AlreadyCalibrated == 1 && fail == 0)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    bool clearEEPROM()
    {
        if(digitalRead(EEPROMclear) == HIGH)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }

    void nihromDisable()
    {
        digitalWrite(nihrom, LOW);
        digitalWrite(nihrom2, LOW);
    }

    void nihromActivate()
    {
        digitalWrite(nihrom, HIGH); //pirmais nihroms //? commented out for testing   
        Serial.println("First Nihrom activated");
        buzzer::buzz(3400);             
                                    //!POSSIBLE PROBLEM WITH TIMER INTERRUPT - SHOULD USE DIFFERENT INTERRUPT HANDLING FUNCTION (otherwise when checking timeKeeper it's already 1)
        timer = timerBegin(0, 80, true);
        timerAttachInterrupt(timer, &onTimer, true);
        timerAlarmWrite(timer, 1000000, false); //1sek
        timerAlarmEnable(timer);

        //*testing
        delay(200); 
        buzzer::buzzEnd();
        //*

        if (timeKeeper)
        {
            Serial.println("Second Nihrom activated"); 
            digitalWrite(nihrom2, HIGH); //otrais nihroms //? commented out for testing
            buzzer::buzz(3400);
            //*testing
            delay(200); 
            buzzer::buzzEnd();
        //*
        }
    }

    sens_data::BatteryData getBatteryState()
    {
        sens_data::BatteryData BDat;
        BDat.bs = getParachuteBatteryStatus();
        BDat.bat1 = getBattery1Voltage();
        BDat.bat2 = getBattery2Voltage();
        return BDat;
    }

}