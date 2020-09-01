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

#include "HA_devHeat.h"
#include "Wakeup.h"
#include "HA_root.h"
#include "HA_switcher.h"




// **************** HA_devHeat ***************
//
// Single non-parasitic Dallas one-wire device - output is temp in C * 10
//
// Initialisation - set up OneWire pin and check device characteristics - if OK then set READY, else UNAVAILABLE

void HA_devHeat::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_HEAT);
}

void HA_devHeat::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_HEAT);
}


void HA_devHeat::initDev(byte devNum, byte channel, byte pin, byte handler) {
	put(VAL_PIN, pin);					
	put(VAL_CHANNEL, channel);
	put(VAL_HANDLER, handler);
	put(VAL_DEVIDX, devNum);
	
	HA_channel *chanPtr = root.getChanObj(channel);
	
	put(VAL_STATUS, STATUS_READY);														// Assume success

	switch (chanPtr->get(VAL_CHAN_ACCESS)) {
		case CHAN_ACCESS_DIRECT:
			_oneWire.init(pin);
			break;
		case CHAN_ACCESS_MUX:			// Get exclusive use of channel and initialise device
	  	if (pin <= (chanPtr->get(VAL_CHAN_MAX_PIN)) && chanPtr->lock() && chanPtr->enablePin(pin) && _oneWire.init(chanPtr->get(VAL_CHAN_IO_PIN))) break;		// Exit with channel locked
	  	else chanPtr->unlock();					// Unlock and fall through
	  default:
	  	// If got to here then a problem
			Serial.println("initDev: channel fault"); 
			put(VAL_STATUS, STATUS_UNAVAILABLE);
			return;
	}
  
  // Reset the bus and get the address of the first (& only) device to determine ROM family]
  _buffer[ROM_CRC] = 0;
	_oneWire.reset();
  _oneWire.readROM(_buffer);
  
  if (_oneWire.crc8(_buffer, ROM_CRC) != _buffer[ROM_CRC]) {
	  Serial.println("Invalid CRC on init");
	  put(VAL_STATUS, STATUS_UNAVAILABLE);
	  if (chanPtr->get(VAL_CHAN_ACCESS) == CHAN_ACCESS_MUX) chanPtr->unlock();			// Release lock if previously used
	  return;
  }
  
  _romFamily = _buffer[ROM_FAMILY];				// Hereafter, _buffer used to hold scratchpad, not address (saves Heap space)
  
  // Set the resolution.  Have to write all three bytes of scratchpad, but as alarm function not used first two can be random
  switch (_romFamily) {
	  case DS18S20: 	
	  	put(VAL_STATUS, STATUS_READY);
	  	break;						// 9 bit resolution only
		case DS18B20:
		case DS1822:
			switch (TEMPERATURE_PRECISION) {					
      	case 12:		_buffer[CONFIGURATION] = TEMP_12_BIT; break;
      	case 11:		_buffer[CONFIGURATION] = TEMP_11_BIT; break;
      	case 10:		_buffer[CONFIGURATION] = TEMP_10_BIT; break;
      	case 9:			
      	default:		_buffer[CONFIGURATION] = TEMP_9_BIT; break;
      }
			_oneWire.reset();
			_oneWire.skip();												// Avoids the need for sending the address (only on single device buses)
  		_oneWire.write(WRITESCRATCH);
  		_oneWire.write(_buffer[HIGH_ALARM_TEMP]);	
  		_oneWire.write(_buffer[LOW_ALARM_TEMP]);
  		_oneWire.write(_buffer[CONFIGURATION]);			
			put(VAL_STATUS, STATUS_READY);
  		break;
  	default:
  		Serial.println("Unrecognised family");
  		put(VAL_STATUS, STATUS_UNAVAILABLE);
  		return;
	}
	
	if (chanPtr->get(VAL_CHAN_ACCESS) == CHAN_ACCESS_MUX) chanPtr->unlock();			// Release lock if previously used
}


void HA_devHeat::resetDev() {
	initDev(get(VAL_CHANNEL), get(VAL_PIN), get(VAL_HANDLER));
}


//	Read device - initiate read, activate wakeup of getReading, then transfer control back to main program (which needs to test for status before taking reading)

