 /*
    Copyright (C) 2020  Andrew Richards
 
    Part of home automation suite
    
    Loosely modelled on a light weight and heavily adapted version of the Dallastemperature library
    Main assumption is that oneWire bus is used by one Dallas temperature sensor ONLY. Multi-drop or other
    oneWire devices on the bus are not catered for

    To use:
    - declare an HA_temperature object
    - call INIT to identify its physical pin, target precision (DS18S20 and DS1822 devices only) and polling frequency
    - call getTempC periodically to get the latest readout


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



#include "HA_temperature.h" 
#include "HA_globals.h"
#include "HA_syslog.h"


// PUBLIC
// ------

boolean HA_temperature::init(byte pin, byte targetPrecision, byte pollingFreq) {

    SAVE_CONTEXT("tInit")

    targetPrecision = constrain(targetPrecision, 9, 12);

    /*     
    Initialise the sensor and store basic config details, then schedule background reads 
    sensorID is allocated on instantiation and available to class objects only - has to be passed as parameter to static members
    */

    // Proceed only if spare sensor slots
    if (_sensorID < NUM_TEMP_SENSORS) {

        s_tempC[_sensorID] = ERR_TEMP;          // By default, flag an error reading
        releaseBus(_sensorID);

        /* Bail if sensor already in use - shouldn't happen
        if (!reserveBus(_sensorID)) {
            logError(0xA9);
            return false;
        }*/

        oneWire[_sensorID].init(pin);

        // Reset the bus and get the address of the first (& only) device to determine ROM family
        if (oneWire[_sensorID].reset()) {
#ifdef DEBUG
            byte bufPosn = 0;
            const byte BUFLEN = 64;
            char buffer[BUFLEN];
            #define BUF_ADD bufPosn += snprintf(buffer + bufPosn, BUFLEN - bufPosn, 
#endif

            byte deviceBuffer[8];                      // Onewire device buffer - contains family code (ROM_FAMILY), device ID (not used here) & CRC
            byte configuration;
            byte romFamily;

            oneWire[_sensorID].readROM(deviceBuffer);

#ifdef DEBUG
            BUF_ADD "Sensor %u pin %u ROM: ", _sensorID, pin);
            for (int i = 0; i < 8; i++) BUF_ADD "%02x", deviceBuffer[i]);
            BUF_ADD "\0");
            Serial.println(buffer);
            SENDLOGM('D', buffer);
#endif

            if (oneWire[_sensorID].crc8(deviceBuffer, ROM_CRC) != deviceBuffer[ROM_CRC]) {
                logError(0xA1);
                releaseBus(_sensorID);
                RESTORE_CONTEXT
                return false;
            }

            // Flag whether this is a DS18S20, set the conversion time and set the precision (if applicable)
            switch (deviceBuffer[ROM_FAMILY]) {
                case DS18S20:
                    s_convTime[_sensorID] = T_CONV_DS18S20;               // Fixed 9 bit precision, no option to set target
                    setBit(&s_DS18S20, _sensorID, true);                  // Flag need to determine processing of temperature data
                    break; 
                case DS18B20:
                case DS1822:
                    switch (targetPrecision) {
                        case 12:		configuration = TEMP_12_BIT; s_convTime[_sensorID] = T_CONV_12_BIT;  break;
                        case 11:		configuration = TEMP_11_BIT; s_convTime[_sensorID] = T_CONV_11_BIT;  break;
                        case 10:		configuration = TEMP_10_BIT; s_convTime[_sensorID] = T_CONV_10_BIT;  break;
                        case 9:
                        default:		configuration = TEMP_9_BIT; s_convTime[_sensorID] = T_CONV_9_BIT;  break;
                    }

                    setBit(&s_DS18S20, _sensorID, false);                

                    // Set desired resolution - have to write all three bytes of scratchpad, but as alarm function not used first two can be random
                    oneWire[_sensorID].reset();
                    oneWire[_sensorID].skip();					                  // Avoids the need to send the address (only on single device buses)
                    oneWire[_sensorID].write(WRITESCRATCH);
                    oneWire[_sensorID].write(configuration);
                    oneWire[_sensorID].write(configuration);
                    oneWire[_sensorID].write(configuration);             // Only meaningful byte

                    // Check precision has taken
                    if (readPrecision(_sensorID) != targetPrecision) {
                        logError(0xA2);
                        releaseBus(_sensorID);
                        RESTORE_CONTEXT
                        return false;
                    }
                    break;
                default:                // Unrecognised sensor type
                    logError(0xA3);
                    releaseBus(_sensorID);
                    RESTORE_CONTEXT
                    return false;
            }

            releaseBus(_sensorID);          // Release sensor bus

            // Schedule 'immediate' initial reading - cancel any prior instruction
            wakeup.cancelWakeup((void (*)(void*))scheduleTempC, IMMEDIATE_READ_MS, (void*)_sensorID, TREAT_AS_NORMAL);
            
            if (!wakeup.wakeMeAfter((void (*)(void*))scheduleTempC, IMMEDIATE_READ_MS, (void*)_sensorID, TREAT_AS_NORMAL)) {
                logError(0xA4);
                RESTORE_CONTEXT
                return false;
            }

            // Schedule regular temperature reads
            wakeup.cancelWakeup((void (*)(void*))scheduleTempC, pollingFreq * 1000, (void*)_sensorID, TREAT_AS_NORMAL | REPEAT_COUNT);
            
            if (wakeup.wakeMeAfter((void (*)(void*))scheduleTempC, pollingFreq * 1000, (void*)_sensorID, TREAT_AS_NORMAL | REPEAT_COUNT)) {
                RESTORE_CONTEXT
                return true;
            }
            else {
                logError(0xA4);
                RESTORE_CONTEXT
                return false;
            }
        }
        else {      // Error on Reset() 
            logError(0xAA);
            releaseBus(_sensorID);
            RESTORE_CONTEXT
            return false;
        }
    }
    else {    // No more sensor slots
        logError(0xA0);
        RESTORE_CONTEXT
        return false;
    }
}

