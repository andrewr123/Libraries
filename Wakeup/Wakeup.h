 /*  
	*****************  WAKEUP  **********************
	
	Description
	-----------
	
	Definition file to accompany wakeup.cpp - see full description there
	
	Version history
	---------------
	
	Version 1.0 Oct 2011 - Initial release, Andrew Richards
	Version 1.1 Jul 2012 - Revised to allow delay of 49 days (4,294,967 seconds)
											 - change long ms to unsigned long ms
											 - specify repeating timer with additional flag, rather than negative ms
											 - specify units with additional flags - ms, secs, min, hours, days
	
	Licencing
	---------

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

 
#ifndef Wakeup_h
#define Wakeup_h

#include "Arduino.h"  
//#include "HA_syslog.h"     

static const byte MAXSLEEPERS 					= 32;							// Max 64k, but unworkable (out of memory and would take too long). Tested with 256; 16-32 generally enough
static const byte MAXPENDING 						= 32;							// Needs to be large enough to hold max number of sleepers woken in one go, assuming prompt clearance by runAnyPending()
static const unsigned int MAXHEARTBEAT 	= 8350;						// in ms.  Round down from absolute max of 8,388,480 us (Timer1 limit)   4,294,967,295
static const byte CODEOVERHEAD 					= 8; 							// Logic analyser-determined to allow for duration of WAKEUP code; calibrated @ 10mS with 8 sleepers
static const unsigned long MAX_SECONDS	= 4294967;				// Base units always held as ms in unsigned long (4,294,967,295 ms)
static const unsigned long MAX_MINUTES	= 71582;					// MAX_SECONDS / 60
static const unsigned int MAX_HOURS			= 1193;						// MAX_HOURS / 60
static const unsigned int MAX_DAYS			= 49;							// MAX_DAYS / 24

static const byte TREAT_AS_NORMAL 			= 0;							// Queue for execution after call to runAnyPending; runs with interrupts enabled
static const byte TREAT_AS_ISR 					= B00000001;			// Run immediately timer expires with interrupts DISABLED
static const byte REPEAT_COUNT					= B00000010; 			// Repeat when triggered
static const byte UNITS_SECONDS 				= B00000100;			// Default is ms
static const byte UNITS_MINUTES					= B00001000;
static const byte UNITS_HOURS						= B00010000;
static const byte UNITS_DAYS						= B00100000;
static const byte HAS_CONTEXT 					= B10000000;			// Whether to pass context field on call or not

class WAKEUP {
public:
  void init();																																										// Must be called at startup
  boolean wakeMeAfter( void (*sleeper)(), unsigned long delay, byte flags);												// Function to wake after delay.  Flags determine whether one-shot or repeat, and whether woken as ISR or normal
  boolean wakeMeAfter( void (*sleeper)(void*), unsigned long delay, void *context, byte flags);		// As above, but with context to be passed to sleeper on wakeup
  void runAnyPending();																																						// Called by the main program to run any pending sleepers
  unsigned int freeSlots();																																				// Returns number of bunks available
  void timerISR();																																								// Called every _heartbeat.  Must be public to allow call by timerISRWrapper()
  boolean cancelWakeup(void (*sleeper)(void*), unsigned long delay, void *context, byte flags);								// Cancels wakeup call and removes sleeper
  boolean resetWakeup(void (*sleeper)(void*), unsigned long delay, void *context, byte flags);								// Resets wakeup call to original
 
private:
  // Methods
  boolean addSleeper( void (*sleeper)(void*), unsigned long delay, void *context, byte flags);	
  void printBunks();
  void startHeartbeat();						// Starts timer based on shortest time to wake
  void stopHeartbeat();							// Stops timer (when no sleepers)
  unsigned int msElapsed();					// Duration in mS since last heartbeat
  unsigned long adjustDelay(unsigned long delay, byte flags);			// Adjust delay to reflect units

  // Properties - many can be changed via an ISR, so need to be volatile
  byte _oldSREG;													// Temporary store of status register
  boolean _inISR;													// Blocks use of runAnyPending by sleepers running under ISR
  volatile unsigned long _heartbeat;			// mS frequency of checking timeToWake 
  
  volatile unsigned int _numSleepers;			// Number of sleepers - if zero then turn off heartbeat
  struct _bunk {													// 'Bunk' holding sleeper or empty
  	union {
			void (*callback)(void*);							// Callback function ('sleeper')
			void (*callback2)();
		};
  	void *context;												// For sleeper to interpret as appropriate when woken
		byte flags; 													// See constants above
  	unsigned long sleepDuration;					// Requested delay in mS; if negative then repeating
  	unsigned long timeToWake;							// Loaded with _sleepDuration and then decremented by time since last heartbeat
  }  volatile _bunks[MAXSLEEPERS];				// Dynamically reshuffled for performance - first _numSleepers slots are always valid

  volatile unsigned int _numPending;			// Updated in ISR and non-ISR code     
  struct _pend {													// A sleeper that has been woken and is ready to go
  	union {
			void (*callback)(void*);							// Callback function ('sleeper')
			void (*callback2)();
		};				
		void *context;
		byte flags;
  } volatile _pending[MAXPENDING];				// Top end also used as temporary scratchpad by timerISR

  	
};

extern WAKEUP wakeup;

extern void timerISRWrapper();

#endif

