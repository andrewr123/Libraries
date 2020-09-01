 /*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
    
    Contains the HA_DeviceMap library to manage the directory of devices

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

#include "WProgram.h"
#include "HomeAutom.h"




// ************* MAPHELPER *******************

// ************ GLOBALS ********************

const static byte numRegionCodes = 7;
const static byte numDeviceTypes = 14;
const static byte numActorTypes = 8;
const static char regionCodes[numRegionCodes] = { 'G', 'D', 'S', 'E', 'K', 'B' };          // Gt Hall, Dining, Study, External, Kitchen, Basement.  (X1.00.Z translates to 0x00 and is a NULL device)
const static char *deviceTypes[numDeviceTypes] = {  "Z", "xT", "xF", "xH", "xL", "xM", "xP", "xR", "xo", "p", "D", "L", "R", "P" };  
								// Sensors:  dummy (to avoid interrupting zero ref as G1.00.xT), Touch, fire, heat, luminance, motion, presence, RFID, open
								// Actors: 5a power, door lock (was 'B'), light, relay, power
const static unsigned int indDeviceType = (B00000001 * 256) + B11111110;		// 1 == sensor; 0 == actor (LSB == element 0)
const static unsigned int indSensorType = (B00000001 * 256) + B10011000;      // Flag for each sensor - 1 - device gives readings; 0 - device is on/off  (LSB == element 0)
const static unsigned int indTempSensor = (B00000000 * 256) + B00000010;		// 1 if temperature sensor
const static unsigned int indSlowSensor = (B00000000 * 256) + B00000010;		// 1 if sensor needs to be read in two passes


LOCATION::LOCATION(ZONE *zoneAddr) {
	: _location.parentZone(zoneAddr);
}

DEVICE::DEVICE(byte deviceType) {			// http://stackoverflow.com/questions/120876/c-superclass-constructor-calling-rules
	: _device.deviceType(deviceType);
}

SENSOR::SENSOR() {
	: DEVICE(devSensor);
}

ACTOR::ACTOR() {
	: DEVICE(devActor);
}

          
MAPHELPER::MAPHELPER() {};
MAPHELPER::~MAPHELPER() {};

// ******** Device map helper functions ******************

unsigned int MAPHELPER::get(byte deviceIdx, byte type) { 
  unsigned int result = access (deviceIdx, type, NULL, readFlag); return result; 
}

void MAPHELPER::put(byte deviceIdx, byte type, unsigned int value) { 
  access (deviceIdx, type, value, writeFlag); 
}
  
unsigned int MAPHELPER::access(byte deviceIdx, byte type, unsigned int value, int flag) {            // Get appropriate value out of deviceMap or varReading
 
  byte deviceMapIdx = type & maskDeviceMapIdx;
  unsigned int mask;
  int offset, i, temp1, temp2;
    
  if (deviceIdx & mask8BitMSB) {        // Variable
    if ((deviceIdx &= ~mask8BitMSB) >= maxVars) Serial.println("Var out of bounds");    
    switch (type) {
      case valRegion:     return 'V';
      case valArduino:    return arduinoMe;
      case valPin:        return pinNoOp;
      case valStatus:     return valStatusStable;
      case valCascade:    return false;
      case valPrev:
      case valAvg:  
      case valMax:
      case valMin:
      case valROC:
      case valCurr:    if (flag == readFlag) return varReading [deviceIdx]; else { varReading [deviceIdx] = value; break; }
      default:
        Serial.println("Unexpected var type");
        return 0;
    }
  }
  else {
    if (deviceIdx >= maxDevices) { Serial.print("Device "); Serial.print(deviceIdx); Serial.println(" out of bounds"); }
    switch (type) {
      case valRef:        mask = maskRef; offset = 0; break;
      case valRegion:     mask = maskRegion; offset = offsetRegion; break;
      case valZone:       mask = maskZone; offset = offsetZone; break; 
      case valLocation:   mask = maskLocation; offset = offsetLocation; break;
      case valSensor:     mask = maskSensor; offset = offsetSensor; break;
      case valType:       mask = maskDeviceType; offset = 0; break;
      case valArduino:    mask = maskArduino; offset = offsetArduino; break;
      case valPin:        mask = maskPin; offset = offsetPin; break;
      case valCascade:    mask = maskCascade; offset = offsetCascade; break;
      case valHandler:    mask = maskHandler; offset = offsetHandler; break;
      case valPollFreq:   mask = maskFreq; offset = 0; break;
      case valStatus:     mask = maskStatus; offset = offsetStatus; break;
//      case valStackMode:  mask = maskStackMode; offset = offsetStackMode; break;
      case valTOSIdx:     mask = maskTOSIdx; offset = offsetTOSIdx; break;
      case valStack:      mask = maskStack; offset = 0; break;
      case valCurr:    // GET - return latest reading.  PUT - Store reading on stack
        if (flag == readFlag) return stackGet (deviceIdx, 0); else { stackPush (deviceIdx, value); return NULL; }
      case valPrev: 
        return stackGet (deviceIdx, 1);      // Only GET - return previous reading
      case valAvg:  
        temp1 = 0;
        for (i = 0; i < 8; i++) temp1 += stackGet (deviceIdx, i);
        return temp1/8;
      case valMax:
        temp1 = stackGet (deviceIdx, 0);
        for (i = 1; i < 8; i++) if (temp2 = stackGet (deviceIdx, i) > temp1) temp1 = temp2;
        return temp1;
      case valMin:
        temp1 = stackGet (deviceIdx, 0);
        for (i = 1; i < 8; i++) if (temp2 = stackGet (deviceIdx, i) < temp1) temp1 = temp2;
        return temp1;
      case valROC:
        temp1 = 0;
        for (i = 1; i < 8; i++) temp1 += (stackGet (deviceIdx, i - 1) - (temp2 = stackGet (deviceIdx, i))) * 100 / temp2;
        return temp1/8/100;
      default: Serial.println("Unknown access type");
    }
    
    if (flag == readFlag) return (deviceMap[deviceMapIdx][deviceIdx] & mask ) >> offset;
    else {
      deviceMap[deviceMapIdx][deviceIdx] &= ~mask;                          // Clear
      deviceMap[deviceMapIdx][deviceIdx] |= ((value << offset) & mask);                // Store 
    }
  }
}


// Reading helper functions

byte MAPHELPER::stackMode(byte deviceIdx) {
	return get(deviceIdx, valSensor) ? ((indSensorStackMode & (1 << get(deviceIdx, valType))) != 0) : 0;
}

unsigned int MAPHELPER::stackGet (byte deviceIdx, unsigned int element) { 
  unsigned int result = stackAccess (deviceIdx, element, NULL, readFlag); 
  return result;
}
  

  
void MAPHELPER::stackPush (byte deviceIdx, unsigned int value) { 
  stackAccess (deviceIdx, 0, value, writeFlag); 
}

  

unsigned int MAPHELPER::stackAccess (byte deviceIdx, unsigned int element, unsigned int value, byte flag) {          // Get element off or add element to stack (0 = TOS). NB: stacksize implied here as 8
  unsigned int result;

  byte stack = get(deviceIdx, valStack);    // Holds 8 on/off readings if StackMode == 0, else pointer into readingHistory
  
  if (stackMode(deviceIdx)) {
    unsigned int index = stack * stackSize;
    unsigned int offset = (get(deviceIdx, valTOSIdx) + element) % stackSize;
    
    if (flag == readFlag) result = readingHistory [index + offset];
    else {
       unsigned int newOffset = (offset % stackSize != 0) ? offset - 1 : offset + 7;    // If not at top of stack (non-zero) then decrement, else rotate round    
       readingHistory [index + newOffset] = value;      // Store reading (overwrites oldest)  
       put (deviceIdx, valTOSIdx, newOffset);
    }
  }
  else if (flag == readFlag) result = ((stack << element) >> 7) & maskLSB; else put (deviceIdx, valStack, (stack >> 1) | ((value & 1) << 7));

  return result;
}



 


unsigned int  MAPHELPER::convertRefToBit (char *device) { // convert device chars in form an.nn.aaa to bitstring
  unsigned int bitstring = 0;

  if (device[0] == 'X') Serial.println("Null char");
  else { 
    // Convert region ref.  Lots of casts to avoid "error: invalid operands of types 'unsigned int' and 'void*' to binary 'operator|'"
    bitstring |= (unsigned int)(memchr(regionCodes, device[0], numRegionCodes) - (void *)regionCodes ) << offsetRegion;    // 3 bit << 13 (MSB)
    
    // Convert zone number (reduce by 1) - max 8
    bitstring |= (atoi(device+1) - 1) << offsetZone;    // 3 bits << 10
    
    // Convert location - max 31 (0 reserved for whole zone).
    bitstring |= atoi(device+3) << offsetLocation;  // 5 bits << 5

    // Convert device type    
    char *deviceType = device + 6;			// Point to device type substring
    for (int i = 0; i < numDeviceTypes; i++) {
      if (strcmp(deviceType, deviceTypes[i]) == 0) {
        bitstring |= i << offsetDeviceType;
        break;
      }
    }
  
  return bitstring;
}  

void MAPHELPER::convertRefToChar (unsigned int bitstring, char *device) { // Convert bitstring device to chars and return in form an.nn.aaa
  if (bitstring) {
    // Get region ref
    device[0] = regionCodes[(bitstring & maskRegion) >> offsetRegion];      // 3-bit region is index into code string
    
    // Get zone number
    device[1] = ((bitstring & maskZone) >> offsetZone) + 0x31;        // ASCII '1' is 0x31
    
    device[2] = '.';
    
    // Get location
    byte bitCode = (bitstring & maskLocation) >> offsetLocation;
    device[3] = bitCode / 10 + 0x30;
    device[4] = bitCode % 10 + 0x30;
    
    device[5] = '.';
    
    // Get device type
    strcpy(device + 6, deviceTypes[bitstring & maskDeviceType];
  }
  else { device[0] = 'X'; device[1] = '\0'; Serial.println("Null bits"); }        // NULL device
}  

boolean MAPHELPER::isTempSensor (byte deviceIdx) {
	return get(deviceIdx, valRef) | indTempSensor;
}

boolean MAPHELPER::isSlowSensor (byte deviceIdx) { 
	return get(deviceIdx, valRef) | indSlowSensor;
}


// ************* EVALHELPERS *******************

const static byte numCalcs = 14;
const static char *calcTypes[numCalcs] = { "ListE", "ListM", "!", "CURR", "PREV", "Avg", "Max", "Min", "ROfC", "Year", "Month", "Day", "Hour", "Minute"};  
          // List of evals; list of devices/variables; A qualifiers: not, current/latest, previous, average, max, min, rate of change; A replacements: now(year, month, day, hour, minute).  B is always current value (except in lists)



EVALHELPERS::EVALHELPERS() {};
EVALHELPERS::~EVALHELPERS() {};


byte EVALHELPERS::getCalcIdx(char *calcTypeChar) { 
  for (int j=0; j < numCalcs; j++) if (strstr(calcTypes[j], calcTypeChar)) return j; 
  Serial.println("Calc NF");
}

void EVALHELPERS::getCalcChar(byte calcTypeIdx, char *calcTypeChar) { 
  if (calcTypeIdx > numCalcs) Serial.println("Calc idx OF"); else strcpy (calcTypeChar, calcTypes[calcTypeIdx]);
}
*/