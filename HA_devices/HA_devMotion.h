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

 
#ifndef HA_devMotion_h
#define HA_devMotion_h

#include "HA_globals.h"
#include "HA_device_bases.h"
#include "HA_channels.h"
#include "OneWire.h"

//	const static byte NUM_INTERRUPT_RANGES_SINT = 4;


// ************ HA_devMotion *****************
//
// Sensor - on or off.  Based on the Zilog ePIR module in serial interface mode with interrupt-driven wakeup of Mega
// Incorporates elements from ePIR library by Corey Johnson Sep 2010, published under GNU licence v2

class HA_devMotion : public HA_sensDigital {
	public:
		void getRef(char *device);
		void getSnapshot();
		void initDev(byte devNum, byte channel, byte muxPin, byte handler);
		void readDev(byte devNum);
		void readPort(byte alertingPin = 0);
		void devISR(byte pin);
		
	private:
		// Methods
		char readChar(char); 										// Read value from the ePIR.
		char writeChar(char, char); 						// Write value to the ePIR.
		char confirm(void); 										// Sends confirmation command for special functions 'Sleep' and 'Reset'.
		
		void handleEvent(byte status);					// Called in response to change of state

		boolean initEPIR();											// True if OK, false if not initialised
		char getStatus(); 											// READ ONLY <Y,N,U>
		byte gateThresh(word threshold);				// 0-255/256 sets default
		char MDRmode();													// Always M in HA suite
		byte MDtime(word mdTime);								// 0-255/256 sets default
		char extRange(char extended);						// Y, N
		char freq(char frequency);							// H, L
		byte pulseCount(byte pulseCount);				// 1, 2
		byte sensitivity(word sensitivity);			// 0-255/256 sets default

		// Constants
		static const byte ACK = 0x06; 								// "Acknowledge"
		static const byte NACK = 0x15; 								// "Non-Acknowledge"
		static const unsigned int STABILISE_TIMEOUT = 5000;		// ms max to wait for device to stabilise
		static const unsigned int RETRY_DELAY = 50;						// ms testing frequency
		static const unsigned int NUM_TRIES = STABILISE_TIMEOUT / RETRY_DELAY;
		static const unsigned int GATE_THRESH = 90;
		static const byte MDR_MODE = 'M';							// Flag motion detect
		static const byte MD_TIME = 2;								// Stay active for 2 secs
		static const byte EXT_RANGE = 'Y';						// Set Extended Range
		static const byte FREQ = 'L';									// Detect Low and High freq responses
		static const byte PULSE_COUNT = 1;						// Trigger on first detection
		static const byte SENSY = 3;									// Sensitive
		


};

typedef void (HA_devMotion::*HA_devMotionMemPtr)();



#endif
