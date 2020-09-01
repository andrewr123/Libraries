 /*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
    
    Contains the HA_device library to manage the directory of devices (sensors or actors)

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


#include "Arduino.h"
#include "wakeup.h"
#include "HA_switcher.h"
#include "HA_device_bases.h"
#include "HA_channels.h"
#include "HA_root.h"
#include "HA_syslog.h"

/*
// *********** Globals

const char REGIONCODES[NUMREGIONCODES + 1] = { 'G', 'D', 'S', 'E', 'K', 'B', '/0' };          // Gt Hall, Dining, Study, External, Kitchen, Basement, null termination
const char *DEVICETYPES[NUM_DEV_TYPES + NUM_VAR_TYPES + NUM_ZONE_TYPES + NUM_OBJ_TYPES] = { "xT", "xF", "xH", "xL", "xM", "xP", "xR", "xO", "p", "P", "D", "L", "R", "vB", "vI", "vR", "Z", "Ti", "HB" };  
								// Sensors:  Touch, fire, heat, luminance, motion, presence, RFID, open
								// Actors: 5a power, 13A power, lock (was 'B'), light, relay
*/
// *********** BASE CLASSES ****************

// ***********  HA_device  *****************
// 
// Base class for all devices


void HA_device::put(byte type, unsigned int value) { 
  byte deviceMapIdx = type & MASK_DEVICEMAPIDX;
  unsigned int mask;
  byte offset;
  
  if (type == VAL_DEVIDX) {
	  _devNum = value;
	  return;
  }
  
  switch (type) {
    case VAL_REGION:     		mask = MASK_REGION; offset = OFFSET_REGION; value = strchr(REGIONCODES, (char)value) - REGIONCODES; break;   
    case VAL_ZONE:       		mask = MASK_ZONE; offset = OFFSET_ZONE; value -= 1; break;
    case VAL_REGION_ZONE:		mask = MASK_REGION_ZONE; offset = OFFSET_REGION_ZONE; break;
    case VAL_LOCATION:   		mask = MASK_LOCATION; offset = OFFSET_LOCATION; value -= 1; break;
    case VAL_DEVNUM:     		mask = MASK_DEVICENUM; offset = OFFSET_DEVICENUM; break;
    case VAL_STATUS:     		mask = MASK_STATUS; offset = OFFSET_STATUS; break;
    case VAL_HANDLER:    		mask = MASK_HANDLER; offset = OFFSET_HANDLER; break;
    case VAL_PIN:        		mask = MASK_PIN; offset = OFFSET_PIN; break;
    case VAL_CHANNEL:    		mask = MASK_CHAN; offset = OFFSET_CHAN; break;
    case VAL_STATE:      		mask = MASK_STATE; offset = OFFSET_STATE; break;
  }
    
	_deviceMap[deviceMapIdx] &= ~mask;                          // Clear
  _deviceMap[deviceMapIdx] |= ((value << offset) & mask);     // Store 
}

unsigned int HA_device::get(byte type) { 
  byte deviceMapIdx = type & MASK_DEVICEMAPIDX;
  unsigned int mask;
  byte offset;
  
	if (type == VAL_DEVIDX) return _devNum;
  
  switch (type) {
    case VAL_REGION:     		return REGIONCODES[(_deviceMap[deviceMapIdx] & MASK_REGION) >> OFFSET_REGION]; break;
    case VAL_ZONE:       		return ((_deviceMap[deviceMapIdx] & MASK_ZONE) >> OFFSET_ZONE) + 1; break;
		case VAL_REGION_ZONE:		mask = MASK_REGION_ZONE; offset = OFFSET_REGION_ZONE; break;
    case VAL_LOCATION:   		return ((_deviceMap[deviceMapIdx] & MASK_LOCATION) >> OFFSET_LOCATION) + 1; break;
    case VAL_DEVNUM:     		mask = MASK_DEVICENUM; offset = OFFSET_DEVICENUM; break;
    case VAL_STATUS:     		mask = MASK_STATUS; offset = OFFSET_STATUS; break;
    case VAL_HANDLER:    		mask = MASK_HANDLER; offset = OFFSET_HANDLER; break;
    case VAL_PIN:        		mask = MASK_PIN; offset = OFFSET_PIN; break;
    case VAL_CHANNEL:    		mask = MASK_CHAN; offset = OFFSET_CHAN; break;
    case VAL_STATE:      		mask = MASK_STATE; offset = OFFSET_STATE; break;
    default:						 		return 0;
  }
  
  return (_deviceMap[deviceMapIdx] & mask) >> offset;
}


