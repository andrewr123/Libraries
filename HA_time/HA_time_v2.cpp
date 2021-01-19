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
	Nov 20 - v3 - simplify to single NTP server - no polling/voting etc
*/

/*

#include "HA_time_v2.h"


  

/************ NTP TIME STUFF ************

EthernetUDP UdpNTP;                    	// Socket for getting the time

byte timeServer[4] = { 192, 168, 7, 20 };       // Pandora
unsigned int _refreshIntervalSecs;
volatile time_t _latestNtpTime;									// Refreshed every 1s 
byte _mode;                                       // One of NEW_SEARCH, TEST_RESPONSE
byte _retryCount;
time_t _epoch;
int _currentYear;																// Change in year causes re-calc of BST thresholds
time_t _startBST_t, _endBST_t;										// Start and end thresholds for this year
  



boolean initialiseTime(unsigned int port, unsigned int refreshIntervalSecs) {
	SAVE_CONTEXT("Tim1")
	
	UdpNTP.begin(port);   														// Open up UDP socket
	_refreshIntervalSecs = refreshIntervalSecs;
	_latestNtpTime = 0;
	_currentYear = 0;
	_startBST_t = 0;
	_endBST_t = 0;
	
	// Kickoff regular timecheck
	_mode = NEW_SEARCH;
	askNtpServer(NULL);
  
	// Tell Time.cpp where to go for the time
	setSyncProvider(getNtpTime);
  
	// Wait a while to ensure time stabilises
	for (int i = 0; (i < MAX_RETRIES) && (timeStatus() == timeNotSet); i++) {
		delay(RETRY_DELAY);						// Wait a bit
		wakeup.runAnyPending();				// See if anything's happened
		now();												// Set time status
	}
  
	if (timeStatus() != timeSet) SENDLOG('N', "Time not set", timeStatus());
  
	// Set the start and end times this year for BST
	setBSTThresholds();
  
	// Kickoff regular refresh of time 
	wakeup.wakeMeAfter(refreshTime, REFRESH_CYCLE, (void*)NULL, TREAT_AS_ISR | REPEAT_COUNT);						
  
	RESTORE_CONTEXT
  
	return timeStatus() == timeSet;
}

boolean initialiseTime(unsigned int port) {
	initialiseTime(port, REFRESH_INTERVAL_SECS);
}

void refreshTime(void *dummy) {													// Update time every 1s.  16 bit value, but runs as ISR, so no need to turn interrupts off 
	_latestNtpTime++;  																		// Also adjusted every REFRESH_INTERVAL_SECS by askNtpServer
}

time_t getNtpTime() {																		// Called by now() (Time.cpp)
	return (_latestNtpTime > _startBST_t && _latestNtpTime < _endBST_t) ? _latestNtpTime + SECS_PER_HOUR : _latestNtpTime;
}

void askNtpServer(void *dummy) {												// Re-entrant routine to get time from NTP server. Updates _latestNtpTime on conclusion

	SAVE_CONTEXT("askNTP")
			
	switch (_context.mode) {
		case NEW_SEARCH:																									// First call, and thereafter every _refreshIntervalSecs
			_epoch = 0;
			sendNTPpacket((IPAddress)timeServer);   		// Send request for time
			_retryCount = 0;
			_mode = TEST_RESPONSE;																	// Next time round check for a response
			wakeup.wakeMeAfter(askNtpServer, RETRY_DELAY, NULL, TREAT_AS_NORMAL);							// Queue request
			break;
		case TEST_RESPONSE:																																// See if the time server has responded
			if (gotResponse()) calcTime();
			else {																																					// Nothing yet.  See if our patience has run out
				if (++(_context.retryCount) < MAX_RETRIES) {																	// Go to sleep then check response												
					wakeup.wakeMeAfter(askNtpServer, RETRY_DELAY, NULL, TREAT_AS_NORMAL);	
				}
				else {																																				// Patience exceeded; record a null reading
					_epoch = 0;
					calcTime();
				}
			}
			break;
		default: SENDLOG('E', "Invalid mode", _context.mode);
	}
	RESTORE_CONTEXT
}

boolean gotResponse() {
	if (UdpNTP.parsePacket()) {																											// See if got something
	  byte packetBuffer[ NTP_PACKET_SIZE]; 																					// Buffer to hold incoming and outgoing packets 
  
    UdpNTP.read(packetBuffer, NTP_PACKET_SIZE);  																	// Read the packet into the buffer

    // The timestamp starts at byte 40 of the received packet and is four bytes, or two words, long. 
    // First, extract the two words, then combine the two words into a long integer - this is NTP time (seconds since Jan 1 1900)
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    unsigned long secsSince1900 = highWord << 16 | lowWord;     

    // now convert NTP time into Arduino time - subtract seventy years:
    _epoch = secsSince1900 - SEVENTY_YEARS;

    // Do a rough credibility test - check it's after this software was built and before the end of time
    if ((_epoch < CURRENT_YEAR) || (_epoch > EXPIRY_YEAR)) _epoch = 0; 					
    
    return true;
  }
  
  return false;
}


void calcTime() {																							// Decide on and store the reading, then schedule the next search			
	SAVE_CONTEXT("calcT");
	
	time_t newTime = 0;														
  
  if (_epoch == 0) {           // No valid time available.  If was valid before, then just add refresh interval
      if (_latestNtpTime) newTime = _latestNtpTime + (unsigned long)_refreshIntervalSecs;
      SENDLOGM('W', "No NTP servers");
  }
  else {
			newTime = _epoch;		
	}

	SENDLOG('I', "NTP time = ", newTime);
  
	// Update master time variable - protect from interrupts, as not running inside ISR
	byte oldSREG = SREG;
	cli();
	_latestNtpTime = newTime;
	SREG = oldSREG;
	
	// Check if the year has changed and, if so, calculate the BST thresholds
	if (_currentYear != year()) {
		_currentYear = year();
  	setBSTThresholds();
	}
			
	// Schedule next search
	_mode = NEW_SEARCH;
	wakeup.wakeMeAfter(askNtpServer, _refreshIntervalSecs, (void*)NULL, TREAT_AS_NORMAL | UNITS_SECONDS);	
	
	// Reset refresh cycle
	wakeup.resetWakeup(refreshTime, REFRESH_CYCLE, (void*)NULL, TREAT_AS_ISR | REPEAT_COUNT);
	
	RESTORE_CONTEXT
}


// send an NTP request to the time server
void sendNTPpacket(IPAddress ntpServer) {
	
  SAVE_CONTEXT("sNTP")
	
  byte packetBuffer[ NTP_PACKET_SIZE]; 		//buffer to hold incoming and outgoing packets 
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 		
  UdpNTP.beginPacket(ntpServer, 123); //NTP requests are to port 123
  if (UdpNTP.write(packetBuffer, NTP_PACKET_SIZE) != NTP_PACKET_SIZE) SENDLOG('E', "Pkt", " size")
  UdpNTP.endPacket(); 
  
  RESTORE_CONTEXT
}	

time_t timeValue(int hr,int min,int sec,int dy, int mnth, int yr){
 // Adapted from timeNTP library
 // year can be given as full four digit year or two digts (2010 or 10 for 2010);  
 //it is converted to years since 1970
 
  tmElements_t tm;
 
  if( yr > 99)
      yr = yr - 1970;
  else
      yr += 30;  
  tm.Year = yr;
  tm.Month = mnth;
  tm.Day = dy;
  tm.Hour = hr;
  tm.Minute = min;
  tm.Second = sec;
  return makeTime(tm);
}   

void timeToText(time_t tm, char* buffer, int maxLen){
  char time[9];
  const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  const char days[]="SunMonTueWedThuFriSat";
  char monChar[4];
  char dayChar[4];
  strncpy(monChar, months + (3 * (month(tm) - 1)), 3);
  strncpy(dayChar, days + (3 * (weekday(tm) - 1)), 3);
  monChar[3] = '\0';
  dayChar[3] = '\0';
  sprintf(time, "%02d%s%d%d%s%d%d", hour(tm), ":", minute(tm) / 10, minute(tm) % 10, ":", second(tm) / 10, second(tm) % 10);
  snprintf(buffer, maxLen, "%s%s%02d%s%s%s%d%s%s%s", dayChar, ", ", day(tm), " ", monChar, " ", year(tm), " ", time, " GMT");
}

void setBSTThresholds() {
  _startBST_t = timeValue(1,0,0,31, 3, _currentYear);      										// Get 01:00:00 hrs on 31 Mar of current year
  _startBST_t = _startBST_t - (weekday(_startBST_t) - 1) * SECS_PER_DAY;    	// Deduct seconds to the preceding Sunday
  _endBST_t = timeValue(1,0,0,31, 10, _currentYear);      										// Repeat for 31 Oct
  _endBST_t = _endBST_t - (weekday(_endBST_t) - 1) * SECS_PER_DAY;  
}

*/