/*
    Copyright (C) 2012  Andrew Richards, based on original library by 
  	Copyright (C) 2011 Markus Heller - https://code.google.com/p/ardusyslog/
    
    Part of home automation suite
       
    Contains the HA_syslog class for sending UDP messages to a syslog server

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


#ifndef HA_syslog_h
#define HA_syslog_h

#include "Arduino.h"
#include "HA_globals.h"
#include <inttypes.h>
#include <IPAddress.h>
#include <EthernetUdp.h>
#define UDP_TX_PACKET_MAX_SIZE 128			// Override the default 24
#include "HA_queue.h"

/*
#define SYSLOG_DEBUG 7
#define SYSLOG_INFORMATION 6
#define SYSLOG_NOTICE 5
#define SYSLOG_WARNING 4
#define SYSLOG_ERROR 3
#define SYSLOG_CRITICAL 2
#define SYSLOG_ALARM 1
#define SYSLOG_EMERGENCY 0
*/
// ************ Environment definitions - ensure these come before the macros ***********

#define DEBUGON

// ************ Macro definitions *******************	

#ifdef DEBUGON
	#define SAVE_CONTEXT(localContext) 					syslog.setContext(localContext); 
	#define RESTORE_CONTEXT 										syslog.getContext(); 
	#define SENDLOGM(sev, message)							syslog.sendLog(sev, message);
	#define SENDLOG(sev, tag, message) 					syslog.sendLog(sev, tag, message);
	#define SENDLOGF(sev, tag, message, format) syslog.sendLog(sev, tag, message, format);
#else
	#define SAVE_CONTEXT (localContext)
	#define RESTORE_CONTEXT
	#define SENDLOGM(sev, message)
	#define SENDLOG(sev, tag, message)
	#define SENDLOGF(sev, tag, message, format)		
#endif	



class HA_syslog {
	
	public:
			void init(unsigned int myPort, byte* farIP, unsigned int farPort, char syslogLevel);    
			void init(unsigned int myPort, IPAddress farIP, unsigned int farPort, char syslogLevel);
	    void adjustLevel(char sev);
	    void setContext(char *localContext);
	    char* getContext();
	    void getContext(char *buffer, int maxLen);
	    void sendLog(char sev, char *message);
	    void sendLog(char sev, char *tag, char *message); 			
			void sendLog(char sev, char *tag, IPAddress IPAddr);
			void sendLog(char sev, char *tag, unsigned long num);
			void sendLog(char sev, char *tag, byte num);
			void sendLog(char sev, char *tag, unsigned int num);
			void sendLog(char sev, char *tag, int num);
			void sendLog(char sev, char* tag, float num);
			void sendLog(char sev, char *tag, unsigned int num, char *format);

	    
	private:
	    IPAddress ip_syslogserver;
	    unsigned int syslogPort;
	     
	   	byte _syslogLevel; 										// This level or lower (more severe) will be logged.  Set with init; varied with Can be varied dynamically with console message 
			HA_stack _contextStack;								// Trace of function calls to current context
};

extern HA_syslog syslog;



#endif