byte HA_device::putRef(char *device) { // Load device chars in form an.nn.a[1-2]n, and return device type
  byte devType;

  if (device[0] == 'X') Serial.println("Null char");
  else { 
    // Convert region ref
    put(VAL_REGION, device[0]);
    
    // Convert zone number
    put(VAL_ZONE, atoi(device+1));  
    
    // Convert location
    put(VAL_LOCATION, atoi(device+3));
    
    devType = getDevTypeIdx (device + 6);
    
    // Get device number (if specified)
		switch (devType) {
			case DEV_TYPE_TOUCH: 
			case DEV_TYPE_FIRE: 
			case DEV_TYPE_HEAT: 
			case DEV_TYPE_LUMINANCE: 
			case DEV_TYPE_MOTION: 
			case DEV_TYPE_PRESENCE: 
			case DEV_TYPE_RFID: 
				if (isalnum(*(device + 8))) put(VAL_DEVNUM, atoi(device+8)); else put(VAL_DEVNUM, 0); break;	
			case DEV_TYPE_OPEN: 				
				if (isalnum(*(device + 8))) put(VAL_DEVNUM, atoi(device+8) - 1); break;				// xOn stored as n-1
			case DEV_TYPE_5APWR: 
			case DEV_TYPE_LOCK: 
			case DEV_TYPE_LIGHT: 
			case DEV_TYPE_RELAY: 
			case DEV_TYPE_13APWR: 
				if (isalnum(*(device + 7))) put(VAL_DEVNUM, atoi(device+7)); else put(VAL_DEVNUM, 0); break;	
		}    
  }
  
  return devType;
}  

void HA_device::getRef(char *device, byte devType) { // Convert bitstring device to chars and return in form an.nn.aaa
  // Get region ref
  device[0] = (char)get(VAL_REGION);      
  
  // Get zone number
  device[1] = get(VAL_ZONE) + 0x30;        // ASCII '0' is 0x30
  
  device[2] = '.';
  
  // Get location
  byte bitCode = get(VAL_LOCATION);
  device[3] = bitCode / 10 + 0x30;
  device[4] = bitCode % 10 + 0x30;
  
  device[5] = '.';
  
  // Get device type
  strcpy(device + 6, DEVICETYPES[devType]);
  
  // Add device number if needed (if zero then don't use, unless xO)
  byte devNum = get(VAL_DEVNUM);
  switch (devType) {
		case DEV_TYPE_TOUCH: 
		case DEV_TYPE_FIRE: 
		case DEV_TYPE_HEAT: 
		case DEV_TYPE_LUMINANCE: 
		case DEV_TYPE_MOTION: 
		case DEV_TYPE_PRESENCE: 
		case DEV_TYPE_RFID: 
			if (devNum > 0) { device[8] = devNum + 0x30; device[9] = '\0'; } 	// 0x30 == '0'
			else device[8] = '\0';
			break;					
		case DEV_TYPE_OPEN: 				
			device[8] = devNum + 0x31; device[9] = '\0'; break;
		case DEV_TYPE_5APWR: 
		case DEV_TYPE_LOCK: 
		case DEV_TYPE_LIGHT: 
		case DEV_TYPE_RELAY: 
		case DEV_TYPE_13APWR:  		
			if (devNum > 0) { device[7] = devNum + 0x30; device[8] = '\0'; } 	// 0x30 == '0'
			else device[7] = '\0';
			break;
	}
}  

