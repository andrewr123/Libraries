 /*
    Copyright (C) 2011  Andrew Richards
 
    Part of home automation suite
    
    Contains the HA_device class to manage the directory of devices (sensors or actors)
    HA_devHeat and HA_devMotion held separately

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

 
#ifndef HA_devices_h
#define HA_devices_h

#include "HA_globals.h"
#include "HA_device_bases.h"
#include "HA_channels.h"
#include "OneWire.h"
#include "HA_queue.h"




// ************ HA_devTouch *****************

class HA_devTouch : public HA_sensDigital {
	public:
		void getRef(char *device);
		void getSnapshot();
		boolean logChange(byte val);
};

// ************ HA_devFire *****************

class HA_devFire : public HA_sensDigital {
	public:
		void getRef(char *device);
		void getSnapshot();
		boolean logChange(byte val);
};

// ************ HA_devLuminance *****************

class HA_devLuminance : public HA_sensAnalog {
	public:
		void getRef(char *device);
		void getSnapshot();
		boolean logChange(byte val);
};

typedef void (HA_devLuminance::*HA_devLuminanceMemPtr)(unsigned int context);

// ************ HA_devPresence *****************

class HA_devPresence : public HA_sensDigital {
	public:
		void getRef(char *device);
		void getSnapshot();
		boolean logChange(byte val);
};

// ************ HA_devRFID *****************

class HA_devRFID : public HA_devMultiByte {
	public: 
		HA_devRFID();
		boolean create();
		void init(byte *addr);
		void getRef(char *device);
		void getSnapshot();
		boolean logChange(byte val);
		
		void initDev(byte devNum, byte channel, byte pin = 0, byte handler = 0);
		void readDev();
	
		static const byte _bufLen = 16;
};

// ************ HA_devOpen *****************

class HA_devOpen : public HA_sensAnalog {
	public:
		void getRef(char *device);
		void getSnapshot();
		boolean logChange(byte val);
		
		void readDev(byte devType = 0);
};

typedef void (HA_devOpen::*HA_devOpenMemPtr)(unsigned int context);

// ************ HA_dev5APower *****************

class HA_dev5APower : public HA_actDigital {
	public:
		void getRef(char *device);
		void getSnapshot();
		boolean logChange(byte val);
};

// ************ HA_dev13APower *****************

class HA_dev13APower : public HA_actDigital {
	public:
		void getRef(char *device);
		void getSnapshot();
		boolean logChange(byte val);
};

// ************ HA_devLock *****************

class HA_devLock : public HA_actDigital {
	public:
		void getRef(char *device);
		void getSnapshot();
		boolean logChange(byte val);
};

// ************ HA_devLight *****************

class HA_devLight : public HA_actDigital {
	public:
		void getRef(char *device);
		void getSnapshot();
		boolean logChange(byte val);
};

// ************ HA_devRelay *****************

class HA_devRelay : public HA_actDigital {
	public:
		void getRef(char *device);
		void getSnapshot();
		boolean logChange(byte val);
};

extern HA_queue changeList;

#endif