float HA_temperature::getTempC() {

    // Return latest temperature reading
    return s_tempC[_sensorID];
}

// PRIVATE - see comments in HA_temperature.h for detail
// -----------------------------------------------------

byte HA_temperature::s_nextSensorID = 0;

volatile byte HA_temperature::s_sensorInUse = 0;
volatile byte HA_temperature::s_DS18S20 = 0;
volatile byte HA_temperature::s_conversionState = 0;
volatile byte HA_temperature::s_sensorError = 0;

unsigned int HA_temperature::s_convTime[NUM_TEMP_SENSORS];
volatile float HA_temperature::s_tempC[NUM_TEMP_SENSORS];    

OneWire HA_temperature::oneWire[NUM_TEMP_SENSORS];



void HA_temperature::scheduleTempC(byte sensorID) {

    SAVE_CONTEXT("sTempC")

    /*
    Main background read routine to get temperature - takes around 800ms in elapsed time.  Is a static as function pointer needs to be passed to wakeup and this is too complicated if a member of a class

    Has two states held in relevant bit in s_conversionState
    - reset sensor (unset) - triggered to a schedule set in scheduleTempReadingsFor() and initiate conversion
    - get temperature (set) - triggered once previous state completes

    If error, then reset state and wait for repeat run
    */

    // Select appropriate stage of processing depending on how far through the read process has got
    switch (getBit(&s_conversionState, sensorID)) {

        case RESET_SENSOR:                    // Initial state

            // Bail if sensor already in use
            if (!reserveBus(sensorID)) {
                logError(0xe0);
                RESTORE_CONTEXT
                return;
            }

            // Reset bus and start conversion
            oneWire[sensorID].reset();
            oneWire[sensorID].skip();
            oneWire[sensorID].write(STARTCONVO);

            // Next step will be to read the result . . .
            setBit(&s_conversionState, sensorID, GET_TEMP);

            // . .  which happens in s_convTime ms.  If error, then reset state and release bus
            //int convTime = (getBit(&s_DS18S20, sensorID)) ? T_CONV_DS18S20 : T_CONV_DS18B20[targetPrecision - 9];   // Index into array for DS18B20

            wakeup.cancelWakeup((void (*)(void*))scheduleTempC, s_convTime[sensorID], (void*)sensorID, TREAT_AS_NORMAL);
            
            if (!wakeup.wakeMeAfter((void (*)(void*))scheduleTempC, s_convTime[sensorID], (void*)sensorID, TREAT_AS_NORMAL)) {
                logError(0xA5);
                setBit(&s_conversionState, sensorID, RESET_SENSOR);
                releaseBus(sensorID);
            }

            RESTORE_CONTEXT

            break;

        case GET_TEMP: {                  // Get result of conversion

            byte scratchpad[9];
            float tempC;

            // Get the data into buffer
            if (readScratchPad(sensorID, scratchpad)) {

                // Load the temperature to single variable and add extra resolution if needed
                int reading = (((int)scratchpad[TEMP_MSB]) << 8) | scratchpad[TEMP_LSB];

                if (getBit(&s_DS18S20, sensorID)) {			                    // Fixed 9 bit resolution expandable using 'extended resolution temperature' algorithm
                    reading = reading >> 1;                                 // Truncate 0.5C bit 
                    tempC = (float)reading - 0.25 + ((float)(16 - scratchpad[COUNT_REMAIN]) / 16); 
                }
                else {
                    switch (scratchpad[CONFIGURATION]) {
                        case TEMP_12_BIT: tempC = (float)reading * 0.0625; break;
                        case TEMP_11_BIT: tempC = (float)(reading >> 1) * 0.125; break;
                        case TEMP_10_BIT: tempC = (float)(reading >> 2) * 0.25; break;
                        case TEMP_9_BIT: 
                        default: tempC = (float)(reading >> 3) * 0.5; break;
                    }
                }

#ifdef DEBUG
                byte bufPosn = 0;
                const byte BUFLEN = 64;
                char buffer[BUFLEN];
                #define BUF_ADD bufPosn += snprintf(buffer + bufPosn, BUFLEN - bufPosn, 
                
                BUF_ADD "Sensor %u (%s). Scratchpad: ", sensorID, (getBit(&s_DS18S20, sensorID)) ? "18S" : "18B");
                for (int i = 0; i < 9; i++) BUF_ADD "%02x", scratchpad[i]);
                BUF_ADD " reading: %u\0", (unsigned int)tempC);
                Serial.println(buffer);
                SENDLOGM('D', buffer);
#endif
            }
            else {
                tempC = ERR_TEMP;
            }

            // Allow one transient error
            if (tempC == ERR_TEMP && !getBit(&s_sensorError, sensorID)) {       // If 1st error then flag but don't record it; on 2nd error will fall through and record ERR_TEMP
                setBit(&s_sensorError, sensorID, true);
            }
            else {                                                              // Save the reading for access by getTempC() - bus is locked, but need to avoid clash with getTempC() when writing float
                if (tempC == ERR_TEMP) logError(0xe1);
                noInterrupts();
                s_tempC[sensorID] = tempC;
                interrupts();
                setBit(&s_sensorError, sensorID, false);
            }

            // All done.  Wakeup will automatically repeat the process, so just need to set the state and hand back the bus to others
            setBit(&s_conversionState, sensorID, RESET_SENSOR);
            releaseBus(sensorID);

            break;
        }
        default: logError(0xD0);
    }

    RESTORE_CONTEXT
}

