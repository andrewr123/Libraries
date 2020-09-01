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

#ifndef HA_globals_h
#define HA_globals_h

#include "Arduino.h"


// *************** IP and other comms addresses - statics here, variables in HA_globals.cpp *************

#define PITTSFOLLY

#ifdef PITTSFOLLY
	const static unsigned int syslogServerPort = 514;		
#else
	const static unsigned int syslogServerPort = 514;
#endif



const static unsigned int UdpNTPPort = 8888;      					// port for NTP time comms
const static unsigned int UdpLogPort = 8889;      					// port for syslog (in practice not used, as syslog is send only)
const static unsigned int UdpArdPort = 8890;						// port for comms with other Arduinos and with console
const static unsigned int REPLY_PORT = 8891;						// Responses to status requests go to this port on requestor IP

const static byte NUM_PINGS				= 4;

extern byte diningIP[];
extern byte basementIP[];
extern byte studyIP[];
extern byte testIP[];
extern byte syslogServerIP[];
extern byte gatewayIP[];
extern byte pingAddr[NUM_PINGS][4];
extern byte diningMac[];
extern byte boilerMac[];
extern byte basementMac[];
extern byte studyMac[];
extern byte testMac[];

// *********** VARIABLES ******************

extern char *globalContext;
extern byte syslogLevel;
extern boolean sendToSyslog;
// extern unsigned int heartBeat;
extern char meName[];


// *********** CONSTANTS *****************

const unsigned int HEARTBEAT_MS 					= 1000;				// Frequency of checking things
const unsigned int CHECK_FREQ_OPEN 				= 2;					// 
const unsigned int CHECK_FREQ_TIME				= 10;					// Send browser revised time every 10 secs
const unsigned int LOG_FREQ_TIME					= 30;					// Send timing log every 30 secs

const static unsigned int STATUS_STABLE = 0; 					// Valid reading
const static unsigned int STATUS_PENDING = 1;         // Reading initiated; awaiting response
const static unsigned int STATUS_READY = 2;  					// Device initialised, but no reading
const static unsigned int STATUS_UNAVAILABLE = 3;  		// Device not ready/inaccessible/other error

const static byte SD_CS_PIN = 4;
const static byte SPI_CS_PIN = 53;

const static byte MASK_BYTE = 0xFF;
const static byte MASK_MS_NIBBLE = 0xF0;
const static byte MASK_LS_NIBBLE = 0x0F;
const static byte OFFSET_MS_NIBBLE = 4;

const byte ARDUINO_VOLTAGE = 5;
const int ANALOG_RANGE = 1024;

const static byte VAL_TO_INT = 0;
const static byte VAL_TO_CHAR = 1;
const static unsigned int OFF = 0;
const static unsigned int ON = 1;

const static byte NUMREGIONCODES = 6;

const static byte READ_FLAG = 0x01;
const static byte WRITE_FLAG = 0x02;
const static byte VAL_DAY = 0x01;
const static byte VAL_HOUR = 0x02;
const static byte VAL_MINUTE = 0x03;

const static byte MASK_MSB = 0x80;
const static byte MASK_LSB = 0x01;
const static unsigned int MASK_2BYTE_MSB = 0x8000;

const static byte MAX_HANDLERS 					= 4;
const static byte MAX_PINS 							= 128;
const static byte MAX_CHANNELS 					= 16;
const static byte NUM_INTERRUPT_PORTS 	= 6;						// On a Mega

const byte OFFSET_DAY = 11;
const byte OFFSET_HOUR = 6;
const byte OFFSET_MIN = 0;
const unsigned int MASK_DAY = B1111 << OFFSET_DAY;     	// 4 bits: 0 = everyday, 1-7 are days of week (1 = Sun), 8 = Mon-Thu, 9 = Mon-Fri, 10 = Sat/Sun.  
																												// Range 8-14 also used to represent days in the next week in dhmAccess()
const unsigned int MASK_HOUR = B11111 << OFFSET_HOUR;   // 5 bits: 0-23 are hours
const unsigned int MASK_MIN = B111111 << OFFSET_MIN;    // 6 bits: 0-59 are minutes

