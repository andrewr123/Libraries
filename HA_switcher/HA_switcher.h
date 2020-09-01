 /*
    Copyright (C) 2011  Andrew Richards
 
    Part of home automation suite
    
    Contains non-class functions to handle callbacks of generic classes and member functions
    The 'switcher' function is used as an argument to wakeup

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

#ifndef HA_switcher_h
#define HA_switcher_h

#include "HA_globals.h"
#include "HA_devices.h"
#include "HA_zone.h"
#include "HA_devHeat.h"
#include "HA_devMotion.h"
#include "HA_channels.h"
#include "wakeup.h"


// ************ For test purposes ************

class X {
public:
  void f(byte arg);
};

typedef void (X::*XMemPtr)(byte arg);

//void switcher(X *object, XMemPtr fPtr);

// ********* Switcher used as callback to route wakeup calls 
// Cannot wakeup class members directly
// See http://www.parashift.com/c++-faq-lite/pointers-to-members.html
// and http://publib.boulder.ibm.com/infocenter/lnxpcomp/v8v101/index.jsp?topic=%2Fcom.ibm.xlcpp8l.doc%2Flanguage%2Fref%2Fstrct.htm
// Could define as a class, but then 'switcher' would have to be static with other functions as members - all too complicated

void initSwitcher();

int saveContext(X *object, XMemPtr fPtr, byte arg = 0, byte repeated = 0);
int saveContext(HA_devHeat *object, HA_devHeatMemPtr fPtr, byte arg = 0, byte repeated = 0);
int saveContext(HA_devOpen *object, HA_devOpenMemPtr fPtr, byte arg = 0, byte repeated = 0);
//int saveContext(HA_channel *object, HA_channelMemPtr fPtr, byte arg = 0, byte repeated = 0);
int saveContext(HA_zone *object, HA_zoneMemPtr fPtr, byte arg = 0, byte repeated = 0);

boolean updateContext(int contextNum, HA_devHeat *objPtr, HA_devHeatMemPtr fPtr, byte arg = 0, byte repeated = 0);

void freeContext(unsigned int context);

byte getContextObj(int contextNum);

void switcher(void *context);				// Callback function called on wakeup.  Context is index into instance of contextTable, which then gives 
																		// - the object and member function pointers
																		// - 7 bit optional argument
																		// - 1 bit repeat flag

struct contextTable {
	byte objType;
	union {
		struct {
			X 									*objPtr;
			XMemPtr							fPtr;
		} X;
		struct {
			HA_devHeat 					*objPtr;
			HA_devHeatMemPtr 		fPtr;
		} HA_devHeat;		
		struct {
			HA_devOpen 					*objPtr;
			HA_devOpenMemPtr 		fPtr;
		} HA_devOpen;	
		struct {
			HA_channel 					*objPtr;
			HA_channelMemPtr 		fPtr;
		} HA_channel;	
		struct {
			HA_zone 						*objPtr;
			HA_zoneMemPtr 			fPtr;
		} HA_zone;	
	};
	byte arg;
};


extern int _numContexts;
extern contextTable wakeupContext[MAXSLEEPERS];

const static byte REPEATED = 0x80;

#define callMem(object, fPtr) ((object).*(fPtr))	// Macro to simplify invoking objects and member functions

/*
// *********** Interrupt handling methods, used by HA_channels in conjunction with HA_switcher ******************

void registerChanISR(byte intNum, byte chanNum, byte intMode);				// Register ISR and associated chanNum to be called on receipt of interrupt on intNum (Int 0 = pin 2; 1 = 3; 2 = 21; 3 = 20; 4 = 19; 5 = 18)
void ISR0();																													// Interrupt Service Routine invoked on interrupt 0 - calls ISRChanNum[0]
void ISR1();
void ISR2();
void ISR3();
void ISR4();
void ISR5();

		
// *********** VARIABLES *****************

static byte ISRChanNum[NUM_INTERRUPT_PORTS];					// Holds the number of the channnel to invoke on receipt of interrupt [n]

*/


#endif
