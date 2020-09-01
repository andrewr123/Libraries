 /*
    Copyright (C) 2011  Andrew Richards
 
    Part of home automation suite
    
    Contains the HA_web class for handling dialogue with browser

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

 
#ifndef HA_web_h
#define HA_web_h

#include "Arduino.h"
#include "HA_globals.h"
#include "HA_time.h"
#include "HA_syslog.h"
#include <Ethernet.h>
#include <SPI.h>
#include <SD.h>
#include "HA_queue.h"
//#include "HA_device_bases.h"


//************ PROPERTIES ************

class HA_web {
	
	public:
		void init();																																	// Start up HTTP server
		EthernetClient available();																						// Test if any clients
		void processHTTP(EthernetClient client);																			// Interpret HTTP dialogue with browser and invoke routines in support
	
	private:
		void serveFile(EthernetClient client, char *clientLine);											// Retrieve file from SD card and serve to browser
		void handleAjaxGet(EthernetClient client, char* actionline, char type);       // Used to process Ajax GET; actionline points to first char after 'R', 'T' or 'P' 
		void handleHTTPCmd(EthernetClient client, char* actionline);
		void stopClient(EthernetClient client);
		
		static const unsigned int HTTP_BUFSIZE = 100;
		static const unsigned int ELEM_BUFSIZE = 15;
};

//extern EthernetServer server;

  


// *********** METHODS ******************


extern HA_queue changeList;
extern HA_web web;

#endif
