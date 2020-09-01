 /*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
    
    Contains the HA_device library to manage the directory of devices (sensors or actors)

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

#include "HA_evaluations.h"
//#include "Time.h"


// **************** Evaluation helpers ********************
        
byte getCalcIdx(char calcTypeChar) {
	return (char*)memchr(CALC_TYPES, calcTypeChar, NUM_CALCS) - CALC_TYPES;
}

char getCalcChar(byte calcTypeIdx) {
	if (calcTypeIdx <= NUM_CALCS) return CALC_TYPES[calcTypeIdx];
}

byte getExpIdx(char expTypeChar) {
	return (char*)memchr(EXP_TYPES, expTypeChar, NUM_EXPS) - EXP_TYPES;
}

char getExpChar(byte expTypeIdx) {
	if (expTypeIdx <= NUM_EXPS) return EXP_TYPES[expTypeIdx];
}



//***********  HA_evaluation  *****************

void HA_evaluation::put(byte valType, byte value) { 
	byte evalMapIdx = valType & MASK_EVAL_MAP_IDX;
  byte mask, offset;
  
  switch (valType) {
	  case VAL_A:					mask = MASK_BYTE; offset = 0; break;
	  case VAL_B:					mask = MASK_BYTE; offset = 0; break;
	  case VAL_CALC:				
	  case VAL_A_TYPE:		mask = MASK_MS_NIBBLE; offset = OFFSET_MS_NIBBLE; break;
	  case VAL_EXP:				
	  case VAL_B_TYPE:		mask = MASK_LS_NIBBLE; offset = 0; break;
	}
  
	_map.evalMap[evalMapIdx] &= ~mask;                          // Clear
  _map.evalMap[evalMapIdx] |= ((value << offset) & mask);     // Store 
}

void HA_evaluation::put(byte valCalc, byte valAType, byte valA, byte valExp, byte valBType, byte valB) {
	_map.eval.valA = valA; 
	_map.eval.valB = valB; 
	_map.eval.valCalcExp = (valCalc << OFFSET_MS_NIBBLE) | (MASK_LS_NIBBLE & valExp);
	_map.eval.valTypes = (valAType << OFFSET_MS_NIBBLE) | (MASK_LS_NIBBLE & valBType);
}
        
byte HA_evaluation::get(byte valType) { 
	byte evalMapIdx = valType & MASK_EVAL_MAP_IDX;
  byte mask, offset;
  
  switch (valType) {
	  case VAL_A:					mask = MASK_BYTE; offset = 0; break;
	  case VAL_B:					mask = MASK_BYTE; offset = 0; break;
	  case VAL_CALC:				
	  case VAL_A_TYPE:		mask = MASK_MS_NIBBLE; offset = OFFSET_MS_NIBBLE; break;
	  case VAL_EXP:				
	  case VAL_B_TYPE:		mask = MASK_LS_NIBBLE; offset = 0; break;
	}
	
	return (_map.evalMap[evalMapIdx] & mask) >> offset;
}

void HA_evaluation::get(byte *valCalc, byte *valAType, byte *valA, byte *valExp, byte *valBType, byte *valB) {
	*valA = _map.eval.valA;
	*valB = _map.eval.valB;
	*valCalc = (_map.eval.valCalcExp & MASK_MS_NIBBLE) >> OFFSET_MS_NIBBLE;
	*valExp =  _map.eval.valCalcExp & MASK_LS_NIBBLE;
	*valAType = (_map.eval.valTypes & MASK_MS_NIBBLE) >> OFFSET_MS_NIBBLE;
	*valBType =  _map.eval.valTypes & MASK_LS_NIBBLE;
	}




// **************** HAargPtr.argList ****************

HA_argList::HA_argList (byte numArgs) {
	create(numArgs);
}

HA_argList::~HA_argList () {
	free(argPtr.argList);
}

boolean HA_argList::create(byte numArgs) {			// To be called directly if constructor not called (if malloc'ed)
	byte *addr;	

	if (numArgs > 3) addr = (byte*)malloc(numArgs);			// Get some space
	else addr = _argElem;																// Unless small enough to be held locally
	
	if (addr != NULL) {
		init(addr, numArgs);		// If got space then initialise it
		return true;
	}
	else {
		_numArgs = 0; 
		return false;
	}
}

void HA_argList::init(byte *addr, byte numArgs) {		
	_numArgs = numArgs;
	argPtr.argList = addr;			
	memset(addr, 0, _numArgs); 
}

byte HA_argList::numArgs() {
	return _numArgs;
}

void HA_argList::put(byte argNum, byte argVal) {
	if (argNum < _numArgs) 
		if (_numArgs > 3) argPtr.argList[argNum] = argVal;
		else _argElem[argNum] = argVal;
}

byte HA_argList::get(byte argNum) { 
	if (argNum < _numArgs) return (_numArgs > 3) ? argPtr.argList[argNum] : _argElem[argNum];
}