void HA_device::getSnapshot(byte devType) { 
  Serial.print("Status = ");
  Serial.print(get(VAL_STATUS));
  
  Serial.print(", handler = ");
  Serial.print(get(VAL_HANDLER));
  
  Serial.print(", pin = ");
  Serial.print(get(VAL_PIN));
  
  Serial.print(", channel = ");
  Serial.print(get(VAL_CHANNEL));
  
  Serial.print(", state = ");
  Serial.println(get(VAL_STATE));  
}

/*
byte getDevTypeIdx (char *device) {
	  for (int i = 0; i < NUM_DEV_TYPES; i++) {
      if (strncmp(device, DEVICETYPES[i], strlen(DEVICETYPES[i])) == 0) return i;
    }
    return 0;
}

void getDevTypeChar (byte entIdx, char *entChar) {			// Returns device type, var type or zone type char string
	strncpy (entChar, DEVICETYPES[entIdx], 3);
}
*/

// ********************** HA_devBit ***********************
//
// Base class for all on/off devices (some sensors and all actors)

void HA_devBit::put(byte valType, unsigned int val) {
	switch (valType) {
		case VAL_PUSH: {
			char oldSREG = SREG;                               
		  cli();                                    // Disable interrupts whilst accessing map
		    
		  unsigned int temp = HA_device::get(VAL_STATE);
			
			temp &= ~(MASK_CURR | MASK_PREV);						// Clear the current and previous state fields
			
			temp |= get(VAL_CURR) << OFFSET_PREV;					// Move current to previous
			temp |= MASK_CURR & (val << OFFSET_CURR);		// Load latest to current
			
			HA_device::put(VAL_STATE, temp);											// Update
			
			
			SREG = oldSREG;														// Restore status register
			break;
		}
		default:					HA_device::put(valType, val);
	}
}

unsigned int HA_devBit::get(byte valType) {
	switch (valType) {
		case VAL_CURR:		return (HA_device::get(VAL_STATE) & MASK_CURR) >> OFFSET_CURR;
		case VAL_PREV:		return (HA_device::get(VAL_STATE) & MASK_PREV) >> OFFSET_PREV; 
		default:					return HA_device::get(valType); 
	}
}

void HA_devBit::put(byte valType, void *valPtr) {
	put(valType, *(unsigned int*)valPtr);
}

void HA_devBit::get(byte valType, void *valPtr) {
	*(unsigned int*)valPtr = get(valType);
}

void HA_devBit::readDev() {
	byte pin = get(VAL_PIN);
	byte handler = get(VAL_HANDLER);
	byte status = get(VAL_STATUS);
	
	switch (handler) {
		case 0: ;
	}
};

void HA_devBit::setDev(byte val) {
	byte pin = get(VAL_PIN);
	byte handler = get(VAL_HANDLER);
	
	switch (handler) {
		case 0: 
		  digitalWrite(pin, val);
		  put(VAL_PUSH, val);
  		put(VAL_STATUS, STATUS_STABLE);
  		break;
		default:		Serial.println("devBit::setDev handler OOB");
	}	
};

// ********************* HA_dev2Byte ******************
//
// Base class for sensors yielding a non-binary value, other than RFIDs

void HA_dev2Byte::put(byte valType, unsigned int val) {
	switch (valType) {
		case VAL_PUSH: {
			char oldSREG = SREG;                               
		  cli();                                    // Disable interrupts whilst accessing map
		    
		  _readingPrevious = _readingCurrent;				// Move current to previous
			_readingCurrent = val;										// Load latest to current
			
			SREG = oldSREG;														// Restore status register
			break;
		}
		default:					HA_device::put(valType, val);
	}
}

unsigned int HA_dev2Byte::get(byte valType) {
	switch (valType) {
		case VAL_CURR:			return _readingCurrent; break;
		case VAL_PREV:			return _readingPrevious; break;
		default:						return HA_device::get(valType); 
	}
}

void HA_dev2Byte::put(byte valType, void *valPtr) {
	put(valType, *(unsigned int*)valPtr);
}

