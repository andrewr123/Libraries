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


#include "HA_syslog.h"
//#include "HA_queue.h"


EthernetUDP SyslogUdp;
static const char severityCodes[] = "XACEWNID";				// See http://en.wikipedia.org/wiki/Syslog	


void HA_syslog::init(unsigned int myPort, byte* farIP, unsigned int farPort, char syslogLevel) {
	init(myPort, (IPAddress)farIP, farPort, syslogLevel);
}

void HA_syslog::init(unsigned int myPort, IPAddress farIP, unsigned int farPort, char syslogLevel) {
    ip_syslogserver = farIP;
    syslogPort = farPort;
    SyslogUdp.begin(myPort);  
    _syslogLevel = strchr(severityCodes, syslogLevel) - severityCodes;
    _contextStack.create(30);
}

void HA_syslog::adjustLevel(char sev) {	
	_syslogLevel = strchr(severityCodes, sev) - severityCodes;
}

void HA_syslog::setContext(char *localContext){
	_contextStack.push((unsigned int)localContext);
}

char* HA_syslog::getContext(){
	return (char*)_contextStack.popInt();
}

void HA_syslog::getContext(char *buffer, int maxLen) {
	_contextStack.peekAll(buffer, maxLen);
}
 
void HA_syslog::sendLog(char sev, char *tag, char *message) {	
	char buffer[UDP_TX_PACKET_MAX_SIZE];
	int bufPosn;
	char path[30];
	const byte facility = 1;														// Default - user level messages 
	byte severity = strchr(severityCodes, sev) - severityCodes;
	
	if (severity > _syslogLevel) return;								// Exit if lower priority than threshold

	_contextStack.peekAll(path, 30);

	bufPosn = snprintf(buffer, UDP_TX_PACKET_MAX_SIZE, "<%d>%s @%s %s %s", (facility << 3) | severity, meName, path, tag, message);

	if (bufPosn > 0) {
		SyslogUdp.beginPacket(ip_syslogserver, syslogPort);
		SyslogUdp.write((byte*)buffer, bufPosn);
		SyslogUdp.endPacket();
	}
}

void HA_syslog::sendLog(char sev, char *message) {
	sendLog(sev, NULL, message);
}

void HA_syslog::sendLog(char sev, char *tag, IPAddress IPAddr) {
	char buffer[20];
	snprintf(buffer, 20, "%d%c%d%c%d%c%d", IPAddr[0], '.', IPAddr[1], '.', IPAddr[2], '.', IPAddr[3] );
	sendLog(sev, tag, buffer);
}

void HA_syslog::sendLog(char sev, char *tag, unsigned long num) {
	char buffer[20];
	snprintf(buffer, 20, "%lu", num);
	sendLog(sev, tag, buffer);
}

void HA_syslog::sendLog(char sev, char *tag, byte num) {
	char buffer[20];
	snprintf(buffer, 20, "%x", num);
	sendLog(sev, tag, buffer);
}

void HA_syslog::sendLog(char sev, char *tag, unsigned int num) {
	char buffer[20];
	snprintf(buffer, 20, "%u", num);
	sendLog(sev, tag, buffer);
}

void HA_syslog::sendLog(char sev, char *tag, int num) {
	char buffer[20];
	snprintf(buffer, 20, "%d", num);
	sendLog(sev, tag, buffer);
}

void HA_syslog::sendLog(char sev, char *tag, unsigned int num, char *format) {
	char buffer[20];
	snprintf(buffer, 20, format, num);
	sendLog(sev, tag, buffer);
}

/* Create one global object */
HA_syslog syslog;



	

