 /*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
       
    Contains the HA_comms class for communicating between Arduinos, 
    and between Arduinos and UDP clients

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


#include "HA_comms.h"
#include "HA_syslog.h"



EthernetUDP UdpArd;


byte validIPs[NUM_VALID_IPS][5] = {	{192, 168, 7, 20},
									{192, 168, 2, 60},
									{192, 168, 7, 74},
									{192, 168, 7, 178}
								  };

 
void HA_comms::init(unsigned int myPort) {
    UdpArd.begin(myPort);  
}

unsigned int HA_comms::get(char *message, byte size) {

	unsigned int numBytes;
	
	SAVE_CONTEXT("Ard1")
	
	// If there's data available, then read it
	if (numBytes = UdpArd.parsePacket()) {
		_remoteIP = UdpArd.remoteIP();
		_remotePort = UdpArd.remotePort();

		// Check it's from a known source
		if (validIP(_remoteIP)) {

			// Clear the buffer then read the packet into packetBufffer
			memset(message, '\0', size);
			UdpArd.read(message, size);  		// read the packet into the buffer
		}
		else {
			SENDLOG('W', "Bad IP: ", _remoteIP)
			numBytes = 0;
		}
	}

	RESTORE_CONTEXT

	return numBytes;
}

 

boolean HA_comms::validIP(IPAddress testIP) {
	return true;						// <<<
	for (int i = 0; i < NUM_VALID_IPS; i++) if ((IPAddress)validIPs[i] == testIP) return true;
	return false;
}

void HA_comms::put(IPAddress IP, unsigned int port, char message) {
    // Send message 
    UdpArd.beginPacket(IP, port); 
    UdpArd.write(message);
    UdpArd.endPacket();
}

void HA_comms::put(IPAddress IP, unsigned int port, char *message, byte size) {
	// Send message 
	if (size > 0) {
		UdpArd.beginPacket(IP, port);
		UdpArd.write(message, size);
		UdpArd.endPacket();
	}
}

void HA_comms::reply(unsigned int port, char message) {
	put(_remoteIP, port, message);
}

void HA_comms::reply(unsigned int port, char *message, byte size) {
	put(_remoteIP, port, message, size);
}

// Create global object
HA_comms ard;

