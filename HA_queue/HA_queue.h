 /*
    Copyright (C) 2013  Andrew Richards
    
    Part of home automation suite
    
    Contains methods to manage byte-oriented ring queues (fifo) and stacks

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

 
#ifndef HA_queue_h
#define HA_queue_h

#include "Arduino.h" 
//#include "HA_syslog.h" 
//#include "HA_device_bases.h"     

					

class BYTESTRING {
public:
	BYTESTRING();													// Constructor 
  BYTESTRING(unsigned int bufLen);
	~BYTESTRING();													// If malloc used then frees memory
	
  boolean create(unsigned int bufLen);
	boolean init(byte *buffer, unsigned int bufLen);
	void freeMem();
	  
  boolean put(unsigned int byteNum, byte byteVal);
  int get(unsigned int byteNum);
  void clearAll();
  void setAll();

protected:
  byte *_buffer;												// Pointer to buffer
  unsigned int _bufLen;						// Number of bytes in buffer
};


class HA_queue : public BYTESTRING {
public:
  boolean create(unsigned int bufLen);  
	boolean init(byte *buffer, unsigned int bufLen);
  boolean put(byte byteVal);
  boolean put(byte devType, byte devNum, byte val);
  boolean put(byte devType, unsigned int val);
  int get();
  void get(byte *devType, unsigned int *val);
  boolean full();
  void rewind();
  void flush();
  unsigned int size();
 // void printQueue();
  
private:
  unsigned int _putPtr;			
  unsigned int _getPtr;	
};


class HA_stack : public BYTESTRING {
public:
  boolean create(unsigned int stackDepth);
  boolean init(byte *stack, unsigned int stackDepth);
  boolean push(byte byteVal);
  boolean push(unsigned int val);
  int pop();
  unsigned int popInt();
  unsigned int peekInt();
  void peekAll(char *buffer, int maxLen);
  unsigned int depth();
  boolean full();
private:
  unsigned int _depth;
};

extern HA_queue changeList;

#endif

