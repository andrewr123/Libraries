 /*
    Copyright (C) 2011  Andrew Richards
 
    Part of home automation suite
    
    Contains non-class functions to handle interrupts resulting from:
    - Mega interrupts 0 - 5 (ISR0 - ISR5)
    - Timer expiry in support of 'wakeup' ('switcher' function)
    
    In both situations, the ISRs lookup up a static table to determine what to call and with what arguments
    - ISR0-5 lookup 
    - switcher looks up wakeupContext, pre-loaded using saveContext

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

#include "HA_switcher.h" 
#include "HA_root.h"


contextTable wakeupContext[MAXSLEEPERS];

int _numContexts;


void initSwitcher() {
	for (int i = 0; i < MAXSLEEPERS; i++) wakeupContext[i].objType = DEV_TYPE_NULL;		// Initialise
	_numContexts = 0;
}

int saveContext(X *objPtr, XMemPtr fPtr, byte arg, byte repeated) {	
	noInterrupts();	
	
	// If space, then find slot
	if (_numContexts < MAXSLEEPERS) {
		int i = 0;
		while (i < MAXSLEEPERS && wakeupContext[i].objType != DEV_TYPE_NULL) i++;

		if (i < MAXSLEEPERS) {
			_numContexts++;
			wakeupContext[i].objType = DEV_TYPE_RELAY;
			wakeupContext[i].X.objPtr = objPtr;
			wakeupContext[i].X.fPtr = fPtr;
			wakeupContext[i].arg = (arg & 0x7F) | repeated;
			Serial.print("Context = ");
			Serial.println(i);
		}
		interrupts();

		return i;
	}
	else {
		interrupts();
		return -1;
	}
}

int saveContext(HA_devHeat *objPtr, HA_devHeatMemPtr fPtr, byte arg, byte repeated) {	
	noInterrupts();	
	
	// If space, then find slot
	if (_numContexts < MAXSLEEPERS) {
		int i = 0;
		while (i < MAXSLEEPERS && wakeupContext[i].objType != DEV_TYPE_NULL) i++;

		if (i < MAXSLEEPERS) {
			_numContexts++;
			wakeupContext[i].objType = DEV_TYPE_HEAT;
			wakeupContext[i].HA_devHeat.objPtr = objPtr;
			wakeupContext[i].HA_devHeat.fPtr = fPtr;
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

int saveContext(HA_devOpen *objPtr, HA_devOpenMemPtr fPtr, byte arg, byte repeated) {	
	noInterrupts();	
	
	// If space, then find slot
	if (_numContexts < MAXSLEEPERS) {
		int i = 0;
		while (i < MAXSLEEPERS && wakeupContext[i].objType != DEV_TYPE_NULL) i++;

		if (i < MAXSLEEPERS) {
			_numContexts++;
			wakeupContext[i].objType = DEV_TYPE_OPEN;
			wakeupContext[i].HA_devOpen.objPtr = objPtr;
			wakeupContext[i].HA_devOpen.fPtr = fPtr;
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
/*
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
*/
int saveContext(HA_zone *objPtr, HA_zoneMemPtr fPtr, byte arg, byte repeated) {	
	noInterrupts();	
	
	// If space, then find slot
	if (_numContexts < MAXSLEEPERS) {
		int i = 0;
		while (i < MAXSLEEPERS && wakeupContext[i].objType != DEV_TYPE_NULL) i++;

		if (i < MAXSLEEPERS) {
			_numContexts++;
			wakeupContext[i].objType = OBJ_TYPE_ZONE;
			wakeupContext[i].HA_zone.objPtr = objPtr;
			wakeupContext[i].HA_zone.fPtr = fPtr;
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

boolean updateContext(int contextNum, HA_devHeat *objPtr, HA_devHeatMemPtr fPtr, byte arg, byte repeated) {	
	if (wakeupContext[contextNum].objType == DEV_TYPE_HEAT) {
		wakeupContext[contextNum].HA_devHeat.objPtr = objPtr;
		wakeupContext[contextNum].HA_devHeat.fPtr = fPtr;
		wakeupContext[contextNum].arg = (arg & 0x7F) | repeated;
		return true;
	}
	else return false;
}

void freeContext(unsigned int context) {
	byte contextNum = context & 0x00ff;
	if (contextNum > MAXSLEEPERS) {
		Serial.println("Bad context");
		return;
	}
	
	noInterrupts();
	wakeupContext[contextNum].objType = DEV_TYPE_NULL;
	_numContexts--;
	interrupts();
}

byte getContextObj(int contextNum) {
	return wakeupContext[contextNum].objType;
}



void switcher(void* context) {
	byte contextNum = (unsigned int)context & 0x00ff;
	byte arg = wakeupContext[contextNum].arg & 0x7f;
	byte repeated = wakeupContext[contextNum].arg >> 7;
	byte objType = wakeupContext[contextNum].objType;
	
	if (!repeated) freeContext(contextNum);
		
	switch (objType) {
		case DEV_TYPE_RELAY: 
			callMem(*(wakeupContext[contextNum].X.objPtr), wakeupContext[contextNum].X.fPtr)(arg);
			break;
		case DEV_TYPE_HEAT: 
			callMem(*(wakeupContext[contextNum].HA_devHeat.objPtr), wakeupContext[contextNum].HA_devHeat.fPtr)(arg);
			break;
		case DEV_TYPE_OPEN: 
			callMem(*(wakeupContext[contextNum].HA_devOpen.objPtr), wakeupContext[contextNum].HA_devOpen.fPtr)(arg);
			break;
		case OBJ_TYPE_CHANNEL: 
			callMem(*(wakeupContext[contextNum].HA_channel.objPtr), wakeupContext[contextNum].HA_channel.fPtr)(arg);
			break;
		case OBJ_TYPE_ZONE: 
			callMem(*(wakeupContext[contextNum].HA_zone.objPtr), wakeupContext[contextNum].HA_zone.fPtr)(arg);
			break;
		default: 
			Serial.print("sw: Wrong type ");
			Serial.print(objType);
			Serial.print(" context = ");
			Serial.println(contextNum);
	}
}

/*  Class-specific versions of switcher
		Much simpler, but not compatible with wakeup and no longer used 
	
void switcher(X *object, XMemPtr fPtr) {
	callMem(*object, fPtr)(value);
}

void switcher(HA_devHeat *object, HA_devHeatMemPtr fPtr) {
	callMem(*object, fPtr)();
}

*/

// ********** Function to test switcher and member function pointers

void X::f(byte arg) {
	int context;
	static byte value = 20;
  Serial.print("Val received = ");
  Serial.println(arg);
  delay(500);
  XMemPtr fPtr = &X::f;
  
	if (++value < 30) {
	  // Set this as the class member to invoke
		if ((context = saveContext(this, fPtr, value)) < 0) Serial.println("out of stack");
		else switcher((void*)context);	
	}
	else value = 20;
}
/*
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
*/