// **************** Device types *********************
// Check HA_root and HA_variables before amending

const static byte DEV_TYPE_TOUCH 			= 0;
const static byte DEV_TYPE_FIRE 			= 1;
const static byte DEV_TYPE_HEAT 			= 2;
const static byte DEV_TYPE_LUMINANCE 	= 3;
const static byte DEV_TYPE_MOTION 		= 4;
const static byte DEV_TYPE_PRESENCE 	= 5;
const static byte DEV_TYPE_RFID 			= 6;
const static byte DEV_TYPE_OPEN 			= 7;
const static byte DEV_TYPE_5APWR 			= 8;
const static byte DEV_TYPE_13APWR 		= 9;
const static byte DEV_TYPE_LOCK 			= 10;
const static byte DEV_TYPE_LIGHT 			= 11;
const static byte DEV_TYPE_RELAY 			= 12;
const static byte DEV_TYPE_NULL 			= 13;						// Used in switcher to indicate no device
const static byte VAR_TYPE_BYTE 		  = 13;				
const static byte VAR_TYPE_2BYTE 		  = 14;
const static byte VAR_TYPE_RFID 		  = 15;		
const static byte OBJ_TYPE_ZONE				= 16;
const static byte OBJ_TYPE_TIME				= 17;
const static byte OBJ_TYPE_HEARTBEAT 	= 18;

const static byte NUM_DEV_TYPES 			= 13;	
const static byte NUM_VAR_TYPES       = 3;
const static byte NUM_ZONE_TYPES			= 1;
const static byte NUM_OBJ_TYPES				= 2;

const static byte MAX_VARS 						= 64;                           // 127 is limit

const static unsigned int BINARY_DEV = (B11111 * 256) + B00110011;			// Flags indicating which devices are binary


const static byte PIN_NOOP = 						127;



// Serial port numbers
const static byte S0 = 0;
const static byte S1 = 1;
const static byte S2 = 2;
const static byte S3 = 3;

// Range numbers
const static byte R0 = 0;
const static byte R1 = 1;
const static byte R2 = 2;
const static byte R3 = 3;

// Interrupt numbers
const static byte Int0	= 0;
const static byte Int1	= 1;
const static byte Int2	= 2;
const static byte Int3	= 3;
const static byte Int4	= 4;
const static byte Int5	= 5;

// Channel numbers
const static byte CH0	= 0;
const static byte CH1	= 1;
const static byte CH2	= 2;
const static byte CH3	= 3;
const static byte CH4	= 4;
const static byte CH5	= 5;
const static byte CH6	= 6;
const static byte CH7	= 7;
const static byte CH8	= 8;
const static byte CH9	= 9;
const static byte CH10	= 10;
const static byte CH11	= 11;
const static byte CH12	= 12;
const static byte CH13	= 13;
const static byte CH14	= 14;
const static byte CH15	= 15;
const static byte CH16	= 16;
const static byte CH17	= 17;
const static byte CH18	= 18;
const static byte CH19	= 19;
const static byte CH20	= 20;
const static byte CH21	= 21;
const static byte CH22	= 22;
const static byte CH23	= 23;
const static byte CH24	= 24;
const static byte CH25	= 25;
const static byte CH26	= 26;
const static byte CH27	= 27;
const static byte CH28	= 28;
const static byte CH29	= 29;
const static byte CH30	= 30;
const static byte CH31	= 31;