void HA_dev2Byte::get(byte valType, void *valPtr) {
	*(unsigned int*)valPtr = get(valType);
}



// **************** HA_devMultiByte ****************
//
// Base class for devices whose values exceed an unsigned int - only RFIDs at present

HA_devMultiByte::HA_devMultiByte () {							// Instantiates class, but does not create the buffer
}

HA_devMultiByte::HA_devMultiByte (byte bufLen) {	// Instantiates class and creates buffer
	create(bufLen);
}

HA_devMultiByte::~HA_devMultiByte () {
	freeMem();
}

boolean HA_devMultiByte::create(byte bufLen) {		// Finds space for buffer and then initialises it
	byte *addr = (byte*)malloc(bufLen * 2);					// Get some space
	
	if (addr != NULL) {
		init(addr, bufLen);		// If got space then initialise it
		return true;
	}
	else {
		_bufLen = 0; 
		return false;
	}
}

void HA_devMultiByte::init(byte *addr, byte bufLen) {		// Initialise variables and buffer (space found using create() or by user)
	_bufLen = bufLen;
	_bufCurr = addr;			
	_bufPrev = _bufCurr + _bufLen;				// Position at end of current
	memset(addr, 0, _bufLen * 2); 
}

void HA_devMultiByte::freeMem() {							// Called either by user (if user called create()), or from deconstructor
	free(_bufCurr);															// Pointer to originally malloc'ed memory
}

byte HA_devMultiByte::bufLen() {
	return _bufLen;
}

void HA_devMultiByte::put(byte valType, unsigned int val) {
	HA_device::put(valType, val);
}

unsigned int HA_devMultiByte::get(byte valType) {
	return HA_device::get(valType); 
}

void HA_devMultiByte::put(byte valType, byte *valBuf) {   // <<<<<<<<<<<<<< Change to void*
	switch (valType) {
		case VAL_PUSH: {
			char oldSREG = SREG;                               
		  cli();
		  
		  // Swap pointers to move curr to prev, and then fill curr with new valBufue
		  byte *temp = _bufPrev;
		  _bufPrev = _bufCurr;
		  _bufCurr = temp;
		  for (int i = 0; i < _bufLen; i++) _bufCurr[i] = valBuf[i];
		  
			SREG = oldSREG;														// Restore status register
			break;
		}
		default:						HA_device::put(valType, *valBuf);
	}
}

void HA_devMultiByte::get(byte valType, byte *valBuf) {
	switch (valType) {
		case VAL_CURR:		for (int i = 0; i < _bufLen; i++) valBuf[i] = _bufCurr[i]; break;
		case VAL_PREV:		for (int i = 0; i < _bufLen; i++) valBuf[i] = _bufPrev[i]; break;
		default:					*valBuf = HA_device::get(valType);
	}
}

byte *HA_devMultiByte::getBufPtr(byte valType) {
	switch (valType) {
		case VAL_CURR:		return _bufCurr;
		case VAL_PREV:		return _bufPrev;
	}
}

void HA_devMultiByte::putElem(byte valType, byte elemIdx, byte elemVal) {
	switch (valType) {
		case VAL_CURR:		_bufCurr[elemIdx] = elemVal; break;
		case VAL_PREV:		_bufPrev[elemIdx] = elemVal; break; 
	}
}

byte HA_devMultiByte::getElem(byte valType, byte elemIdx) {
	switch (valType) {
		case VAL_CURR:		return _bufCurr[elemIdx]; break;
		case VAL_PREV:		return _bufPrev[elemIdx]; break; 
	}
}

// ************ HA_sensAnalog
//
// Base class for reading analog devices - yields value in 1/100th units

