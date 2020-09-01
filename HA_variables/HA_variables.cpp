 /*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
    
    Contains the HA_variable class to manage variables; dhm routines to manipulate day-hour-minute variables

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


#include "HA_variables.h"

          

//***********  HA_varByte  *****************


void HA_varByte::put(unsigned int value) { 
  _variable = value; 
}

unsigned int HA_varByte::get() { 
	return _variable; 
}



//***********  HA_var2Byte  *****************

void HA_var2Byte::put(unsigned int value) { 
  _variable = value; 
}

unsigned int HA_var2Byte::get() { 
	return _variable; 
}

// **************** HA_varMultiByte ****************

HA_varMultiByte::HA_varMultiByte () {
}

HA_varMultiByte::HA_varMultiByte (byte bufLen) {
	create(bufLen);
}

HA_varMultiByte::~HA_varMultiByte () {
	freeMem();
}

boolean HA_varMultiByte::create(byte bufLen) {		// To be called directly if constructor not called (if malloc'ed)
	byte *addr = (byte*)malloc(bufLen);				// Get some space
	
	if (addr != NULL) {
		init(addr, bufLen);		// If got space then initialise it
		return true;
	}
	else {
		_bufLen = 0; 
		return false;
	}
}

void HA_varMultiByte::init(byte *addr, byte bufLen) {		
	_bufLen = bufLen;
	_bufVar = addr;			
	memset(addr, 0, _bufLen); 
}

void HA_varMultiByte::freeMem () {
	free(_bufVar);
}

byte HA_varMultiByte::bufLen() {
	return _bufLen;
}

void HA_varMultiByte::put(byte *val) {
	char oldSREG = SREG;                               
  cli();
  
  for (int i = 0; i < _bufLen; i++) _bufVar[i] = val[i];
  
	SREG = oldSREG;														// Restore status register
}

void HA_varMultiByte::get(byte *val) { 
  for (int i = 0; i < _bufLen; i++) val[i] = _bufVar[i];
}

byte *HA_varMultiByte::getBufPtr() {
	return _bufVar;
}

void HA_varMultiByte::putElem(byte elemIdx, byte elemVal) {
	_bufVar[elemIdx] = elemVal; 
}

byte HA_varMultiByte::getElem(byte elemIdx) {
	return _bufVar[elemIdx];
}

// **************** HA_varRFID ****************

HA_varRFID::HA_varRFID() : HA_varMultiByte(_bufLen) {
}

boolean HA_varRFID::create() {
	HA_varMultiByte::create(_bufLen);
}

void HA_varRFID::init(byte *addr) {
	HA_varMultiByte::init(addr, _bufLen);
}



