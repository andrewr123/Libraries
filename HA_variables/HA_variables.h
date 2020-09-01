 /*
    Copyright (C) 2011  Andrew Richards
 
    Part of home automation suite
    
    Contains the HA_variables library to manage the internal variables

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

 
#ifndef HA_variables_h
#define HA_variables_h

#include "HA_globals.h"

/*
// **************** Global constants *****************

const static byte MAX_VARS = 64;                           // 127 is limit

// Check HA_root before amending

const static byte VAR_TYPE_BYTE = 13;
const static byte VAR_TYPE_2BYTE = 14;
const static byte VAR_TYPE_RFID = 15;		
const static byte NUM_VAR_TYPES = 3;
*/
// ***************** Global methods *******************

unsigned int dhmGet(unsigned int dhmVal, byte type);
void dhmPut(unsigned int *dhmVal, byte type, unsigned int value);
unsigned int dhmNow();
unsigned int dhmMake(unsigned int dhmDay, unsigned int dhmHour, unsigned int dhmMinute); 
unsigned int dhmAccess(unsigned int *dhmVal, byte type, unsigned int value, int flag); 


class HA_varByte {				
public:
	// Methods
	void put (unsigned int value);
	unsigned int get();
		
protected:	
  // Properties
	byte _variable;
};

class HA_var2Byte {				
public:
	// Methods
	void put (unsigned int value);
	unsigned int get();
		
protected:	
  // Properties
	unsigned int _variable;
};

class HA_varMultiByte {
	public:
		HA_varMultiByte();
		HA_varMultiByte(byte bufLen);
		~HA_varMultiByte();
		
		boolean create(byte bufLen);
		void init(byte *addr, byte bufLen);
		void freeMem();
		byte bufLen();
		
		void put(byte *val);
	  void get(byte *val);
	  byte *getBufPtr();
	  
	  void putElem(byte elemIdx, byte elemVal);
	  byte getElem(byte elemIdx);
	  
	protected:
		byte _bufLen;
		byte *_bufVar;
};

class HA_varRFID : public HA_varMultiByte {
	public: 
		static const byte _bufLen = 16;
		HA_varRFID();
		boolean create();
		void init(byte *addr);
};


#endif