void HA_sensAnalog::initDev(byte devNum, byte channel, byte pin, byte handler) {
	put(VAL_PIN, pin);
	put(VAL_CHANNEL, channel);
	put(VAL_DEVIDX, devNum);
	
	HA_channel *chanPtr = root.getChanObj(channel);		// Open up the channel (including direct read (== CH0)) and check max pins
	if (pin > chanPtr->get(VAL_CHAN_MAX_PIN)) { 	
		Serial.println("Max pin exceeded");		
		put(VAL_STATUS, STATUS_UNAVAILABLE);
		return;
	}
	pinMode((channel == CH0) ? pin : chanPtr->get(VAL_CHAN_IO_PIN), INPUT);
	
	put(VAL_STATUS, STATUS_READY);
}


void HA_sensAnalog::readDev(byte devType) {
	byte pin = get(VAL_PIN), dataPin;
	byte channel = get(VAL_CHANNEL);
	byte status = get(VAL_STATUS);
	
	HA_channel *chanPtr = root.getChanObj(channel);
	long analogValue;
	
	if (status != STATUS_READY && status != STATUS_STABLE) {Serial.println("Bad status"); return;}
	
	switch (chanPtr->get(VAL_CHAN_ACCESS)) {
		case CHAN_ACCESS_DIRECT:
		  analogValue = analogRead(pin);
  		break;
		case CHAN_ACCESS_MUX: 						// Enable pin and read
			if (!(chanPtr->lock())) {			// Test if can gain exclusive use of channel
			 	Serial.println("No lock avail");		
	  		put(VAL_STATUS, STATUS_UNAVAILABLE);
 				return;
	  	}

	  	// Have got the lock; enable pin & read
	  	chanPtr->enablePin(pin);
	  	dataPin = chanPtr->get(VAL_CHAN_IO_PIN);
	  	digitalWrite(dataPin, LOW);						// Clear any pullup resistors
	  	pinMode(dataPin, INPUT);							// Just in case other channel pins used for output
		  analogValue = analogRead(dataPin);
  		
  		chanPtr->unlock();			// Release lock
  		break;
    default: 
    	Serial.print("Unrecognised chanType");
    	Serial.println(chanPtr->get(VAL_CHAN_ACCESS), DEC);
	}
	
	analogValue = analogValue * ARDUINO_VOLTAGE * 100 / ANALOG_RANGE;
	
	put(VAL_PUSH, (unsigned int)analogValue, devType);
  put(VAL_STATUS, STATUS_STABLE);
}

void HA_sensAnalog::put(byte valType, unsigned int val, byte devType) {  
	SAVE_CONTEXT("HA_sA::put")
	
	switch (valType) {
		case VAL_ON_EVENT: 	_onEvent = val; break;
		case VAL_PUSH:
			HA_dev2Byte::put(valType, val);
			if (get(VAL_CURR) != get(VAL_PREV)) changeList.put(devType, HA_device::get(VAL_DEVIDX), val);
			break;
		default:						HA_dev2Byte::put(valType, val);
	}
	RESTORE_CONTEXT
}

unsigned int HA_sensAnalog::get(byte valType) {
	switch (valType) {
		case VAL_ON_EVENT:	return _onEvent; break;
		default:						return HA_dev2Byte::get(valType); 
	}
}


// ************ HA_sensDigital
//
// Base class for reading digital devices - on/off

void HA_sensDigital::initDev(byte devNum, byte channel, byte pin, byte handler) {
	put(VAL_PIN, pin);
	put(VAL_CHANNEL, channel);
	put(VAL_DEVIDX, devNum);
	
	HA_channel *chanPtr = root.getChanObj(channel);		// Open up the channel (including direct read (== CH0)) and check max pins
	if (pin > chanPtr->get(VAL_CHAN_MAX_PIN)) { 	
		Serial.println("Max pin exceeded");		
		put(VAL_STATUS, STATUS_UNAVAILABLE);
		return;
	}
	pinMode((channel == CH0) ? pin : chanPtr->get(VAL_CHAN_IO_PIN), INPUT);
	
	put(VAL_STATUS, STATUS_READY);
}

