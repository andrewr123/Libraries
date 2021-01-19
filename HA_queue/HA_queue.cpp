 /*
    Copyright (C) 2013  Andrew Richards
    
    Part of home automation suite
    
    Contains methods to manage data queues

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


#include "HA_queue.h"
#include "HA_syslog.h"
//#include "HA_time.h"


//***************** HA_queue - FIFO (ring) ************************

boolean HA_queue::init(byte *buffer, unsigned int bufLen) {
  _putPtr = 0;
  _getPtr = 0;
  return BYTESTRING::init(buffer, bufLen);
}

boolean HA_queue::create(unsigned int bufLen) {
	_putPtr = 0;
  _getPtr = 0;
	return BYTESTRING::create(bufLen);
}

boolean HA_queue::put(byte val) {
  if (full()) return false;										// Buffer full; any more will overwrite data
  
  BYTESTRING::put(_putPtr % _bufLen, val);		// Save data, rolling over if end of array
  
  _putPtr++; 																	// Increment - overflows if larger than 64k
  
	return true;																						// Buffer not full (not completely foolproof if original overflow warning ignored)
}

boolean HA_queue::put(byte devType, byte devNum, byte val) {
	SAVE_CONTEXT("Put1");
	
	if (!full()) {
 
		put(devType);
		put(devNum);
		put(val);
		
		if (full()) SENDLOG('W', "Queue full", devType);
	}
		
	RESTORE_CONTEXT
	
	return !full();
}

boolean HA_queue::put(byte devType, unsigned int val) {
	SAVE_CONTEXT("Put2");
	
	if (!full()) {
 
		put(devType);
		put(val >> 8);
		put(val & 0xff);
		
		if (full()) SENDLOG('W', "Queue full", devType);
	}
	
	RESTORE_CONTEXT
	
	return !full();
}

int HA_queue::get() {													// Repeated calls return next byte in buffer
	if (size() == 0) {
		flush();			
		return -1;									
	}
		
	int result = BYTESTRING::get(_getPtr % _bufLen);

	_getPtr++; 																				// Increment
	
	return result;
}
/*
void HA_queue::printQueue() {
	Serial.print("P: ");
	Serial.print(_putPtr);
	Serial.print(" G: ");
	Serial.println(_getPtr);
	
	for (int i = _getPtr; i < _putPtr; i=i+3) {
		byte devTypeB = BYTESTRING::get(i % _bufLen);
		byte devNum = BYTESTRING::get((i + 1) % _bufLen);
		byte devVal = BYTESTRING::get((i + 2) % _bufLen);
		char devType[5];
		
		getDevTypeChar(devTypeB, devType);
		Serial.print(devType);
		Serial.print(" ");
		
		switch (devTypeB) {
			case OBJ_TYPE_TIME: {
				char responseText[20];
				unsigned int dhm = (unsigned int)(devNum << 8) | devVal;
				dhmConvert(dhm, responseText);
				Serial.println(responseText);
				break;
			}
			case OBJ_TYPE_HEARTBEAT:
				Serial.println(devVal);
				break;
			default:
				Serial.print(devNum);
				Serial.print(" ");
				Serial.println(devVal);
		}
	}
}
*/
boolean HA_queue::full() {
	return (_putPtr - _getPtr) >= _bufLen;
}

void HA_queue::flush() {
  _putPtr = 0;
  _getPtr = 0;
}

unsigned int HA_queue::size() {
	if (_putPtr == _getPtr) flush();
  return _putPtr - _getPtr;
}

//***************** HA_stack - LIFO ************************

boolean HA_stack::init(byte *stack, unsigned int stackDepth) {
	_depth = 0;
  return BYTESTRING::init(stack, stackDepth);
}

boolean HA_stack::create(unsigned int stackDepth) {
	_depth = 0;
	return BYTESTRING::create(stackDepth);
}

boolean HA_stack::push(byte val) {
  if (BYTESTRING::put(_depth, val)) {
	  _depth++;
	  return true;
  }
  else return false;
}

boolean HA_stack::push(unsigned int val) {
	if (full()) return false;
	
	char buffer[30];
	
	push((byte)(val >> 8));
	push((byte)(val & 0xff));
	
	return true;
}

int HA_stack::pop() {													// Repeated calls return byte from top of stack
	if (_depth > 0) {
		_depth--;
		return BYTESTRING::get(_depth);
  }  
  else return -1;
}

unsigned int HA_stack::popInt() {
	return (unsigned int)(pop()) | ((unsigned int)pop() << 8);
}

unsigned int HA_stack::peekInt() {
	if (_depth > 0) return (unsigned int)(BYTESTRING::get(_depth - 1)) | (unsigned int)(BYTESTRING::get(_depth - 2) << 8);
}

unsigned int HA_stack::depth() {
  return _depth;
}

boolean HA_stack::full() {
	return _depth == _bufLen;
}

void HA_stack::peekAll(char *buffer, int maxLen) {
	byte offset = 0;
	char *temp;
	
	buffer[0] = '\0';

	for (int i = 0; i < _depth; i += 2) {
		temp = (char*)((unsigned int)(BYTESTRING::get(i+1)) | (unsigned int)(BYTESTRING::get(i) << 8));
		
		snprintf(buffer + offset, maxLen, "%s%s", temp, "->");
		
		offset += strlen(temp) + 2;
	}
	
	if (strlen(buffer) >= 2) strncpy((char*)(buffer + strlen(buffer) - 2), ":", 2);
}

// *************  BYTESTRING  *******************
// Provides methods for setting, clearing and testing a byte in an arbitrary length buffer
//

BYTESTRING::BYTESTRING() {
}

BYTESTRING::BYTESTRING(unsigned int bufLen) {
	create(bufLen);
}

BYTESTRING::~BYTESTRING() {
	freeMem();
}

boolean BYTESTRING::create(unsigned int bufLen) {
	_bufLen = bufLen;
	_buffer = (byte*)malloc(_bufLen);		// Get the space
	
	if (_buffer == NULL) return false;								// If no space
	else return true;
}

boolean BYTESTRING::init(byte *buffer, unsigned int bufLen) {			// Pass pointer to array and the length of that array (in bits)
  _buffer = buffer;
  _bufLen = bufLen;

  return true;
}

void BYTESTRING::freeMem() {							// Called either by user (if user called create()), or from deconstructor
	free(_buffer);
}

boolean BYTESTRING::put(unsigned int byteNum, byte byteVal) {
  if (byteNum < _bufLen) {														// Check not outside bounds    
  	_buffer[byteNum] = byteVal;	  
	  return true;
  }
  else return false;
}

int BYTESTRING::get(unsigned int byteNum) {
	if (byteNum < _bufLen) return _buffer[byteNum];
	else return -1;
}


void BYTESTRING::clearAll() {

	memset(_buffer, 0x00, _bufLen);
}

void BYTESTRING::setAll() {
	memset(_buffer, 0xff, _bufLen);
}

HA_queue changeList;