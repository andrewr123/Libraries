 /*
    Copyright (C) 2011  Andrew Richards
 
    Part of home automation suite
    
    Contains the HA_time class for providing accurate time

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
	
	Dec 16 - v2 - minor updates to simplify initialisation
*/

 
#ifndef HA_time_h
#define HA_time_h


#include "HA_globals.h"
#include "Time.h"
#include "EthernetUdp.h"
#include "HA_syslog.h"
#include "Wakeup.h"


//************ PROPERTIES ************

static const unsigned int REFRESH_INTERVAL_SECS 	= 300;														// Check NTP timeserver every 5 mins
static const byte NUM_NTP_SERVERS 								= 5;
static const byte MAX_RETRIES											= 20;
static const unsigned int RETRY_DELAY							= 1000 / MAX_RETRIES;							// Allow up to 1s for NTP server to provide time
static const byte NEXT_SERVER_DELAY								= 5;															// Nominal pause
static const unsigned int REFRESH_CYCLE						= 1000;														// Refresh every 1s
static const unsigned long MAX_DISCREPANCY 				= 30;															// Nummber of seconds discrepancy allowed between sampled time to count as the same

static const unsigned long CURRENT_YEAR 					= (2013 - 1970) * SECS_PER_YEAR;  // used for sense test
static const unsigned long EXPIRY_YEAR 						= (2050 - 1970) * SECS_PER_YEAR;  // Used for sense test
static const unsigned long SEVENTY_YEARS 					= 2208988800UL;    							  // In Arduino Time format time starts on Jan 1 1970. In seconds, that's 2208988800 after 1900 (NTP time) 

static const byte NEW_SEARCH 				= 0;
static const byte CALL_SERVER					= 1;
static const byte TEST_RESPONSE			= 2;
static const int NTP_PACKET_SIZE 		= 48; 				// NTP time stamp is in the first 48 bytes of the message
 
struct contextStruct {									// Used to hold current state of NTP enquiry
	byte mode;														// One of NEW_SEARCH, CALL_SERVER, TEST_RESPONSE
  byte retryCount;											// Countdown for retries - when 0 or valid packet received then move on to next server
  byte serverIdx;												// Index into timeServer - the current NTP server being checked.  Range 0 - NUM_NTP_SERVERS
};

extern byte timeServer[NUM_NTP_SERVERS][4];
extern unsigned int _refreshIntervalSecs;
extern volatile time_t _latestNtpTime;									// Refreshed every 1s 
extern contextStruct _context;
extern time_t epoch[NUM_NTP_SERVERS];	
extern int _currentYear;
extern time_t _startBST_t, _endBST_t;											// Set by setBSTThresholds; tested by refreshTime


// *********** METHODS ******************

boolean initialiseTime(unsigned int port, unsigned int refreshIntervalMS);
boolean initialiseTime(unsigned int port);

void refreshTime(void *dummy);
time_t getNtpTime();													// Call to get the latest time

void askNtpServers(void *dummy);							// Call with value of NEW_SEARCH to initiate 
boolean gotResponse();
void calcTime();

void sendNTPpacket(IPAddress ntpServer);
time_t timeValue(int hr,int min,int sec,int dy, int mnth, int yr);
void timeToText(time_t tm, char* buffer, int maxLen);
void setBSTThresholds();




#endif