void HA_sensDigital::readDev() {
	byte pin = get(VAL_PIN), dataPin;
	byte channel = get(VAL_CHANNEL);
	byte status = get(VAL_STATUS);
	
	HA_channel *chanPtr = root.getChanObj(channel);
	byte digitalValue;
	
	if (status != STATUS_READY && status != STATUS_STABLE) {Serial.println("Bad status"); return;}
		
	switch (chanPtr->get(VAL_CHAN_ACCESS)) {
		case CHAN_ACCESS_DIRECT:
		  digitalValue = digitalRead(pin);
  		break;
		case CHAN_ACCESS_MUX: 						// Enable pin and read
			if (!(chanPtr->lock())) {			// Test if can gain exclusive use of channel
			 	Serial.println("No lock avail");		
	  		put(VAL_STATUS, STATUS_UNAVAILABLE);
 				return;
	  	}

	  	// Have got the lock; enable pin & read
	  	chanPtr->enablePin(pin);
	  	dataPin = chanPtr->get(VAL_CHAN_IO_PIN);
	  	digitalWrite(dataPin, LOW);						// Clear any pullup resistors
	  	pinMode(dataPin, INPUT);							// Just in case other channel pins used for output
		  digitalValue = digitalRead(dataPin);
  		
  		chanPtr->unlock();			// Release lock
  		break;
    default: 
    	Serial.print("Unrecognised chanType ");
    	Serial.println(chanPtr->get(VAL_CHAN_ACCESS), DEC);
	}
	
	put(VAL_PUSH, (unsigned int)digitalValue);
  put(VAL_STATUS, STATUS_STABLE);
}

void HA_sensDigital::put(byte valType, unsigned int val) {
	switch (valType) {
		case VAL_ON_EVENT: 	_onEvent = val; break;
		default:						HA_devBit::put(valType, val);
	}
}

unsigned int HA_sensDigital::get(byte valType) {
	switch (valType) {
		case VAL_ON_EVENT:	return _onEvent; break;
		default:						return HA_devBit::get(valType); 
	}
}

// ************ HA_actDigital
//
// Base class for digital actors - on/off

void HA_actDigital::initDev(byte devNum, byte channel, byte pin, byte handler) {
	put(VAL_PIN, pin);
	put(VAL_CHANNEL, channel);
	put(VAL_DEVIDX, devNum);
	
	HA_channel *chanPtr = root.getChanObj(channel);		// Open up the channel (including direct read (== CH0)) and check max pins
	if (pin > chanPtr->get(VAL_CHAN_MAX_PIN)) { 	
		Serial.println("Max pin exceeded");		
		put(VAL_STATUS, STATUS_UNAVAILABLE);
		return;
	}
	if (chanPtr->get(VAL_CHAN_ACCESS) == CHAN_ACCESS_DIRECT) pinMode((channel == CH0) ? pin : chanPtr->get(VAL_CHAN_IO_PIN), OUTPUT); 
	
	put(VAL_STATUS, STATUS_READY);
}

void HA_actDigital::setDev(byte dval) {
	byte pin = get(VAL_PIN), dataPin;
	byte channel = get(VAL_CHANNEL);
	byte status = get(VAL_STATUS);
	
	HA_channel *chanPtr = root.getChanObj(channel);
	
	if (status != STATUS_READY && status != STATUS_STABLE) {Serial.println("Bad status"); return;}
	
	switch (chanPtr->get(VAL_CHAN_ACCESS)) {
		case CHAN_ACCESS_DIRECT:
		  digitalWrite(pin, dval);
  		break;
		case CHAN_ACCESS_MUX: 					
			if (!(chanPtr->digitalWrite(pin, dval))) {
				put(VAL_STATUS, STATUS_UNAVAILABLE);
				return;
			}
			break;  	
		case CHAN_ACCESS_POWER:
			chanPtr->digitalWrite(pin, dval);
			break;
    default: 
    	Serial.print("Unrecognised chanType ");
    	Serial.println(chanPtr->get(VAL_CHAN_ACCESS), DEC);
	}
	
	put(VAL_PUSH, (unsigned int)dval);
  put(VAL_STATUS, STATUS_STABLE);
}