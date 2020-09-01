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

 
#ifndef Bitstring_h
#define Bitstring_h

#include "Arduino.h"

class BITSTRING {
public:
	BITSTRING();													// Constructor 
  BITSTRING(unsigned int bufLenBits);
	~BITSTRING();													// If malloc used then frees memory
	
  boolean create(unsigned int bufLen);
	boolean init(byte *buffer, unsigned int bufLen);
	void freeMem();
	  
  boolean put(unsigned int bitNum, byte bitVal);
  int get(unsigned int bitNum);
  byte getByte(unsigned int byteNum);
  void clearAll();
  void setAll();

protected:
  byte *_buffer;												// Pointer to buffer
  unsigned int _bufLenBits;							// Number of bits in buffer
  unsigned int _bufLenBytes;						// Number of bytes used to store bits, rounded up
};


class FIFO : public BITSTRING {
public:
  boolean create(unsigned int bufLen);  
	boolean init(byte *buffer, unsigned int bufLen);
  boolean put(byte bitVal);
  int get();
  void rewind();
  void flush();
  unsigned int size();
private:
  unsigned int _putPtr;
  unsigned int _getPtr;	
};


class LIFO : public BITSTRING {
public:
  boolean create(unsigned int stackDepth);
  boolean init(byte *stack, unsigned int stackDepth);
  boolean push(byte bitVal);
  int pop();
  unsigned int depth();
private:
  unsigned int _depth;
};



#endif
