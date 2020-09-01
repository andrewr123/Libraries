 /*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
       
    Contains the HA_comms class for communicating between Arduinos

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

#ifndef HA_comms_h
#define HA_comms_h

#include "Arduino.h"
#include "HA_globals.h"
#include <inttypes.h>
#include <IPAddress.h>
#include <EthernetUdp.h>

class HA_comms {
	
	public:
		void init(unsigned int myPort);
		unsigned int get(char *message, byte size);
		void put(IPAddress IP, unsigned int port, char message);
		void put(IPAddress IP, unsigned int port, char *message, byte size);
		void reply(unsigned int port, char message);
		void reply(unsigned int port, char *message, byte size);
	    
	private:
		boolean validIP(IPAddress testIP);
		
	  IPAddress _remoteIP;
	  unsigned int _remotePort;
};

static const byte NUM_VALID_IPS = 5;

extern HA_comms ard;
extern byte validIPs[NUM_VALID_IPS][5];



#endif