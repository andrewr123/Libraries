 /*
    Copyright (C) 2011  Andrew Richards
 
    Part of home automation suite
    
    Contains the HA_zone class 

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
    
    Class structure as follows:
    - one base class for all devices (HA_device)
    - three abstractions from that to hold the data (HA_devBit, HA_dev2Byte, HA_devMultiByte)
    - three abstractions from those to represent different device types (HA_sensAnalog, HA_sensDigital, HA_actDigital)
*/

 
#ifndef HA_zones_h
#define HA_zones_h

#include "HA_globals.h"
#include "HA_device_bases.h"
#include "HA_variables.h"



class HA_zone {					// Instantiated as an array containing all zones on this Arduino (max 256)
public:
	// Methods
	HA_zone();
	~HA_zone();
	
	void put (byte type, unsigned int val);
	unsigned int get(byte type);
	void getSnapshot();
  	
protected:		
	void handleOccupancyTimeout(byte dummy);			// Called by switcher on expiry of occupancy timer - turns light off
	void handleEvent(byte state);														// Sets one or more relays on or off, dependent on event

	// Properties
	byte _regionZone;
	byte _flags;												// Occupancy and handler
	unsigned int _targetTemp;
	unsigned int _actualTemp;
	unsigned int _luminance;
	byte  _context;											// Index in contextTable (see HA_switcher.h) for occupancy counter
	byte _onEvent;											// Argument to use when occupancy changes.  Either a direct relay number (if Handler == 0), or ArgList number containing a list of relays (if Handler == 1)
	byte _zoneNum;
	
	const static byte MASK_REGION 		= B00111000;
	const static byte MASK_ZONE 			= B00000111;
	const static byte MASK_OCCUPANCY 	= B00000001;
	const static byte MASK_HANDLER 		= B00000110;
	
	const static byte OFFSET_REGION = 3;
	const static byte OFFSET_ZONE = 0;
	const static byte OFFSET_OCCUPANCY = 0;
	const static byte OFFSET_HANDLER = 1;
	
	const static byte ZONE_OCCUPANCY_TIMEOUT = 5;			// Minutes
};


typedef void (HA_zone::*HA_zoneMemPtr)(byte);


extern const char REGIONCODES[NUMREGIONCODES + 1];


#endif
