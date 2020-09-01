 /*
    Copyright (C) 2011  Andrew Richards
    Bitstring library providing raw, FIFO and LIFO (AKA stack) manipulation of arbitrary length bitstrings

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
/*
 
#ifndef HomeAutom_h
#define HomeAutom_h

#include "WProgram.h"

// Limits
const static byte maxDevices = 128;                       // 127 is limit (0 is reserved as null)
const static byte maxVars = 64;                           // 127 is limit

// constants
const static byte valRef = 0xf0;
const static byte valRegion = 0x00;  
const static byte valZone = 0x10;
const static byte valLocation = 0x20;
const static byte valSensor = 0x30;
const static byte valType = 0x40;
const static byte valArduino = 0x50 | 1;
const static byte valPin = 0x60 | 1;
const static byte valCascade =0x70 | 1;
const static byte valHandler = 0x80 | 1;
const static byte valPollFreq = 0x90 | 1;
const static byte valStatus = 0xa0 | 2;
const static byte valStackMode = 0xb0 | 2;
const static byte valTOSIdx = 0xc0 | 2;
const static byte valStack = 0xd0 | 2;                    // Stack of bits, or index to stack of values
const static byte valCurr = 0xe0 | 2;              // GET or PUT value to top of history stack; value is either a bit and stored in Stack (StackMode = 0) or an int stored in readingHistory
const static byte valPrev = 0xf0 | 2;                    // GET previous reading
const static byte valAvg = 0x14 | 2;                    // GET average of stack values
const static byte valMax = 0x24 | 2;                    // GET max of stack values
const static byte valMin = 0x34 | 2;                    // GET min of stack values
const static byte valROC = 0x44 | 2;                    // GET rate of change of stack values
const static byte valDay = 0x54 | 2;                    
const static byte valHour = 0x64 | 2;                    
const static byte valMinute = 0x74 | 2;      

const static byte valStatusStable = 0x00;
const static byte valStatusPending = 0x01;             // Used to indicate sensor reading initiated and to be read on next heartbeat (used for slow devices such as temperature)
const static byte valStatusTarget = 0x02;              // Used to indicate target physical state of device, typcially requiring pin to be set
const static byte valStatusUnset = 0x03;
const static byte valSlowCapture = 3;               // Indicates temp sensor read triggered previously, so collect reading this time
	

const static byte valToInt = 0;
const static byte valToChar = 1;

const static byte readFlag = 0x01;
const static byte writeFlag = 0x02;
const static byte pinNoOp = 127;
const static byte stackSize = 8;
const static int maxReadings = stackSize * 80;            // Max zones (xH and xL) = 16 * 2 + max windows = 32 + max doors = 16
const static byte maskLSB = 0x01;
const static byte mask8BitMSB = 0x80;
extern byte arduinoMe;


class REGION {
public:
	// Methods
	REGION();
	~REGION();
	
protected:
	// Properties
	char _regionCode;		// See Types.h
};
	
class ZONE {					// Instantiated as an array containing all zones on this Arduino (max 256)
public:
	// Methods
	ZONE();
	~ZONE();
	
protected:
  // Properties
	REGION *parentRegion;
	byte zoneNum;
	byte zoneOccupancy;
	unsigned int _zoneTargetTemp;
	unsigned int _zoneActualTemp;
	unsigned int _zoneLuminance;
};

class DEVICE {				// Instantiated as an array containing all devices on this Arduino
public:
	// Methods
	DEVICE();
	~DEVICE();
	
protected:
  // Properties
  //unsigned int deviceMap[3][maxDevices];      // [0] = coded device ref; [1] = device handler; [2] = state  
  unsigned int _deviceRef;
  unsigned int _deviceHandler;
  unsigned int _deviceState;
  
  const static byte deviceRefIdx = 0x00;
  const static byte deviceHandlerIdx = 0x01;
  const static byte deviceStateIdx = 0x02;
  
  const static byte maskDeviceMapIdx = 0x03;            // This &'ed with valxxx gives deviceRefIdx, deviceHandlerIdx, deviceStateIdx

	// deviceMap ref ([0]) gives unique ref for device.  Decode format is an.nn.aaa, coded as follows
	const static unsigned int maskRef = (0xff * 256) + 0xff;                // Zero is NULL reference used to indicate evaluation not to write
	const static unsigned int maskRegion = (B11100000 * 256) + 0x00;        // Up to 8 regions
	const static unsigned int maskZone = (B00011100 * 256) + 0x00;      // Up to 8 zones per region.  NB: certain zones need rationalising
	const static unsigned int maskLocation = (B00000011 * 256) + B11110000;  // Up to 64 locations per zone (0 reserved for whole zone). 
	const static unsigned int maskDeviceType = 0x0f;                                // Up to 8 sensors & 8 actors
	const static byte offsetRegion = 13;
	const static byte offsetZone = 10;
	const static byte offsetLocation = 4;
	const static byte offsetDeviceType = 0;
 
  // deviceMap handler ([1]) tells how to access the physical device.  Decode as follows:
//  const static unsigned int maskArduino = (B11100000 * 256) + 0x00;      // 3 bits, up to 8 Arduinos
  const static unsigned int maskHandler = (B11110000 * 256;							// 4 bits, up to 16 handlers per device type
  const static unsigned int maskPin = (B00001111 * 256) + B11111000;    // 9 bits, up to 512 pins/virtual pins per handler.  Pin 511 is NOP
  const static unsigned int maskCascade = B00000100;                    // 1 = cascade this reading to the next deviceIdx; 0 = no cascade
  const static unsigned int maskFreq = B00000011;                    // 2 bits, up to 4 action frequencies
//  const static byte offsetArduino = 13;
  const static byte offsetHandler = 12;
  const static byte offsetPin = 3;
  const static byte offsetCascade = 2;
  
  // deviceMap state([2]) holds the current status of the device.  Decode as follows:
  const static unsigned int maskStatus = (B11000000 * 256) + 0x00;            // 0 = stable; 1 = sensor reading pending; 2 = target; 3 = unset
//  const static unsigned int maskStackMode = (B00100000 * 256) + 0x00;            // How to interpret Stack.  0 = bitmap, 1 = index
  const static unsigned int maskTOSIdx = (B00011100 * 256) + 0x00;            // 3 bits = 8 values, only applicable for StackMode = 1; gives offset on Stack * stackSize to latest reading in readingHistory
  const static unsigned int maskStack = 0xFF;                               // Stackmode = 0 - 8 bits of On/Off history (MSB = latest) for on/off device types (xTo, xF, xM, xP, P1, P2, P, p, D, L, R)
                                                                    // Stackmode = 1 - 0-127 * stackSize as an index into readingHistory, with TOSIdx giving offset to current top of stack                                                                      
  const static byte offsetStatus = 14;
//  const static byte offsetStackMode = 13;
  const static byte offsetTOSIdx = 10;
	
	const static byte valTestTemp = 2;
	const static byte valTestSlow = 3;
 
};



class SENSOR : public DEVICE {
public:
	// Methods
	SENSOR ();
	~SENSOR();
	
protected:
	struct _sensor {
		byte sensorID;
		byte sensorCode;		// See Types.h
	};
};



class ACTOR : public DEVICE {
public:
	// Methods
	ACTOR ();
	~ACTOR();
	
protected:
	struct _actor {
		byte actorID;
		byte actorCode;		// See Types.h
	};
};

class MAPHELPER {
public:
	// Methods
	MAPHELPER();							
	~MAPHELPER();					
  
	unsigned int get(byte deviceIdx, byte type);
	void put (byte deviceIdx, byte type, unsigned int value);
	unsigned int convertRefToBit (char *device);
	void convertRefToChar (unsigned int bitstring, char *device);
	boolean isTempSensor (byte deviceIdx);
	boolean isSlowSensor (byte deviceIdx);
	byte getCalcIdx (char *calcTypeChar);
	void getCalcChar(byte calcTypeIdx, char *calcTypeChar);
	byte stackMode (byte deviceIdx);
	unsigned int stackGet (byte deviceIdx, unsigned int element);
	void stackPush (byte deviceIdx, unsigned int value);
	
protected:
	// Methods
	unsigned int access(byte deviceIdx, byte type, unsigned int value, int flag);
	unsigned int stackAccess (byte deviceIdx, unsigned int element, unsigned int value, byte flag);
	unsigned int convertRef(char *device, unsigned int bitstring, byte type);

   unsigned int readingHistory [maxReadings];        // Only used if StackMode == 1. Interpretation varies dependent on deviceType:
                                                    // For xH is temperature(C) x 10 (+/-), so 25.3 = 253 (negative values cast to unsigned)
                                                    // For xo/xL (open/light) is measured voltage of sensor x 100, so 5v = 500
                                                    // For xb is ID of button
                                                    // For time holds day:hour:minute psuedo codes see dhmAccess, with MSG == 1


  unsigned int varReading[maxVars];           // Same format as reading, but for internal variables (indexed by 7 bit variable number - maskVar)

};

class EVALHELPERS {
public:
	// Methods
	EVALHELPERS();													
	~EVALHELPERS();												
  
	byte getCalcIdx (char *calcTypeChar);
	void getCalcChar(byte calcTypeIdx, char *calcTypeChar);
protected:

};
	
#endif
*/