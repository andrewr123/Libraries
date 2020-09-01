 /*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
    
    Contains global utility methods

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


#include "HA_globals.h"


// *************** IP and other comms addresses - variables here, statics in HA_globals.h *************

#ifdef PITTSFOLLY
		byte diningIP[] 								= {192, 168, 7, 178};
		byte basementIP[] 							= {192, 168, 7, 179};
		byte studyIP[] 									= {192, 168, 7, 180};
		byte testIP[]										= {192, 168, 7, 181};
		byte syslogServerIP[] 					= {192, 168, 7, 20};
		byte gatewayIP[] 								= {192, 168, 7, 1};
	  byte pingAddr[NUM_PINGS][4] 		= { {8, 8, 8, 8},														// Google DNS
	  																	{212, 159, 13, 49},															// Plusnet DNS
	  																	{212, 159, 6, 10},															// Plusnet DNS
	  																	{208, 67, 222, 222} };													// OpenDNS
    byte diningMac[] 								= { 0x90, 0xA2, 0xda, 0x00, 0x3c, 0x92 };   	// Pittsfolly Arduino
    byte basementMac[] 							= { 0x90, 0xA2, 0xda, 0x00, 0x2e, 0x9B };
    byte studyMac[]  								= { 0x90, 0xA2, 0xda, 0x00, 0x2e, 0x9C };
    byte testMac[]									= { 0x90, 0xA2, 0xda, 0x00, 0x2e, 0x9D };

	  
#else
		byte diningIP[] 								= {192, 168, 2, 178};
		byte basementIP[] 							= {192, 168, 2, 178};
		byte studyIP[] 									= {192, 168, 2, 178};
		byte testIP[]										= {192, 168, 2, 181};
		byte syslogServerIP[] 					= {192, 168, 2, 61};
		byte gatewayIP[] 								= {192, 168, 2, 1};
	  byte pingAddr[NUM_PINGS][4] 		= { {8, 8, 8, 8},														// Google DNS
	  																	{212, 159, 13, 49},															// Plusnet DNS
	  																	{212, 159, 6, 10},															// Plusnet DNS
	  																	{208, 67, 222, 222} };													// OpenDNS
		byte diningMac[] 								= { 0x90, 0xA2, 0xda, 0x00, 0x2f, 0x9A };		// Away Arduino
		byte basementMac[] 							= { 0x90, 0xA2, 0xda, 0x00, 0x2e, 0x9A };
		byte studyMac[]  								= { 0x90, 0xA2, 0xda, 0x00, 0x2e, 0x9A };
		byte testMac[]  								= { 0x90, 0xA2, 0xda, 0x00, 0x2e, 0x9A };
#endif


// unsigned int heartBeat;


// @author: Mathias Van Malderen (tux4life)

void stoupper(char *s) {
    for(; *s; s++)
        if(('a' <= *s) && (*s <= 'z'))
            *s = 'A' + (*s - 'a');
}

void stolower(char *s) {
    for(; *s; s++)
        if(('A' <= *s) && (*s <= 'Z'))
            *s = 'a' + (*s - 'A');
}


// ************  Device type codes & methods  *********

const char REGIONCODES[NUMREGIONCODES + 1] = { 'G', 'D', 'S', 'E', 'K', 'B', '/0' };          // Gt Hall, Dining, Study, External, Kitchen, Basement, null termination
const char *DEVICETYPES[NUM_DEV_TYPES + NUM_VAR_TYPES + NUM_ZONE_TYPES + NUM_OBJ_TYPES] = { "xT", "xF", "xH", "xL", "xM", "xP", "xR", "xO", "p", "P", "D", "L", "R", "vB", "vI", "vR", "Z", "Ti", "HB" };  
								// Sensors:  Touch, fire, heat, luminance, motion, presence, RFID, open
								// Actors: 5a power, 13A power, lock (was 'B'), light, relay

byte getDevTypeIdx (char *device) {
	  for (int i = 0; i < NUM_DEV_TYPES; i++) {
      if (strncmp(device, DEVICETYPES[i], strlen(DEVICETYPES[i])) == 0) return i;
    }
    return 0;
}

void getDevTypeChar (byte entIdx, char *entChar) {			// Returns device type, var type or zone type char string
	strncpy (entChar, DEVICETYPES[entIdx], 3);
}


// *************** DayHourMinute routines - compressed form of time **************

unsigned int dhmGet(unsigned int dhmVal, byte type) {
	return dhmAccess (&dhmVal, type, NULL, READ_FLAG); 
}

void dhmPut(unsigned int *dhmVal, byte type, unsigned int value) { 
	dhmAccess (dhmVal, type, value, WRITE_FLAG); 
}

unsigned int dhmMake(unsigned int dhmDay, unsigned int dhmHour, unsigned int dhmMinute) {
  unsigned int result = 0;
  dhmPut(&result, VAL_DAY, dhmDay);
  dhmPut(&result, VAL_HOUR, dhmHour);
  dhmPut(&result, VAL_MINUTE, dhmMinute);
  return result;
}

unsigned int dhmAccess(unsigned int *dhmVal, byte type, unsigned int value, int flag) {           
	unsigned int mask, offset, limit;
  
  switch (type) {
		case VAL_DAY:				mask = MASK_DAY; offset = OFFSET_DAY; limit = 10;  break;
		case VAL_HOUR:			mask = MASK_HOUR; offset = OFFSET_HOUR; limit = 23;  break;
		case VAL_MINUTE:		mask = MASK_MIN; offset = 0; limit = 59;  break;
  }
  
	if (flag == READ_FLAG) {
		unsigned int response = (unsigned int)(((*dhmVal) & mask) >> offset);
		return (response > limit) ? 99 : response;
	}
  else {
		if (value > limit) return 0;
		else {
			(*dhmVal) &= ~mask;                                   // Clear
			(*dhmVal) |= (value << offset) & mask;                // Store 
		}
  }
}

void dhmToText(unsigned int dhm, char *responseText) {
	const char *dayText[11] = {"Every", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Mon-Thu", "Mon-Fri", "Sat/Sun"};
	unsigned int dayVal = dhmGet(dhm, VAL_DAY);
		
	snprintf(responseText, 15, "%s%s%02d%s%02d", (dayVal > 10) ? "Error" : dayText[dayVal], " ", dhmGet(dhm, VAL_HOUR), ":", dhmGet(dhm, VAL_MINUTE));
}

boolean dhmBetween(unsigned int dhmVal, unsigned int dhmFrom, unsigned int dhmTo) {
  unsigned int dhmFDay = dhmGet(dhmFrom, VAL_DAY), dhmTDay = dhmGet(dhmTo, VAL_DAY);
  boolean nextWeek = false;
 
  switch (dhmFDay) {			// NB default case results in a return 
    case 0:      dhmFDay = 1; dhmTDay = 7; break;        			// Daily
    case 8:      dhmFDay = 2; dhmTDay = 5; break;        			// Mon-Thu
    case 9:      dhmFDay = 2; dhmTDay = 6; break;        			// Mon-Fri
    case 10:     dhmFDay = 7; dhmTDay = 1; nextWeek = true; break;        			// Weekend 
    default:		 																							// Specific times & days - just need to take account of potential change of week
    	nextWeek = (dhmTDay < dhmFDay);
    	return (nextWeek) ? (dhmVal >= dhmFrom) || (dhmVal <= dhmTo) : (dhmVal >= dhmFrom) && (dhmVal <= dhmTo);
  }

  // Only here if range of days to be tested - cycle through same times each day
  for (int i = dhmFDay; i <= ((nextWeek) ? 7 : dhmTDay); i++) {
    dhmPut(&dhmFrom, VAL_DAY, i);
    dhmPut(&dhmTo, VAL_DAY, i);
    
    // Test if time now is between the times stated; if so break out of loop
    if (dhmVal >= dhmFrom && dhmVal <= dhmTo) return true;
  }
  
  // Second loop needed if end date is next week
  if (nextWeek) {
  	for (int i = 1; i <= dhmTDay; i++) {
    	dhmPut(&dhmFrom, VAL_DAY, i);
    	dhmPut(&dhmTo, VAL_DAY, i);
    
      // Test if time now is between the times stated; if so break out of loop
      if (dhmVal >= dhmFrom && dhmVal <= dhmTo) return true;
    }
  }
  
  // If get here then no match, so return
  return false;
}