// Pin numbers
const static byte P0	= 0;
const static byte P1	= 1;
const static byte P2	= 2;
const static byte P3	= 3;
const static byte P4	= 4;
const static byte P5	= 5;
const static byte P6	= 6;
const static byte P7	= 7;
const static byte P8	= 8;
const static byte P9	= 9;
const static byte P10	= 10;
const static byte P11	= 11;
const static byte P12	= 12;
const static byte P13	= 13;
const static byte P14	= 14;
const static byte P15	= 15;
const static byte P16	= 16;
const static byte P17	= 17;
const static byte P18	= 18;
const static byte P19	= 19;
const static byte P20	= 20;
const static byte P21	= 21;
const static byte P22	= 22;
const static byte P23	= 23;
const static byte P24	= 24;
const static byte P25	= 25;
const static byte P26	= 26;
const static byte P27	= 27;
const static byte P28	= 28;
const static byte P29	= 29;
const static byte P30	= 30;
const static byte P31	= 31;
const static byte P32	= 32;
const static byte P33	= 33;
const static byte P34	= 34;
const static byte P35	= 35;
const static byte P36	= 36;
const static byte P37	= 37;
const static byte P38	= 38;
const static byte P39	= 39;
const static byte P40	= 40;
const static byte P41	= 41;
const static byte P42	= 42;
const static byte P43	= 43;
const static byte P44	= 44;
const static byte P45	= 45;
const static byte P46	= 46;
const static byte P47	= 47;
const static byte P48	= 48;
const static byte P49	= 49;
const static byte P50	= 50;
const static byte P51	= 51;
const static byte P52	= 52;
const static byte P53	= 53;
const static byte P54	= 54;
const static byte P55	= 55;
const static byte P56	= 56;
const static byte P57	= 57;
const static byte P58	= 58;
const static byte P59	= 59;
const static byte P60	= 60;
const static byte P61	= 61;
const static byte P62	= 62;
const static byte P63	= 63;
const static byte P64	= 64;
const static byte P65	= 65;
const static byte P66	= 66;
const static byte P67	= 67;
const static byte P68	= 68;
const static byte P69	= 69;
const static byte P70	= 70;
const static byte P71	= 71;
const static byte P72	= 72;
const static byte P73	= 73;
const static byte P74	= 74;
const static byte P75	= 75;
const static byte P76	= 76;
const static byte P77	= 77;
const static byte P78	= 78;
const static byte P79	= 79;
const static byte P80	= 80;
const static byte P81	= 81;
const static byte P82	= 82;
const static byte P83	= 83;
const static byte P84	= 84;
const static byte P85	= 85;
const static byte P86	= 86;
const static byte P87	= 87;
const static byte P88	= 88;
const static byte P89	= 89;
const static byte P90	= 90;
const static byte P91	= 91;
const static byte P92	= 92;
const static byte P93	= 93;
const static byte P94	= 94;
const static byte P95	= 95;
const static byte P96	= 96;
const static byte P97	= 97;
const static byte P98	= 98;
const static byte P99	= 99;
const static byte P100	= 100;
const static byte P101	= 101;
const static byte P102	= 102;
const static byte P103	= 103;
const static byte P104	= 104;
const static byte P105	= 105;
const static byte P106	= 106;
const static byte P107	= 107;
const static byte P108	= 108;
const static byte P109	= 109;
const static byte P110	= 110;
const static byte P111	= 111;
const static byte P112	= 112;
const static byte P113	= 113;
const static byte P114	= 114;
const static byte P115	= 115;
const static byte P116	= 116;
const static byte P117	= 117;
const static byte P118	= 118;
const static byte P119	= 119;
const static byte P120	= 120;
const static byte P121	= 121;
const static byte P122	= 122;
const static byte P123	= 123;
const static byte P124	= 124;
const static byte P125	= 125;
const static byte P126	= 126;
const static byte P127	= 127;


// *********** METHODS *******************

void stoupper(char *s);		// @author: Mathias Van Malderen (tux4life)
void stolower(char *s);

// ************** Device methods ***************

byte getDevTypeIdx (char *device);
void getDevTypeChar (byte devIdx, char *devChar);

// ********** dhm time methods

unsigned int dhmGet(unsigned int dhmVal, byte type);
void dhmPut(unsigned int *dhmVal, byte type, unsigned int value);
unsigned int dhmMake(unsigned int dhmDay, unsigned int dhmHour, unsigned int dhmMinute);
unsigned int dhmAccess(unsigned int *dhmVal, byte type, unsigned int value, int flag);
void dhmToText(unsigned int dhm, char *responseText);
boolean dhmBetween(unsigned int dhmVal, unsigned int dhmFrom, unsigned int dhmTo);

#endif


// Types.h - for getting round IDE compile problem
// See http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1245557964
