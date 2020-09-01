 /*
    Copyright (C) 2011  Andrew Richards
    Bitstring library providing raw, FIFO and LIFO (AKA stack) manipulation of arbitrary length bitstrings

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

#include "Bitstring.h"


//***************** FIFO ************************

boolean FIFO::init(byte *buffer, unsigned int bufLenBits) {
  _putPtr = 0;
  _getPtr = 0;
  return BITSTRING::init(buffer, bufLenBits);
}

boolean FIFO::create(unsigned int bufLenBits) {
	_putPtr = 0;
  _getPtr = 0;
	return BITSTRING::create(bufLenBits);
}

boolean FIFO::put(byte val) {
  if (BITSTRING::put(_putPtr, val)) {
	  _putPtr++;
	  return true;
  }
  else return false;
}

int FIFO::get() {													// Repeated calls return next bit in buffer
	int result = BITSTRING::get(_getPtr);

  if (result == -1 || ++_getPtr > _putPtr) {										// Increment counter, unless at end of valid bits or outside range
	  _getPtr--;
	  result = -1;
  }  
  
  return result;
}

void FIFO::rewind() {
	_getPtr = 0;
}

void FIFO::flush() {
  _putPtr = 0;
  _getPtr = 0;
}

unsigned int FIFO::size() {
  return _putPtr;
}

//***************** LIFO ************************

boolean LIFO::init(byte *stack, unsigned int stackDepth) {
	_depth = 0;
  return BITSTRING::init(stack, stackDepth);
}

boolean LIFO::create(unsigned int stackDepth) {
	_depth = 0;
	return BITSTRING::create(stackDepth);
}

boolean LIFO::push(byte val) {
  if (BITSTRING::put(_depth, val)) {
	  _depth++;
	  return true;
  }
  else return false;
}

int LIFO::pop() {													// Repeated calls return bit from top of stack
	if (_depth > 0) {
		_depth--;
		return BITSTRING::get(_depth);
  }  
  else return -1;
}

unsigned int LIFO::depth() {
  return _depth;
}

// *************  BITSTRING  *******************
// Provides methods for setting, clearing and testing a bit in an arbitrary length buffer
//

BITSTRING::BITSTRING() {
}

BITSTRING::BITSTRING(unsigned int bufLenBits) {
	create(bufLenBits);
}

BITSTRING::~BITSTRING() {
	freeMem();
}

boolean BITSTRING::create(unsigned int bufLenBits) {
	_bufLenBits = bufLenBits;
  _bufLenBytes = ((bufLenBits - 1) / 8) + 1;
	_buffer = (byte*)malloc(_bufLenBytes);		// Get the space
	
	if (_buffer == NULL) return false;								// If no space
	else return true;
}

boolean BITSTRING::init(byte *buffer, unsigned int bufLenBits) {			// Pass pointer to array and the length of that array (in bits)
  _buffer = buffer;
  _bufLenBits = bufLenBits;
  _bufLenBytes = ((bufLenBits - 1) / 8) + 1;

  return true;
}

void BITSTRING::freeMem() {							// Called either by user (if user called create()), or from deconstructor
	free(_buffer);
}


boolean BITSTRING::put(unsigned int bitNum, byte bitVal) {
  if (bitNum < _bufLenBits) {														// Check not outside bounds    
  	switch (bitVal) {
	  	case 0:		_buffer[bitNum / 8] &= ~_BV(bitNum % 8); break;		// Clear bit
	  	case 1:		_buffer[bitNum / 8] |= _BV(bitNum % 8); break;		// Store LSB bit
    }	  
	  return true;
  }
  else return false;
}

int BITSTRING::get(unsigned int bitNum) {
	if (bitNum < _bufLenBits) {
		int result = _buffer[bitNum / 8] & _BV(bitNum % 8);		// Get bit from relevant byte
		result >>= (bitNum % 8);																		// Shift bit to LSB - 0 or 1
		return result;
	}
	else return -1;
}

byte BITSTRING::getByte(unsigned int byteNum) {
	if (byteNum < _bufLenBytes) return _buffer[byteNum];
}

void BITSTRING::clearAll() {
	memset(_buffer, 0x00, _bufLenBytes);
}

void BITSTRING::setAll() {
	memset(_buffer, 0xff, _bufLenBytes);
}