// I2C bus reservation/release routines
// ------------------------------------

boolean HA_temperature::reserveBus(byte sensorID) {

    // Test if bus already in use; if so then reserve, if not then bail.  
    // Protect test/set against interrupts - in practice probably unnecessary, but extra safeguard

    noInterrupts();

    if (s_sensorInUse & _BV(sensorID)) {
        interrupts();
        return false;
    }
    else {
        s_sensorInUse |= _BV(sensorID);          // Reserve access to the sensor - calling routine must release
        interrupts();

        return true;
    }
}

void HA_temperature::releaseBus(byte sensorID) {
    
    // Restore bus access - not atomic, so need to protect from interrupts
    
    noInterrupts();

    s_sensorInUse &= ~_BV(sensorID);

    interrupts();
}

// One Wire helper functions - run with bus locked
// -----------------------------------------------

boolean HA_temperature::getBit(volatile byte* flags, byte sensorID) {
    return (*flags & _BV(sensorID)) ? true : false;
}

void HA_temperature::setBit(volatile byte* flags, byte sensorID, boolean state) {
    noInterrupts();

    if (state) {
        *flags |= _BV(sensorID); 
    }
    else {
        *flags &= ~_BV(sensorID);
    }

    interrupts();
}

byte HA_temperature::readPrecision(byte sensorID) {

    byte scratchpad[9];

    if (s_DS18S20 & _BV(sensorID)) {     // DS18S20 has only one precision
        return 9;
    }
    else {                              // DS18B20 or DS1822 have configurable precision
        if (readScratchPad(sensorID, scratchpad)) {
            switch (scratchpad[CONFIGURATION]) {
                case TEMP_12_BIT:  return 12; break;
                case TEMP_11_BIT:  return 11; break;
                case TEMP_10_BIT:  return 10; break;
                case TEMP_9_BIT:   return 9; break;
                default: { logError(0xC2); return 0; }
            }
        }
        else {
            logError(0xc1);
            return 0;          // Error reading scratchpad
        }
    }
}

boolean HA_temperature::readScratchPad(byte sensorID, byte* scratchpad) {

    // Read the scratchpad
    if (oneWire[sensorID].reset()) {
        oneWire[sensorID].skip();
        oneWire[sensorID].write(READSCRATCH);

        for (int i = 0; i < 9; i++) scratchpad[i] = oneWire[sensorID].read();

        // Check CRC
        if (oneWire[sensorID].crc8(scratchpad, SCRATCHPAD_CRC) != scratchpad[SCRATCHPAD_CRC]) {
            return false;
        }
        else return true;
    }
    else {
        return false;
    }
}