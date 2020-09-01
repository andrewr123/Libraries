 /*
    Copyright (C) 2011  Andrew Richards
 
    Part of home automation suite
    
    Contains the HA_device class to manage the directory of devices (sensors or actors)

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
    
    Class structure as follows:
    - one base class for all devices (HA_device)
    - three abstractions from that to hold the data (HA_devBit, HA_dev2Byte, HA_devMultiByte)
    - three abstractions from those to represent different device types (HA_sensAnalog, HA_sensDigital, HA_actDigital)
*/

 
#ifndef HA_device_bases_h
#define HA_device_bases_h

#include "HA_globals.h"

// ************** Global constants

const static byte MAX_DEVICES = 128;                       // 127 is limit (0 is reserved as null)

// LS nibble gives index into _deviceMap

const static byte VAL_REF = 0xF0;
const static byte VAL_REGION = 0x00;  
const static byte VAL_ZONE = 0x10;
const static byte VAL_REGION_ZONE = 0x20;								// Special code to allow comparison with region & zone in HA_zone
const static byte VAL_LOCATION = 0x30;
const static byte VAL_STATUS = 0x40;
const static byte VAL_STATE = 0x50;    

const static byte VAL_HANDLER = 0x10 | 1;
const static byte VAL_PIN = 0x20 | 1;
const static byte VAL_CHANNEL =0x30 | 1;
const static byte VAL_DEVNUM = 0x40 | 1;
const static byte MASK_DEVICEMAPIDX = 0x01;            // This &'ed with valxxx gives index for _deviceMap

// Supplementary constants
const static byte VAL_PUSH  = 0x10 | 2;
const static byte VAL_CURR = 0x20 | 2;
const static byte VAL_PREV = 0x30 | 2;
const static byte VAL_ON_EVENT = 0x40 | 2;
const static byte VAL_OCCUPANCY = 0x50 | 2;
const static byte VAL_TARG_TEMP = 0x60 | 2;
const static byte VAL_ACT_TEMP = 0x70 | 2;
const static byte VAL_LUMINANCE = 0x80 | 2;
const static byte VAL_CONTEXT = 0x90 | 2;

const static byte VAL_DEVIDX = 0xA0;

/*
// **************** Device types *********************
// Check HA_root and HA_variables before amending

const static byte DEV_TYPE_TOUCH 			= 0;
const static byte DEV_TYPE_FIRE 			= 1;
const static byte DEV_TYPE_HEAT 			= 2;
const static byte DEV_TYPE_LUMINANCE 	= 3;
const static byte DEV_TYPE_MOTION 		= 4;
const static byte DEV_TYPE_PRESENCE 	= 5;
const static byte DEV_TYPE_RFID 			= 6;
const static byte DEV_TYPE_OPEN 			= 7;
const static byte DEV_TYPE_5APWR 			= 8;
const static byte DEV_TYPE_13APWR 		= 9;
const static byte DEV_TYPE_LOCK 			= 10;
const static byte DEV_TYPE_LIGHT 			= 11;
const static byte DEV_TYPE_RELAY 			= 12;
const static byte NUM_DEV_TYPES 			= 13;		
const static byte DEV_TYPE_NULL 			= 13;						// Used in switcher to indicate no device
//const static byte VAR_TYPE_BYTE 		= 13;					// Definitions in HA_variables.h
//const static byte VAR_TYPE_2BYTE 		= 14;
//const static byte VAR_TYPE_RFID 		= 15;		
const static byte OBJ_TYPE_ZONE				= 16;
const static byte NUM_ZONE_TYPES			= 1;
const static byte OBJ_TYPE_TIME				= 17;
const static byte OBJ_TYPE_HEARTBEAT 	= 18;
const static byte NUM_OBJ_TYPES				= 2;
const static unsigned int BINARY_DEV = (B11111 * 256) + B00110011;			// Flags indicating which devices are binary

*/


class HA_device {				// Base class for all devices
	public:
		// Methods

		void put (byte type, unsigned int val);
		unsigned int get(byte type);
		byte putRef(char *device);
		void getRef(char *device, byte devType);
		void getSnapshot(byte devType);
	
	protected:
		volatile unsigned int _deviceMap[2];  		// Holds basic identity and status information	
		byte _devNum;															// Index number
		
		// Layout of _deviceMap 
		// 
		// First word
		const static unsigned int MASK_REGION = (B11100000 * 256) + 0x00;        	// Up to 8 regions
		const static unsigned int MASK_ZONE = (B00011100 * 256) + 0x00;      			// Up to 8 zones per region.  
		const static unsigned int MASK_REGION_ZONE = (B11111100 * 256) + 0x00;		// Combined code
		const static unsigned int MASK_LOCATION = (B00000011 * 256) + B11110000;  // Up to 64 locations per zone
		const static unsigned int MASK_STATUS = B00001100;                    		// 0 = stable; 1 = pending; 2 = unset; 3 = unavailable
		const static unsigned int MASK_STATE = B00000011;													// For on/off devices, holds current (MSB) and previous (LSB) states
		
