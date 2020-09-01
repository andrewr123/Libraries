 /*
    Copyright (C) 2011  Andrew Richards
 
    Part of home automation suite
    
    Contains the HA_devHeat class for monitoring Dallas temp sensors

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

 
#ifndef HA_devHeat_h
#define HA_devHeat_h

#include "HA_globals.h"
#include "HA_device_bases.h"
#include "HA_channels.h"
#include "OneWire.h"


// ************ Global constants

const static byte TEMPERATURE_PRECISION = 11;			// Gives single decimal place - more than good enough for home automation



// ********** HA_devHeat ****************
//
// For processing Dallas DS18S20 or DS18B20 one-wire temperature sensors

class HA_devHeat : public HA_dev2Byte {
	public:
		void getRef(char *device);
		void getSnapshot();
		void initDev(byte devNum, byte channel, byte pin = 0, byte handler = 0);
		void resetDev();
		void readDev();
		void getReading(byte arg);
		
	private:
		
		OneWire _oneWire;
		
		// Adapted version of DALLASTEMPLIBVERSION "3.7.2" with following licence:
		//
		// This library is free software; you can redistribute it and/or
		// modify it under the terms of the GNU Lesser General Public
		// License as published by the Free Software Foundation; either
		// version 2.1 of the License, or (at your option) any later version.
		
		// Model IDs
		const static byte DS18S20 = 0x10;
		const static byte DS18B20 = 0x28;
		const static byte DS1822  = 0x22;
		
		// OneWire commands
		const static byte STARTCONVO 			= 0x44;  // Tells device to take a temperature reading and put it on the scratchpad
		const static byte COPYSCRATCH     = 0x48;  // Copy EEPROM
		const static byte READSCRATCH     = 0xBE;  // Read EEPROM
		const static byte WRITESCRATCH    = 0x4E;  // Write to EEPROM
		
		// ROM locations
		const static byte ROM_FAMILY					= 0;
		const static byte ROM_CRC							= 7;
		
		// Scratchpad locations
		const static byte TEMP_LSB        = 0;
		const static byte TEMP_MSB        = 1;
		const static byte HIGH_ALARM_TEMP = 2;
		const static byte LOW_ALARM_TEMP  = 3;
		const static byte CONFIGURATION   = 4;
		const static byte INTERNAL_BYTE   = 5;
		const static byte COUNT_REMAIN    = 6;
		const static byte COUNT_PER_C     = 7;
		const static byte SCRATCHPAD_CRC  = 8;
		
		// Device resolution
		const static byte TEMP_9_BIT  = 0x1F; //  9 bit
		const static byte TEMP_10_BIT = 0x3F; // 10 bit
		const static byte TEMP_11_BIT = 0x5F; // 11 bit
		const static byte TEMP_12_BIT = 0x7F; // 12 bit
		
		// Conversion time - see data sheets
		const static byte T_CONV_9_BIT  = 94;  
		const static byte T_CONV_10_BIT = 188; 
		const static byte T_CONV_11_BIT = 375; 
		const static byte T_CONV_12_BIT = 750;  
		const static byte T_CONV_DS18S20 = 750; 
		
		byte _romFamily;
		byte _buffer[9];			// Buffer used for both address and data to save space
};

typedef void (HA_devHeat::*HA_devHeatMemPtr)(byte arg);



#endif
