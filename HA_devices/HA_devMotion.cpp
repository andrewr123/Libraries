/*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
    
    Contains the HA_devMotion library to manage ePIR sensors

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

#include "HA_devMotion.h"
#include "Wakeup.h"
#include "HA_root.h"
#include "HA_switcher.h"
#include "Mcp23s17.h"


// **************** HA_devMotion ***************
//
// Sensor - on or off.  Based on the Zilog ePIR module in serial interface mode with interrupt-driven wakeup of Mega
// Incorporates elements from ePIR library by Corey Johnson Sep 2010, published under GNU licence v2

void HA_devMotion::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_MOTION);
}

void HA_devMotion::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_MOTION);
}

void HA_devMotion::initDev(byte devNum, byte channel, byte muxPin, byte handler) {	
	unsigned int intPin;
	put(VAL_PIN, muxPin);						// Logical pin number (if mux); ignored if direct (channel already set up with serial port)
	put(VAL_CHANNEL, channel);
	put(VAL_HANDLER, handler);
	put(VAL_DEVIDX, devNum);
	
	HA_channel *chanPtr = root.getChanObj(channel);						// Get pointer to channel
	
	// Set up to receive alerts, if required
	switch (chanPtr->get(VAL_CHAN_ALERT)) {		
		case CHAN_ALERT_NONE:										// Read initiated by program; no action required
			break;
		case CHAN_ALERT_INT:										// Simple (physical) interrupt; register this device number as first and only range
			chanPtr->registerDevRange(R0, NULL, DEV_TYPE_MOTION, devNum);
			break;
		case CHAN_ALERT_SINT:										// Logical interrupt channel using single MCP23s17 (max 16 pins). Pin ranges need to be set up in advance by main program
		case CHAN_ALERT_MINT:										// Logical interrupt channel using multiple MCP23s17 (max 256 pins).  Pin ranges need to be set by main program
			intPin = chanPtr->findIntPin(devNum);

			// Enable the interrupt pin
			chanPtr->intMode(intPin, FALLING);		// ePIR int remains low for at least 2s and long after int re-enabled, so not safe to have WHILELOW
			chanPtr->intEnable(intPin);
						
			break;
		default:
			Serial.println("initDev: alert fault");
			put(VAL_STATUS, STATUS_UNAVAILABLE);
			return;
	}
		
	put(VAL_STATUS, STATUS_READY);														// Assume success
	put(VAL_PUSH, OFF);																				// Previous
	put(VAL_PUSH, OFF);																				// Current
	
	// Initialise ePIR through appropriate access route
	switch (chanPtr->get(VAL_CHAN_ACCESS)) {
		case CHAN_ACCESS_DIRECT:
			if (initEPIR()) return;
			break;
		case CHAN_ACCESS_MUX:			// Get exclusive use of channel and initialise device
	  	if (muxPin <= (chanPtr->get(VAL_CHAN_MAX_PIN)) && chanPtr->lock()) {
				if (chanPtr->enablePin(get(VAL_PIN)) && initEPIR()) {
						chanPtr->unlock();
						return;
				}
				chanPtr->unlock();							// Channel locked; release it before falling through to report error	
			}	
	}														

	// On error, fall through to report error
	Serial.println("initDev: access fault "); 
	put(VAL_STATUS, STATUS_UNAVAILABLE);
}

void HA_devMotion::readDev(byte devNum) {						// Read motion sensor, either directly or by setting status to pending and thus allowing channel interrupts
	// Check initiated
	if (get(VAL_STATUS) == STATUS_UNAVAILABLE) {
		Serial.println("readDev: not ready");
		return;
	}
	
	// Set flag to indicate data pending
	put(VAL_STATUS, STATUS_PENDING);
	
	HA_channel *chanPtr = root.getChanObj(get(VAL_CHANNEL));						
	
	if (chanPtr->get(VAL_CHAN_ALERT) == CHAN_ALERT_NONE) readPort();   // Read directly using serial port specified for channel
};

void HA_devMotion::readPort(byte alertingPin) {							// Called by channel daemon on receipt of physical or logical interrupt, and by readDev in POLL mode
	char status;
	
	// Check valid status			
	switch (get(VAL_STATUS)) {
		case STATUS_PENDING:
		case STATUS_STABLE:
			break;
		default:
			Serial.print("devMotion: not ready: ");
			Serial.println(get(VAL_STATUS), DEC);
			return;
	}
	
	// Get the channel
	HA_channel *chanPtr = root.getChanObj(get(VAL_CHANNEL));						

	// Figure out how to get to the sensor and then read it
	switch (chanPtr->get(VAL_CHAN_ACCESS)) {
		case CHAN_ACCESS_DIRECT:														// Direct access; no need to enable channel
			status = getStatus();
			break;
		case CHAN_ACCESS_MUX:																// Mux access - lock for exclusive use and then access device through designated pin
			if (chanPtr->lock()) {
				if (chanPtr->enablePin(get(VAL_PIN))) {
					status = getStatus();
					chanPtr->unlock();
					break;
				}
				else chanPtr->unlock();							// Channel locked; release it before falling through to report error
			}																			// Fall through on all errors
		default:			
			Serial.println("readPort: ePIR fault"); 
			put(VAL_STATUS, STATUS_UNAVAILABLE);
	}
	
	// If polled read then store status
	// If alerted by any interrupt then set status ON and (if alerted by logical interrupt) re-enable interrupt on the pin	
	switch (chanPtr->get(VAL_CHAN_ALERT)) {
		case CHAN_ALERT_NONE:
			switch (status) {
				case 'Y':	
					handleEvent(ON);
					break;
				case 'N': 	
					handleEvent(OFF);
					break;
				default:	
					Serial.print("Status = ");
					Serial.println(status); 	
					put(VAL_STATUS, STATUS_UNAVAILABLE);
			}
			break;
		case CHAN_ALERT_INT:
			handleEvent(ON);
			break;
		case CHAN_ALERT_SINT:
		case CHAN_ALERT_MINT:
			handleEvent(ON);
			chanPtr->intEnable(alertingPin);		// Resume listening for interrupts on this pin	
	}
}

void HA_devMotion::handleEvent(byte status) {
	int zoneNum;
	
	// Set status
	put(VAL_PUSH, status);
	put(VAL_STATUS, STATUS_STABLE);
	
	if (status == ON) {			//	Presence detected - find zone and set zone occupied
		if ((zoneNum = root.findZone(get(VAL_REGION_ZONE))) >= 0) root.putEnt(OBJ_TYPE_ZONE, zoneNum, VAL_OCCUPANCY, ON);
	}
}


void HA_devMotion::devISR(byte pin) {							// Called by channel ISR on receipt of physical interrupt.  Common to all devices, but serial access for ePIR means too much to
	counterInts++;
}																									// do here with interrupts disabled, so return immediately and allow readPort to handle it later when called from channel daemon

// Following elements from ePIR library by Corey Johnson Sep 2010

char HA_devMotion::readChar(char command){ // ......... Returns value selected by command sent to ePIR.
	int count = NUM_TRIES;
	int count2 = NUM_TRIES;
	char outChar = '\0'; // ..................... Assign NULL to output variable.
	
	HA_channel *chanPtr = root.getChanObj(get(VAL_CHANNEL));

	do {	
		(*(chanPtr->serialPtr)).print(command);
		while ((count-- > 0) && ((*(chanPtr->serialPtr)).available() == 0)) delay(RETRY_DELAY);
		outChar = (*(chanPtr->serialPtr)).read();
		count = NUM_TRIES;
	} while ((count2-- > 0) && (outChar == NACK));

	return outChar; // .......................... Return from function with value from ePIR.
}

char HA_devMotion::writeChar(char command, char inChar){ // ... Changes value of ePIR selected by command to value of inChar and returns with ACK. 
	int count = NUM_TRIES;
	int count2 = NUM_TRIES;
	int count3 = NUM_TRIES;
	char outChar = '\0'; // ............................. Assign NULL to output variable.
	
	HA_channel *chanPtr = root.getChanObj(get(VAL_CHANNEL));

	do {
		do {	
			(*(chanPtr->serialPtr)).print(command);
			while ((count-- > 0) && ((*(chanPtr->serialPtr)).available() == 0)) delay(RETRY_DELAY);
		} while ((count2-- > 0) && ((*(chanPtr->serialPtr)).read() == NACK));
		count = count2 = NUM_TRIES;
		(*(chanPtr->serialPtr)).print(inChar);
		while ((count-- > 0) && ((*(chanPtr->serialPtr)).available() == 0)) delay(RETRY_DELAY);
		outChar = (*(chanPtr->serialPtr)).read();
	} while ((count3-- > 0) && (outChar != ACK));
	
	return outChar; // .................................. Return from function with value 'ACK'.
} 

char HA_devMotion::confirm(void){ // ................ Sends confirmation sequence '1234' to ePIR (Needed for .Sleep and .Reset functions).
	char outChar = '\0'; // ................... Assign NULL to output variable.
	
	HA_channel *chanPtr = root.getChanObj(get(VAL_CHANNEL));
	
	for (char num = '1'; num < '5'; num++) (*(chanPtr->serialPtr)).print(num);
	while ((*(chanPtr->serialPtr)).available() == 0);
	outChar = (*(chanPtr->serialPtr)).read();
	
	return outChar; // ........................ Return from function with value 'ACK'.
}

// Initialise
boolean HA_devMotion::initEPIR() {
	int count = NUM_TRIES;

	while ((count-- > 0) && (getStatus() == 'U')) delay(RETRY_DELAY);
	
	char status = getStatus();
	switch (status) {
		case 'Y':
		case 'N':
			break;
		case 'U': 
			Serial.println("ePIR not stable");
			return false;
		case NACK:
			Serial.println("ePIR comms failure");
			return false;
		default:
			Serial.print("Status error: ");
			Serial.println(status);
	}
	delay(10);
	
	if (gateThresh(GATE_THRESH) != GATE_THRESH) {Serial.println("GT wrong"); return false;}
	delay(10);

	if (MDRmode() != MDR_MODE) {Serial.println("MD wrong");	return false;}
	delay(10);

	if (MDtime(MD_TIME) != MD_TIME) {Serial.println("MD time wrong"); return false;}
	delay(10);

	if (extRange(EXT_RANGE) != EXT_RANGE) {Serial.println("ER wrong"); return false;}
	delay(10);

	if (freq(FREQ) != FREQ) {Serial.println("Freq wrong"); return false;}
	delay(10);

	if (pulseCount(PULSE_COUNT) != PULSE_COUNT) {Serial.println("PC wrong"); return false;}
	delay(10);

	if (sensitivity(SENSY) != SENSY) {Serial.println("Sensitivity wrong"); return false; }
	delay(10);

	getStatus();
	
	return true;
}

//  Motion Detect Status (and reset)
char HA_devMotion::getStatus(){
	return readChar('a');
}

// Light Gate Threshold 
byte HA_devMotion::gateThresh(word threshold){
	if (257 > threshold){
		if (256 > threshold){
			writeChar('L', char(lowByte(threshold)));
		}
		else {
			byte defValue = 100;
			writeChar('L', char(defValue));
		}
	}
	byte thresholdOut = byte(readChar('l'));
	return thresholdOut;
}

// MD/R Pin Mode 
char HA_devMotion::MDRmode(){
	writeChar('C', 'M');
	char mdrModeOut = readChar('c');
	return mdrModeOut;
}

// MD-Pin Active Time 
byte HA_devMotion::MDtime(word mdTime){
	if (257 > mdTime){
		if (256 > mdTime){
			writeChar('D', char(lowByte(mdTime)));
		}
		else {
			byte defValue = 2;
			writeChar('D', char(defValue));
		}
	}
	byte mdTimeOut = byte(readChar('d'));
	return mdTimeOut;
}

// Extended Range 
char HA_devMotion::extRange(char extended){
	if (extended != '\0'){
		switch (extended){
			case 'Y':
				writeChar('E', 'Y');
			break;
			default:
				writeChar('E', 'N');
			break;
		}
	}
	char extendedOut = readChar('e');
	return extendedOut;
}

// Frequency Response 
char HA_devMotion::freq(char frequency){
	if (frequency != '\0'){
		switch (frequency){
			case 'H':
				writeChar('F', 'H');
			break;
			default:
				writeChar('F', 'L');
			break;
		}
	}
	char frequencyOut = readChar('f');
	return frequencyOut;
}

// Pulse Count 
byte HA_devMotion::pulseCount(byte pulseCount){
	if (0 < pulseCount){
		byte pcValue;
		switch (pulseCount){
			case 2:
				pcValue = 2;
				writeChar('P', '2');
			break;
			default:
				pcValue = 1;
				writeChar('P', '1');
			break;
		}
	}
	byte pulseCountOut = (byte(readChar('p')) - 48);
	return pulseCountOut;
}

// Sensitivity 
byte HA_devMotion::sensitivity(word sensitivity){
	if (257 > sensitivity){
		if (256 > sensitivity){
			writeChar('S', char(lowByte(sensitivity)));
		}
		else {
			byte defValue = 6;
			writeChar('S', char(defValue));
		}
	}
	byte sensitivityOut = byte(readChar('s'));
	return sensitivityOut;
}

