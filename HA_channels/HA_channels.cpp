 /*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
    
    Contains the HA_channel class used to mediate access to end devices - typically offering mux or multi-interrupt handling

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
#include "HA_channels.h"
#include "HA_root.h"
#include "HA_switcher.h"
#include "SPI.h"


boolean HA_channel::initChan(byte protocolType, byte maxPins, byte ioPin) {
	if (maxPins > MAX_PINS) {
		Serial.println("pin OOB1");
		return false;
	}

	_chanType = protocolType;
	_maxPin = maxPins - 1;
	_ioPin = ioPin;						// Usually physical pin, but if used for Serial then is serialPort number (0 - 3)
	_inUse = UNLOCKED;
	
	switch (get(VAL_CHAN_PROTOCOL)) {
		case CHAN_PROTOCOL_PIO:			break;								// Physical IO - no need to initialise
		case CHAN_PROTOCOL_SIO:		
			switch (_ioPin) {
				case 0: 	serialPtr = &Serial; break;
				case 1:		serialPtr = &Serial1; break;
				case 2:		serialPtr = &Serial2; break;				// TX pin 16; RX pin 17
				case 3:		serialPtr = &Serial3; break;
				default:	Serial.println("Bad ioPin"); return false;
			}
			serialPtr->begin(9600);
			break;
		case CHAN_PROTOCOL_SPI:			break;
		case CHAN_PROTOCOL_TWI:			break;
		default:										Serial.println("Bad protocol"); return false;
	}

	return true;
}

boolean HA_channel::initChanAccess(byte accessType, byte latchPin, byte dataPin, byte clockPin) {
	_chanType |= accessType;					// <<<<<<<<<<<<<<<< Not used?
	_latchPin = latchPin;
	_dataPin = dataPin;
	_clockPin = clockPin;
	
	switch (get(VAL_CHAN_ACCESS)) {
		case CHAN_ACCESS_DIRECT:		
			return true;																// Direct access; no need to enable
		case CHAN_ACCESS_MUX:				
			_numShifts = (_maxPin / 32) + 1;															//  Each SR controls 2 x 74HC4067, each of which handles 16 channels 
			
			// Set control ports as outputs
		  pinMode(_latchPin, OUTPUT);
		  pinMode(_dataPin, OUTPUT);  
		  pinMode(_clockPin, OUTPUT);
		  return true;
		case CHAN_ACCESS_POWER:	
			// Get some space for bitstring control structures
			pinStates = (BITSTRING*)malloc(sizeof(BITSTRING));				
			if (pinStates == NULL) return false;
			
			// Set up bitstring to hold pin states - round up to a whole shift register
			pinStates->create(((_maxPin / 8) + 1) * 8);
			pinStates->clearAll();
			
			_numShifts = (_maxPin / 8) + 1;															// Each SR has 8 pins
			
			// Set control ports as outputs
		  pinMode(_latchPin, OUTPUT);
		  pinMode(_dataPin, OUTPUT);  
		  pinMode(_clockPin, OUTPUT);
		  return true;
		default: 										Serial.println("Bad access"); return false;
	}
}

	
boolean HA_channel::initChanAlert(byte alertType, byte chanNum, byte intNum, byte intMode) {
	_chanType |= alertType;
	
	if (get(VAL_CHAN_ALERT) == CHAN_ALERT_INT) {
		chan_alert_int = (structINT*)malloc(sizeof(structINT));			// Get some space for data needed to call the appropriate device			
		if (chan_alert_int == NULL) return false;										// If no space then quit

		// Establish this channel as the one to invoke on interrupt intNum and attach the relevant ISR
		registerChanISR(intNum, chanNum, intMode);
		
		// Start the daemon to follow through on chanISR
		startDaemon();
		
	  return true;
	}
	else {
		Serial.println("Bad alert"); 
		return false;
	}

}

boolean HA_channel::initChanAlert(byte alertType, byte chanNum, byte intNum, byte intMode, byte resetPin, byte slaveSelectPin) {
	_chanType |= alertType;
	
	if (get(VAL_CHAN_ALERT) == CHAN_ALERT_SINT) {										// Up to 16 interrupts from single MCP23s17
		chan_alert_sint = (structSINT*)malloc(sizeof(structSINT));	// Get some space for data needed to interface to the MCP23S17 and to call the appropriate device(s)			
		if (chan_alert_sint == NULL) return false;									// If no space then quit		
		
		// Clear the interrupt ranges for this channel
		for (int i = 0; i < NUM_INTERRUPT_RANGES_SINT; i++) {
			chan_alert_sint->intRange[i].intPin = 0;
			chan_alert_sint->intRange[i].devType = DEV_TYPE_NULL;
		}
		
		// Clear the flag bits
		chan_alert_sint->flagBitsSINT = 0;
		
		// Initialise SPI comms - to do - improve handling of SPI interface in a multi-threading environment
	  SPI.begin();
	  SPI.setBitOrder(MSBFIRST);
	  SPI.setClockDivider(SPI_CLOCK_DIV2);		// 8MHz
	  
	  // Reset chip
	  pinMode(resetPin, OUTPUT);
	  ::digitalWrite(resetPin, LOW);
	  ::digitalWrite(resetPin, HIGH);   
	  
		// Setup MCP23S17
	  chan_alert_sint->MCP23SINT.beginInt(slaveSelectPin, LOW);		// Interrupt on LOW is best.  HIGH not reliable if interrupt line shared
	  chan_alert_sint->MCP23SINT.intCapture();										// Clear interrupts
	  
	  // Save SS pin
		chan_alert_sint->slaveSelectPin = slaveSelectPin;    
		
		// Establish this channel as the one to invoke on interrupt intNum and attach the relevant ISR
		registerChanISR(intNum, chanNum, intMode);
		
		// Start the daemon to follow through on chanISR
		startDaemon(); 
		
		return true;
	}
	else {									
		Serial.println("Bad alert"); 
		return false;
	}
}

boolean HA_channel::initChanAlert(byte alertType, byte chanNum, byte intNum, byte intMode, byte resetPin, byte slaveSelectPin, byte slaveSelectPinBank0, byte slaveSelectPinBank1) {
	_chanType |= alertType;
	
	if (get(VAL_CHAN_ALERT) == CHAN_ALERT_MINT) {									// Up to 256 interrupts from two level cascade of MCP23s17s
		chan_alert_mint = (structMINT*)malloc(sizeof(structMINT));	// Get some space for data needed to interface to the MCP23S17 and to call the appropriate device(s)			
		if (chan_alert_mint == NULL) return false;									// If no space then quit			
		
		// Clear the interrupt ranges for this channel
		for (int i = 0; i < NUM_INTERRUPT_RANGES_MINT; i++) {
			chan_alert_mint->intRange[i].intPin = 0;
			chan_alert_mint->intRange[i].devType = DEV_TYPE_NULL;
		}
		
		// Clear the flag bits
		chan_alert_mint->flagBitsMUX = 0;
		for (int i = 0; i < NUM_MINT_SLAVES; i++) chan_alert_mint->flagBits[i] = 0;
		
		// Initialise SPI comms - to do - improve handling of SPI interface in a multi-threading environment
	  SPI.begin();
	  SPI.setBitOrder(MSBFIRST);
	  SPI.setClockDivider(SPI_CLOCK_DIV2);		// 8MHz
	  
	  // Reset chips
	  pinMode(resetPin, OUTPUT);
	  ::digitalWrite(resetPin, LOW);
	  ::digitalWrite(resetPin, HIGH);  
	  
		// Set up slave interrupt handlers - individually addressed in banks of 8 
		for (int slave = 0; slave < NUM_MINT_SLAVES; slave++) {
  		chan_alert_mint->MCP23SLAVE[slave].beginInt((slave < 8) ? slaveSelectPinBank0 : slaveSelectPinBank1, LOW, slave);
  		chan_alert_mint->MCP23SLAVE[slave].intCapture();
		}
	  
		// Setup MCP23S17 mux to accept interrupts in from slave MCPs
	  chan_alert_mint->MCP23MUX.beginInt(slaveSelectPin, LOW);		// Interrupt on LOW is best.  HIGH not reliable if interrupt line shared
	  for (int slave = 0; slave < NUM_MINT_SLAVES; slave++) {
		  chan_alert_mint->MCP23MUX.intMode(slave, WHILELOW);
		  chan_alert_mint->MCP23MUX.intEnable(slave);
	  }
	  chan_alert_mint->MCP23MUX.intCapture();											// Clear interrupts
	  
	  // Save SS pins
		chan_alert_mint->slaveSelectPinMux = slaveSelectPin; 
		chan_alert_mint->slaveSelectPinBank0 = slaveSelectPinBank0; 
		chan_alert_mint->slaveSelectPinBank1 = slaveSelectPinBank1; 
		
				// Establish this channel as the one to invoke on interrupt intNum and attach the relevant ISR
		registerChanISR(intNum, chanNum, intMode);
		
		// Start the daemon to follow through on chanISR
		startDaemon();
	
	  return true;
  }
	else {
		Serial.println("Bad alert"); 
		return false;
	}	
}

void HA_channel::put(byte type, byte val) {
	// Return pin values
	switch (type) {
		case VAL_CHAN_TYPE:					_chanType = val;
	}
}

byte HA_channel::get(byte type) {
	// Return pin values
	switch (type) {
		case VAL_CHAN_TYPE:					return _chanType; 
		case VAL_CHAN_IO_PIN:				return _ioPin;
		case VAL_CHAN_LATCH_PIN:		return _latchPin; 
		case VAL_CHAN_DATA_PIN:			return _dataPin;
		case VAL_CHAN_CLOCK_PIN:		return _clockPin;
		case VAL_CHAN_MAX_PIN:			return _maxPin;
		case VAL_CHAN_PROTOCOL:			return _chanType & CHAN_PROTOCOL_MASK;
		case VAL_CHAN_ACCESS:				return _chanType & CHAN_ACCESS_MASK;
		case VAL_CHAN_ALERT:				return _chanType & CHAN_ALERT_MASK;
	}
}

HA_channel * HA_channel::getChanObj() { 
	return this; 
};

boolean HA_channel::registerDevRange(byte rangeNum, byte intPin, byte devType, byte devNum) {				// Register a specific device handler to trigger on interrupts within a defined range
	switch (get(VAL_CHAN_ALERT)) {
		case CHAN_ALERT_NONE:	
			break;
		case CHAN_ALERT_INT:		
			chan_alert_int->devType = devType;
			chan_alert_int->devNum = devNum;
			chan_alert_int->intRaised = false;
			break;
		case CHAN_ALERT_SINT:
			if (rangeNum >= NUM_INTERRUPT_RANGES_SINT) {
				Serial.println("Err: SInt range OOB");
				return false;
			}
			chan_alert_sint->intRange[rangeNum].intPin = intPin;
			chan_alert_sint->intRange[rangeNum].devType = devType;
			chan_alert_sint->intRange[rangeNum].devNum = devNum;
			break;
		case CHAN_ALERT_MINT:
			if (rangeNum >= NUM_INTERRUPT_RANGES_MINT) {
				Serial.println("Err: MInt range OOB");
				return false;
			}
			chan_alert_mint->intRange[rangeNum].intPin = intPin;
			chan_alert_mint->intRange[rangeNum].devType = devType;
			chan_alert_mint->intRange[rangeNum].devNum = devNum;
		break;
	}
	
	return true;
}

unsigned int HA_channel::findIntPin(byte devNum) {							// Find the interrupt pin applicable to this device
	// Search through the interrupt ranges for this device number
	byte rangeNum = 0;
	byte intPin;

	switch (get(VAL_CHAN_ALERT)) {
		case CHAN_ALERT_SINT:		
			// Search through the interrupt ranges for this device number	
			while (devNum >= chan_alert_sint->intRange[rangeNum].devNum && rangeNum < NUM_INTERRUPT_RANGES_SINT) { 
				rangeNum++;
				if (chan_alert_sint->intRange[rangeNum].devType == DEV_TYPE_NULL) break;
			}
			
			// After while loop either have hit next range or end of ranges; reduce index and test
			rangeNum--;				
			if (chan_alert_sint->intRange[rangeNum].devType == DEV_TYPE_NULL || chan_alert_sint->intRange[rangeNum].devNum > devNum) Serial.println("findIP: err");
			
			// Work out which interrupt pin alerts this device 
			intPin = chan_alert_sint->intRange[rangeNum].intPin + devNum - chan_alert_sint->intRange[rangeNum].devNum;

			return intPin;
		case CHAN_ALERT_MINT:
		  // Search through the interrupt ranges for this device number	
			while (devNum >= chan_alert_mint->intRange[rangeNum].devNum && rangeNum < NUM_INTERRUPT_RANGES_MINT) { 
				rangeNum++;
				if (chan_alert_mint->intRange[rangeNum].devType == DEV_TYPE_NULL) break;
			}
			
			// After while loop either have hit next range or end of ranges; reduce index and test
			rangeNum--;				
			if (chan_alert_mint->intRange[rangeNum].devType == DEV_TYPE_NULL || chan_alert_mint->intRange[rangeNum].devNum > devNum) Serial.println("findIP: err");
			
			// Work out which interrupt pin alerts this device 
			intPin = chan_alert_mint->intRange[rangeNum].intPin + devNum - chan_alert_mint->intRange[rangeNum].devNum;

			return intPin;
		default: 										
			Serial.println("Bad chantype - E"); 
			return false;
	}
}

boolean HA_channel::enablePin(byte pin) {																								// Enable single pin on 74HC4067 mux using 595 shift registers (see wiring diagrams in HA_channels.h)
	if (pin > _maxPin) {
		Serial.print("pin OOB ");
		Serial.println(pin);
		return false;
	}
	
  byte muxPattern = pin % 16;																														// Pattern to send to mux
  byte whichMux = pin / 16;																															// Which mux
  byte shiftPattern = (whichMux % 2) ? (muxPattern << 4) & 0xF0 : muxPattern & 0x0F;		// Pattern to send to shift register (each SR controls two muxes)
  byte whichShift = whichMux / 2;																												// Which shift register
  
  // Disconnect shift register(s) from latch
  ::digitalWrite(_latchPin, LOW);
  
  // Force clock low initially to ensure a rising edge for the clock
  ::digitalWrite(_clockPin, LOW);				
  
  // Send the bit string to enable the pin - most significant shift register first
  for (int i = _numShifts - 1; i >= 0; i--) {
	  shiftOut(_dataPin, _clockPin, MSBFIRST, (i == whichShift) ? shiftPattern : 0x00);	// Send the pattern to the correct shift register
	}
  
  // Latch shift register(s) onto outputs
  ::digitalWrite(_latchPin, HIGH);
}


boolean HA_channel::digitalWrite(byte pin, byte val) {
	switch (get(VAL_CHAN_ACCESS)) {
		case CHAN_ACCESS_MUX: 
			if (!lock()) return false; 			// Test if can gain exclusive use of channel

	  	// Have got the lock; enable pin & read
	  	if (enablePin(pin)) {
	  		byte dataPin = get(VAL_CHAN_IO_PIN);
	  		pinMode(dataPin, OUTPUT);		// Just in case other channel pins used for input
		  	::digitalWrite(dataPin, val);
		  	unlock();
		  	return true;
	  	}
	  	else {
		  	unlock();
		  	return false;
	  	}
		case CHAN_ACCESS_POWER:
			// Set or unset relevant bit in bitstring
			pinStates->put(pin, val);				
			
			// Disconnect shift register(s) from latch
		  ::digitalWrite(_latchPin, LOW);
		  
		  // Force clock low initially to ensure a rising edge for the clock
		  ::digitalWrite(_clockPin, LOW);				
		  
		  // Write the bitstring to the shift register, and thus turn on/off the relevant pin (whilst leaving others unchanged) - most significant shift register first
		  for (int i = _numShifts - 1; i >= 0; i--) {														
			  shiftOut(_dataPin, _clockPin, MSBFIRST, pinStates->getByte(i));			// Send the pattern to successive shift registers
			}
		  
		  // Latch shift register(s) onto outputs
		  ::digitalWrite(_latchPin, HIGH);
		  
		  return true;
		  break;
	}
}


boolean HA_channel::intEnable(byte intPin) {				// Enable interrupts on virtual pin
	switch (get(VAL_CHAN_ALERT)) {
		case CHAN_ALERT_NONE:		
		case CHAN_ALERT_INT:
			return false;
		case CHAN_ALERT_SINT:			
			chan_alert_sint->MCP23SINT.intEnable(intPin);	
			return true;
		case CHAN_ALERT_MINT:
		  chan_alert_mint->MCP23SLAVE[intPin / 16].intEnable(intPin % 16);	
		  return true;
		default: 										
			Serial.println("Bad chantype - E"); 
			return false;
	}
}

boolean HA_channel::intMode(byte intPin, byte mode) {				// Set interrupt mode on virtual pin
	switch (get(VAL_CHAN_ALERT)) {
		case CHAN_ALERT_NONE:		
		case CHAN_ALERT_INT:
			return false;
		case CHAN_ALERT_SINT:			
			chan_alert_sint->MCP23SINT.intMode(intPin, mode);
			return true;
		case CHAN_ALERT_MINT:
		  chan_alert_mint->MCP23SLAVE[intPin / 16].intMode(intPin % 16, mode);	
		  return true;
		default: 										
			Serial.println("Bad chantype - E"); 
			return false;
	}
}

boolean HA_channel::lock() {			// True if locked successfully; false if already locked
	byte oldSREG = SREG;

	cli();				// Disable interrupts
	
	if (_inUse) {				// Channel already locked by another thread: reject
		SREG = oldSREG;		// Restore original status register
		return false;
	}
	else {
		_inUse = 1;
		SREG = oldSREG;
		return true;
	}
}

boolean HA_channel::unlock() {
	_inUse = 0;
	return true;
}


// *************** Interrupt handling *********************

void HA_channel::chanISR() {								// Called by ISRn in receipt of physical interrupt.  Interrupts disabled, so leave majority of processing to chanDaemon, unless simple interrupt
	unsigned int alertingPin, rangeNum;
	
	switch (get(VAL_CHAN_ALERT)) {
		case CHAN_ALERT_NONE:										// Shouldn't have received an interrupt, but don't delay by reporting an error
			return;
		case CHAN_ALERT_INT:										// Simple interrupt.  Call to ISR is replicated in chanDaemon (with call to processDevInt), allowing device handler to choose
			root.devISR(chan_alert_int->devType, chan_alert_int->devNum);
			counterInts++;
			chan_alert_int->intRaised = true;			// Tested & cleared by chanDaemon
			return;
		case CHAN_ALERT_SINT:										// Multi-interrupt channel using single MCP23s17; channel interrogation required to figure out which pin(s), so do minimum required before exiting
			// Get interrupting lines (disables further interrupts on the active lines).  
			chan_alert_sint->flagBitsSINT |= chan_alert_sint->MCP23SINT.intValid();					// Logical OR to allow for fresh interrupt whilst downstream functions still processing previous
			break;
		case CHAN_ALERT_MINT:										// Multi-interrupt channel using cascade of MCP23s17s; channel interrogation required to figure out which pin(s)												
		  // Have received interrupt from at least one of NUM_MCP23_SLAVES (default 16) slave handlers; find out which one(s) and interrogate to store pin flag(s)
			chan_alert_mint->flagBitsMUX = chan_alert_mint->MCP23MUX.intValid();               // Get slave interrupt flags
  
			// Check which slave handlers have raised interrupt
  		for (int slaveID = 0; slaveID < NUM_MINT_SLAVES; slaveID++) {   
    		if (chan_alert_mint->flagBitsMUX & (1 << slaveID)) {
	    		// Find out from slave handler what caused the interrupt
      		chan_alert_mint->flagBits[slaveID] |= chan_alert_mint->MCP23SLAVE[slaveID].intValid();     	
      		
      		// Resume listening for interrupts from this slave handler
      		chan_alert_mint->MCP23MUX.intEnable(slaveID);                      
    		}
  		}		
			break;
		default:
			return;
	}
}

void HA_channel::chanDaemon(byte dummy) {							// Woken every 100ms as a normal interruptable process to complete the work of chanISR
	unsigned int alertingPin;
	
	switch (get(VAL_CHAN_ALERT)) {
		case CHAN_ALERT_NONE:										// No interrupts to handle; not interested
			return;
		case CHAN_ALERT_INT:										// Simple interrupt, devISR may already have handled if limited processing required
			if (chan_alert_int->intRaised) {
				chan_alert_int->intRaised = false;
				root.processDevInt(chan_alert_int->devType, chan_alert_int->devNum);
			}
			return;
		case CHAN_ALERT_SINT:										// Multi-interrupt channel using single MCP23s17; chanISR has already populated flagBits with the interrupting pin(s)
			// Invoke device handler for each interrupting pin
			for (alertingPin = 0; alertingPin < NUM_LINES_PER_SLAVE; alertingPin++) {	
				if (chan_alert_sint->flagBitsSINT & (1 << alertingPin)) {
					chan_alert_sint->flagBitsSINT &= ~(1 << alertingPin);						// Clear the interrupt flag
					invokeDevInt(alertingPin);																			// Work out which deviceISR to invoke
				}
			}
			break;
		case CHAN_ALERT_MINT:										// Multi-interrupt channel using cascade of MCP23s17s; chanISR has already populated flagBits with the interrupting pin(s)										
			// Invoke device handler for each interrupting pin
  		for (int slaveID = 0; slaveID < NUM_MINT_SLAVES; slaveID++) {         		
	      for (alertingPin = 0; alertingPin < NUM_LINES_PER_SLAVE; alertingPin++) {			
					if (chan_alert_mint->flagBits[slaveID] & (1 << alertingPin)) {
						chan_alert_mint->flagBits[slaveID] &= ~(1 << alertingPin);				// Clear the interrupt flag
						invokeDevInt(alertingPin + (slaveID * NUM_LINES_PER_SLAVE));															// Work out which deviceISR to invoke
					}
				}
  		}		
			break;
		default:
			return;
	}
}


// *************** Private helper functions  *******************

void HA_channel::startDaemon() {					// Setup chanDaemon to run in the background to follow-through on chanISR
	int contextNum;

	// Set 'this' as the object instance and chanDaemon as the member function to be put on queue
	HA_channelMemPtr fPtr = &HA_channel::chanDaemon;
	if ((contextNum = saveContext(this, fPtr, NULL, REPEATED)) < 0) {
	 	Serial.println("out of stack - chan");
	 	return;
 	}
 	
	if (!wakeup.wakeMeAfter(switcherChan, DAEMON_FREQUENCY, (void*)contextNum, TREAT_AS_NORMAL | REPEAT_COUNT)) { 			// Add to queue for activation every 100ms, outside ISR
		Serial.println("Queue full");
	}
}

void HA_channel::invokeDevInt(byte alertingPin) {					// Work out which device to interrupt, based on alerting pin, and then raise an interrupt
	unsigned int rangeNum = 0;
	unsigned int devType, devNum, intPin;
	
	switch (get(VAL_CHAN_ALERT)) {
		case CHAN_ALERT_SINT:	
			// Determine the range to which this interrupt applies 
			while (alertingPin >= chan_alert_sint->intRange[rangeNum].intPin && rangeNum < NUM_INTERRUPT_RANGES_SINT) {
				rangeNum++;
				if (chan_alert_sint->intRange[rangeNum].devType == DEV_TYPE_NULL) break;
			}
			rangeNum--;			
			
			// Get values for later call
			devType = chan_alert_sint->intRange[rangeNum].devType;
			devNum = chan_alert_sint->intRange[rangeNum].devNum;
			intPin = chan_alert_sint->intRange[rangeNum].intPin;
			break;
		case CHAN_ALERT_MINT:	
			// Determine the range to which this interrupt applies 
			while (alertingPin >= chan_alert_mint->intRange[rangeNum].intPin && rangeNum < NUM_INTERRUPT_RANGES_MINT) {
				rangeNum++;
				if (chan_alert_mint->intRange[rangeNum].devType == DEV_TYPE_NULL) break;
			}
			rangeNum--;			
			
			// Get values for later call
			devType = chan_alert_mint->intRange[rangeNum].devType;
			devNum = chan_alert_mint->intRange[rangeNum].devNum;
			intPin = chan_alert_mint->intRange[rangeNum].intPin;
			break;
	}
	
	if (devType == DEV_TYPE_NULL || intPin > alertingPin) Serial.println("Invk err");
	
	// Call relevant device handler for this range.  Device number calculated as offset from base
	root.processDevInt(devType, devNum + alertingPin - intPin, alertingPin);		
}

// ************ Non-class methods **************

// ***********  Switcher support  *************

int saveContext(HA_channel *objPtr, HA_channelMemPtr fPtr, byte arg, byte repeated) {	
	noInterrupts();	
	
	// If space, then find slot
	if (_numContexts < MAXSLEEPERS) {
		int i = 0;
		while (i < MAXSLEEPERS && wakeupContext[i].objType != DEV_TYPE_NULL) i++;

		if (i < MAXSLEEPERS) {
			_numContexts++;
			wakeupContext[i].objType = OBJ_TYPE_CHANNEL;
			wakeupContext[i].HA_channel.objPtr = objPtr;
			wakeupContext[i].HA_channel.fPtr = fPtr;
			wakeupContext[i].arg = (arg & 0x7F) | repeated;
		}
		
		interrupts();

		return i;
	}
	else {
		interrupts();
		return -1;
	}
}

void switcherChan(void* context) {
	byte contextNum = (unsigned int)context & 0x00ff;
	byte arg = wakeupContext[contextNum].arg & 0x7f;
	byte repeated = wakeupContext[contextNum].arg >> 7;
	byte objType = wakeupContext[contextNum].objType;
	
	if (!repeated) freeContext(contextNum);
		
  callMem(*(wakeupContext[contextNum].HA_channel.objPtr), wakeupContext[contextNum].HA_channel.fPtr)(arg);
}

// ************ Interrupt registration and ISRs ***********************

void registerChanISR(byte intNum, byte chanNum, byte intMode) {
	void (*ptrISR)();
	if (intNum >= NUM_INTERRUPT_PORTS) Serial.print("Err: registerChan");
	else {
		// Save channel number to call
		ISRChanNum[intNum] = chanNum;
		
		// Attach the ISR appropriate to the intNum 
		switch (intNum) {
			case 0:		ptrISR = ISR0; break;					// Pin 2
			case 1:		ptrISR = ISR1; break;					// Pin 3
			case 2:		ptrISR = ISR2; break;					// Pin 21
			case 3:		ptrISR = ISR3; break;					// Pin 20
			case 4:		ptrISR = ISR4; break;					// Pin 19
			case 5:		ptrISR = ISR5; break;					// Pin 18
		}
		attachInterrupt(intNum, ptrISR, intMode);	
	}
}

void ISR0() {
	root.chanISR(ISRChanNum[0]);
}

void ISR1() {
	root.chanISR(ISRChanNum[1]);
}

void ISR2() {
	root.chanISR(ISRChanNum[2]);
}

void ISR3() {
	root.chanISR(ISRChanNum[3]);
}

void ISR4() {
	root.chanISR(ISRChanNum[4]);
}

void ISR5() {
	root.chanISR(ISRChanNum[5]);
}