		const static byte OFFSET_REF = 0;
		const static byte OFFSET_REGION = 13;
		const static byte OFFSET_ZONE = 10;
		const static byte OFFSET_REGION_ZONE = 10;
		const static byte OFFSET_LOCATION = 4;
		const static byte OFFSET_STATUS = 2;
		const static byte OFFSET_STATE = 0;
		
		// Second word
		const static unsigned int MASK_HANDLER = (B11000000 * 256);							// 2 bits, up to 4 handlers per device type
		const static unsigned int MASK_PIN = (B00111111 * 256) + B10000000;   	// 7 bits, up to 128 pins per channel.  Pin 127 is NOP
		const static unsigned int MASK_CHAN = B01111000;												// 4 bits, up to 16 channels (routes to pins)
		const static unsigned int MASK_DEVICENUM = B00000111;                   // Optional device number, up to 7 devices - eg P1, P7.  0 normally indicates null, but xO always has devNum, stored as n-1
		
		const static byte OFFSET_HANDLER = 14;
		const static byte OFFSET_PIN = 7;
		const static byte OFFSET_CHAN = 3;
		const static byte OFFSET_DEVICENUM = 0;
		
		
};


class HA_devBit : public HA_device {						// Generic class for on/off devices.  No local storage needed - data held in base class
	public:
	  		
	  void put(byte valType, unsigned int val);
	  unsigned int get(byte valType);
		
	  void put(byte valType, void *valPtr);
	  void get(byte valType, void *valPtr);
	  
		void readDev();
		void setDev(byte val);
	  
	protected:
		const static unsigned int MASK_CURR = B10;
		const static unsigned int MASK_PREV = B1;
		const static byte OFFSET_CURR = 1;
		const static byte OFFSET_PREV = 0;
		
};



class HA_dev2Byte : public HA_device {						// Generic class for sensors with readings capable of storage in 16 bits
	public:
//		HA_dev2Byte();
	 
		void put(byte valType, unsigned int val);
	  unsigned int get(byte valType);
	   		
	  void put(byte valType, void *valPtr);
	  void get(byte valType, void *valPtr);
	  
	protected:
		unsigned int _readingCurrent;
		unsigned int _readingPrevious;
		
};


class HA_devMultiByte : public HA_device {	// Generic class for sensors with multi-byte readings - buffer is a pointer to storage
	public:
		HA_devMultiByte();											// Instantiate class, but no buffer.  Caller must either use create() or init() with pointer to memory
		HA_devMultiByte(byte bufLen);						// Instantiate class with buffer
		~HA_devMultiByte();											// Remove class and free space
		
		boolean create(byte bufLen);						// Creates buffer from dynamic memory
		void init(byte *addr, byte bufLen);			// Set buffer to point to memory obtained by caller
	  void freeMem();													// Frees buffer from memory.  Can be called by user, but only if deconstructor not applicable
	  byte bufLen();
	  
		void put(byte valType, unsigned int val);
	  unsigned int get(byte valType);

	  void put(byte valType, byte *valBuf);
	  void get(byte valType, byte *valBuf);
	  byte *getBufPtr(byte valType);
	  
	  void putElem(byte valType, byte elemIdx, byte elemVal);
	  byte getElem(byte valType, byte elemIdx);
		
	protected:
		byte *_bufCurr;
		byte *_bufPrev;
		byte _bufLen;
		
};

// Abstractions from base classes

class HA_sensAnalog : public HA_dev2Byte {
	public:
		void initDev(byte devNum, byte channel = 0, byte pin = 0, byte handler = 0);
		void readDev(byte devType = 0);
		void put(byte valType, unsigned int val, byte devType = 0);
	  unsigned int get(byte valType);
		
	private:
		byte _onEvent;						// Argument to use when sensor changes.  Either a direct relay number (if Handler == 0), or ArgList number containing a list of relays (if Handler == 1)
};


class HA_sensDigital : public HA_devBit {
	public:
		void initDev(byte devNum, byte channel = 0, byte pin = 0, byte handler = 0);
		void readDev();
		void put(byte valType, unsigned int val);
	  unsigned int get(byte valType);
		
	private:
		byte _onEvent;						// Argument to use when sensor changes
};


class HA_actDigital : public HA_devBit {
	public:
		void initDev(byte devNum, byte channel = 0, byte pin = 0, byte handler = 0);
		void setDev(byte dval);
};
/*
// ************** Global routines ***************

byte getDevTypeIdx (char *device);
void getDevTypeChar (byte devIdx, char *devChar);
*/
#endif
