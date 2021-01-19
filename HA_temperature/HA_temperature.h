/*
   Copyright (C) 2020  Andrew Richards

   Part of home automation suite

   Loosely modelled on a light weight and heavily adapted version of the Dallastemperature library

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#ifndef HA_temperature_h
#define HA_temperature_h

#include <inttypes.h>
#include "OneWire.h"
#include "wakeup.h"
#include "TimerOne.h"            // NB: modified version of public TimerOne library
#include "HA_globals.h"

// ***********  Adapted version of DALLAS LIB VERSION "3.7.2"   ************

// Model IDs
const static byte DS18S20 = 0x10;
const static byte DS18B20 = 0x28;
const static byte DS1822 = 0x22;

// OneWire commands
const static byte STARTCONVO = 0x44;  // Tells device to take a temperature reading and put it on the scratchpad
const static byte COPYSCRATCH = 0x48;  // Copy EEPROM
const static byte READSCRATCH = 0xBE;  // Read EEPROM
const static byte WRITESCRATCH = 0x4E;  // Write to EEPROM

// ROM locations
const static byte ROM_FAMILY = 0;
const static byte ROM_DEVICE_ID = 1;
const static byte ROM_CRC = 7;

// Scratchpad locations
const static byte TEMP_LSB = 0;
const static byte TEMP_MSB = 1;
const static byte HIGH_ALARM_TEMP = 2;
const static byte LOW_ALARM_TEMP = 3;
const static byte CONFIGURATION = 4;
const static byte INTERNAL_BYTE = 5;
const static byte COUNT_REMAIN = 6;
const static byte COUNT_PER_C = 7;
const static byte SCRATCHPAD_CRC = 8;

// Device precision
const static byte TEMP_9_BIT = 0x1F; 
const static byte TEMP_10_BIT = 0x3F; 
const static byte TEMP_11_BIT = 0x5F; 
const static byte TEMP_12_BIT = 0x7F; 

// Conversion time - see data sheets
const static int T_CONV_9_BIT = 94;    // All per datasheet
const static int T_CONV_10_BIT = 188;   
const static int T_CONV_11_BIT = 375;   
const static int T_CONV_12_BIT = 750;
const static int T_CONV_DS18S20 = 750;
//const static int T_CONV_DS18B20[] = { 94, 188, 375, 750 };   // Array for fast lookup 

const static int MIN_TEMP = 1;            // Minimum credible temperature - less than this suggests sensor is faulty (or house very cold)
const static int MAX_TEMP = 95;           // Max allowable temp for sensors
const static int ERR_TEMP = 99;           // To indicate error reading
const static int RESET_TEMP = 85;         // Power-on reset temperature
 
// Bit states for s_conversionState
const static byte RESET_SENSOR = 0;
const static byte GET_TEMP = 1;

// Delay between initialisation and first read request
#define IMMEDIATE_READ_MS 10



class HA_temperature
{
  public:

    HA_temperature() { 
        _sensorID = s_nextSensorID++; 
        if (s_nextSensorID > NUM_TEMP_SENSORS) logError(0x02);
    };
      
    boolean init(byte pin, byte targetPrecision, byte pollingFreq);     // Set up the device
    
    float getTempC();                                                   // Get the latest reading

  private:

    // Used to allocate unique sensorID for each member - initialised to zero in HA_temperature.cpp & incremented on member instantiation
    static byte s_nextSensorID;                      
    
    // Flags - bit-wise, indexed by sensorID
    static volatile byte s_sensorInUse;                      // To allow co-operative access to sensor bus
    static volatile byte s_DS18S20;                          // Bit set if DS18S20, which needs additional processing
    static volatile byte s_conversionState;                  // Controls state/process path through scheduleTempC
    static volatile byte s_sensorError;                      // Allows up to one transient error before flagging ERR_TEMP

    // Values
    static unsigned int s_convTime[NUM_TEMP_SENSORS];
    static volatile float s_tempC[NUM_TEMP_SENSORS];              // Holds the latest temperature from the sensor - can be updated via ISR, hence volatile

    byte _sensorID;                                 // One per class member

    // Underlying comms bus to access temperature sensor
    static OneWire oneWire[NUM_TEMP_SENSORS];                    

    static void scheduleTempC(byte sensorID);                // Main processing loop - re-entrant two-state processing using wakeup to avoid blocking

    static boolean reserveBus(byte sensorID);               // Used to control semaphore access to the bus
    static void releaseBus(byte sensorID);

    static boolean getBit(volatile byte* flags, byte sensorID);                    // Used to get/set the main processing (scheduleTempC) state
    static void setBit(volatile byte* flags, byte sensorID, boolean state);

    static byte readPrecision(byte sensorID);               // Helper functions in support of scheduleTempC
    static boolean readScratchPad(byte sensorID, byte* scratchpad);
  
};

#endif

