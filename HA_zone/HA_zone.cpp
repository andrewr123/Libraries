 /*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
    
    Contains the HA_zone library

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


#include "Arduino.h"
#include "wakeup.h"
#include "HA_switcher.h"
#include "HA_zone.h"
#include "HA_channels.h"
#include "HA_root.h"

// *********** HA_zone *********************

void HA_zone::put(byte type, unsigned int val) {
	byte oldOccupancy = get(VAL_OCCUPANCY);
	
	// Vars used for countdown
	int contextNum;
	HA_zoneMemPtr fPtr = &HA_zone::handleOccupancyTimeout;
	
	switch (type) {
		case VAL_REGION: 						_regionZone &= ~MASK_REGION; _regionZone |= (((strchr(REGIONCODES, (char)val) - REGIONCODES) << OFFSET_REGION) & MASK_REGION); break;
		case VAL_ZONE:							_regionZone &= ~MASK_ZONE; _regionZone |= (((val - 1) << OFFSET_ZONE) & MASK_ZONE); break;			// Stored value is 1 less than published
		case VAL_REGION_ZONE:				_regionZone = val; break;
		case VAL_HANDLER:						_flags &= ~MASK_HANDLER; _flags |= (val << OFFSET_HANDLER) & MASK_HANDLER; break;
		case VAL_ON_EVENT:					_onEvent = val; break;
		case VAL_OCCUPANCY:		
			_flags &= ~MASK_OCCUPANCY;
			_flags |= (val << OFFSET_OCCUPANCY) & MASK_OCCUPANCY;
			
			if (val == ON) {
				if (oldOccupancy == ON) {																																	// Was ON previously, which means countdown already started, so needs resetting
					wakeup.resetWakeup(switcher, ZONE_OCCUPANCY_TIMEOUT, (void*)_context, UNITS_SECONDS);		// Context was saved when counter first started
				}
				else {			
					// Set a delay after which the zone is deemed unoccupied 
					if ((contextNum = saveContext(this, fPtr)) < 0) {  		// Set 'this' as the object instance and handleOccupancyTimeout as the member function 
					 	Serial.println("out of stack");
					  break;
				 	}
				 	else _context = contextNum; 								// Save to allow for reset later, if occupancy repeated
	
			  	if (!wakeup.wakeMeAfter(switcher, ZONE_OCCUPANCY_TIMEOUT, (void*)_context, TREAT_AS_NORMAL | UNITS_SECONDS)) Serial.println("Queue full");  	// Queue timeout
		  		
		  		// React to On event - enable device or list of devices
		  		handleEvent(ON);
				}
				
				Serial.print("Zone ");
				Serial.print((char)get(VAL_REGION));
				Serial.print(get(VAL_ZONE));
				Serial.println(" occupied");		
			}
			else {				
				// React to Off event - turn off device or list of devices
				handleEvent(OFF);
				 	
				Serial.print("Zone ");
				Serial.print((char)get(VAL_REGION));
				Serial.print(get(VAL_ZONE));
			  Serial.println(" unoccupied");
		  }
			break;
		case VAL_TARG_TEMP:					_targetTemp = val; break;
		case VAL_ACT_TEMP:					_actualTemp = val; break;
		case VAL_LUMINANCE:					_luminance = val; break;
		case VAL_CONTEXT:						_context = val; break;
		case VAL_DEVIDX:						_zoneNum = val; break;
		default:										Serial.print("Bad type1:"); 
	}
}

unsigned int HA_zone::get(byte type) {
	switch (type) {
		case VAL_REGION:						return REGIONCODES[(_regionZone & MASK_REGION) >> OFFSET_REGION];
		case VAL_ZONE:							return ((_regionZone & MASK_ZONE) >> OFFSET_ZONE) + 1;
		case VAL_REGION_ZONE:				return _regionZone;
		case VAL_HANDLER:						return (_flags & MASK_HANDLER) >> OFFSET_HANDLER;
		case VAL_ON_EVENT:					return _onEvent; 
		case VAL_OCCUPANCY:					return (_flags & MASK_OCCUPANCY) >> OFFSET_OCCUPANCY;
		case VAL_TARG_TEMP:					return _targetTemp;
		case VAL_ACT_TEMP:					return _actualTemp;
		case VAL_LUMINANCE:					return _luminance;
		case VAL_CONTEXT:						return _context;
		case VAL_DEVIDX:						return _zoneNum;
		default:										Serial.print("Bad type2:");
	}
}

void HA_zone::handleOccupancyTimeout(byte dummy) {			// Called on completion of occupancy timeout
  put(VAL_OCCUPANCY, OFF); 
}

void HA_zone::handleEvent(byte state) { 			// Respond to event by setting/unsetting device/list of devices
	if (get(VAL_HANDLER) == 0) {								// Direct call
	 	root.setDev(DEV_TYPE_RELAY, get(VAL_ON_EVENT), state);
 	}
 	else {
 		byte argListNum = get(VAL_ON_EVENT);
 		for (int i = 0; i < root.numArgs(argListNum); i++) root.setDev(DEV_TYPE_RELAY, root.getArg(argListNum, i), state);
	}
}

void HA_zone::getSnapshot() { 
  Serial.print("Occupancy = ");
  Serial.print(get(VAL_OCCUPANCY));
  
  Serial.print(", target temp = ");
  Serial.print(get(VAL_TARG_TEMP));
  
  Serial.print(", actual temp = ");
  Serial.print(get(VAL_ACT_TEMP));
  
  Serial.print(", luminance = ");
  Serial.print(get(VAL_LUMINANCE));
  
  Serial.print(", context = ");
  Serial.print(get(VAL_CONTEXT));  
  
  Serial.print(", handler = ");
  Serial.print(get(VAL_HANDLER));
  
  Serial.print(", on event = ");
  Serial.println(get(VAL_ON_EVENT));
}