void HA_devHeat::readDev() {
	byte pin = get(VAL_PIN);
	byte channel = get(VAL_CHANNEL);
	byte status = get(VAL_STATUS);
	int contextNum;
	unsigned int tConv;  
	HA_channel *chanPtr = root.getChanObj(channel);		
	HA_devHeatMemPtr fPtr = &HA_devHeat::getReading;					
	
	if (status != STATUS_READY && status != STATUS_STABLE) {Serial.println("Bad status1"); return;}
	
	switch (chanPtr->get(VAL_CHAN_ACCESS)) {
		case CHAN_ACCESS_MUX: 					// Enable pin and fall through 
			if (!(chanPtr->lock())) {			// Test if can gain exclusive use of channel
			 	Serial.println("No lock avail");		
	  		put(VAL_STATUS, STATUS_UNAVAILABLE);
 				return;
	  	}
	  	else chanPtr->enablePin(pin);   // Have got the lock; enable pin & fall through
		case CHAN_ACCESS_DIRECT:				// Initiate read
			// Start temperature conversion
			_oneWire.reset();
  		_oneWire.skip();								// Avoids the need for sending the address (only on single device buses)
  		_oneWire.write(STARTCONVO);  		// Start temperature conversion
  		
  		if (chanPtr->get(VAL_CHAN_ACCESS) == CHAN_ACCESS_MUX) chanPtr->unlock();			// Release lock if previously used
  		
  		// Work out how long temperature conversion will take
  		if (_romFamily == DS18S20) tConv = T_CONV_DS18S20;
  		else switch (TEMPERATURE_PRECISION) {
	  		case 12: tConv = T_CONV_12_BIT; break;
				case 11: tConv = T_CONV_11_BIT; break;
				case 10: tConv = T_CONV_10_BIT; break;
				case 9: tConv = T_CONV_9_BIT; break;
			}

			// Go to sleep whilst temp conversion takes place
	
		  // Set 'this' as the object instance and getReading as the member function 
			if ((contextNum = saveContext(this, fPtr)) < 0) {
			 	Serial.println("out of stack");
			  put(VAL_STATUS, STATUS_UNAVAILABLE);
			 	return;
		 	}
		 	
	  	if (wakeup.wakeMeAfter(switcher, tConv, (void*)contextNum, TREAT_AS_NORMAL)) {  	// Queue wakeup call
	  		put(VAL_STATUS, STATUS_PENDING);																						// Success; set flag to indicate waiting
  		}
  		else {
	  		Serial.println("Queue full");
	  		put(VAL_STATUS, STATUS_UNAVAILABLE);
  		}
		
  		break;
    default: Serial.println("Unrecognised chanType");
	}
}

// ******** Follow-up complement to readDev - woken after a sleep while Dallas device converts temp

void HA_devHeat::getReading(byte arg) {
	byte pin = get(VAL_PIN);
	byte channel = get(VAL_CHANNEL);
	HA_channel *chanPtr = root.getChanObj(channel);
	unsigned int reading;
	float celsius;
	
	if (get(VAL_STATUS) != STATUS_PENDING) {Serial.println("Bad status2"); return;}
	
	switch (chanPtr->get(VAL_CHAN_ACCESS)) {
		case CHAN_ACCESS_MUX: 					// Enable pin and fall through
			if (!chanPtr->lock()) {			// Test if can gain exclusive use of channel	
				Serial.println("No lock avail");		
	  		put(VAL_STATUS, STATUS_UNAVAILABLE);
 				return;
		  }
	  	else chanPtr->enablePin(pin);   // Have got the lock; enable pin & fall through
		case CHAN_ACCESS_DIRECT:				// Initiate read 	
			// Read the temperature
			_oneWire.reset();
  		_oneWire.skip();
  		_oneWire.write(READSCRATCH);
  		
  		if (chanPtr->get(VAL_CHAN_ACCESS) == CHAN_ACCESS_MUX) chanPtr->unlock();			// Release lock if previously used
  		
  		for (int i = 0; i < 9; i++) _buffer[i] = _oneWire.read();
			if (_oneWire.crc8(_buffer, SCRATCHPAD_CRC) != _buffer[SCRATCHPAD_CRC]) {
	  		Serial.println("Invalid S CRC");
	  		put(VAL_STATUS, STATUS_UNAVAILABLE);
	  		return;
  		}
  		
  		// Load the temperature to single variable and add extra resolution if needed
  		reading = (((unsigned int)_buffer[TEMP_MSB]) << 8) | _buffer[TEMP_LSB];  		
		  if (_romFamily == DS18S20) {			// Fixed 9 bit resolution expandable using 'extended resolution temperature' algorithm
			  	reading = (reading << 3) & 0xFFF0;				// Shift to same position as DS18B20 and truncate 0.5C bit
      		reading = reading + 12 - _buffer[COUNT_REMAIN];		// Simplified version of Dallas algorithm 
		  }
		  
		  // Convert reading to signed 1/10ths of centigrade
      celsius = (float)reading / 16.0;
      reading = (int)(celsius * 10);
      
      put(VAL_PUSH, reading);							// Save the reading
  		put(VAL_STATUS, STATUS_STABLE);			// Declare it available
  		
      break;
    default: Serial.println("Unrecognised chanType");
	}
}
