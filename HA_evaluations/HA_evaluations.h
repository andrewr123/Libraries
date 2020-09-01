 /*
    Copyright (C) 2011  Andrew Richards
 
    Part of home automation suite
    
    Contains the HA_evaluation and HA_argument classes to manage decisions

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

 
#ifndef HA_evaluations_h
#define HA_evaluations_h

#include "HA_globals.h"

//***************** Global constants

// *************** Constants for referencing HA_evaluation *****************
// Values specific to allow evalMap to overlay eval Struct

const static byte VAL_A = 0x00;
const static byte VAL_B = 0x01;
const static byte VAL_CALC = 0x02;
const static byte VAL_EXP = 0x12;
const static byte VAL_A_TYPE = 0x03;
const static byte VAL_B_TYPE = 0x13;

const static byte NUM_CALCS = 16;
const static char CALC_TYPES[NUM_CALCS] = { 'c', '!', 'p', 'r', 'o', 'f', 'y', 'm', 'n', '@', '&', '|', 'V', 'A', 'O', 'e' };  // See CALC_ constants below
const static byte NUM_EXPS = 15;
const static char EXP_TYPES[NUM_EXPS] = { 's', 'u', '+', '-', '*', '/', '&', '|', '=', '!', '>', '<', '[', ']', 'z' }; // See EXP_ constants below

// VAL_CALC values - how to calculate valA - MSnibble of valCalcExp in HA_evaluation
// valA is either:
//		a) the index of a device (DEV_TYPE_n) or a variable (VAR_TYPE_n), as determined by VAL_A_TYPE
//		b) unused, replaced with a time, calculated dynamically
//		c) the index of an argument list with arguments indexing entities
//		d) the index of an arugment list with arguments indexing evaluations
//		e) the index of another evaluation
// valB is either the current value of a device or a variable, as determined by VAL_B_TYPE

const static byte CALC_DEV = 0x00;			// Current value of device/variable indexed by valA
const static byte CALC_NDEV = 0x01;			// Inverse of current value of device/variable indexed by valA
const static byte CALC_PDEV = 0x02;			// Previous value of device (not variable) indexed by valA
const static byte CALC_ROFC = 0x03;			// % x 100 - curr change from prev as percentage x 100 of device indexed by valA.  Interpret as (signed) int
const static byte CALC_ON = 0x04;				// True/false - curr vs prev of device indexed by valA
const static byte CALC_OFF = 0x05;			// True/false - curr vs prev of device indexed by valA

const static byte CALC_YEAR = 0x06;			// Current year
const static byte CALC_MONTH = 0x07;		// Current month
const static byte CALC_NOW = 0x08;			// Current time (in day, hour, minute format - see dhm routines)

const static byte CALC_AVG = 0x09;			// Average of argument list indexed by valA; arg list elements index entities
const static byte CALC_AND = 0x0A;			// Logical AND of argument list indexed by valA
const static byte CALC_OR = 0x0B;				// Logical OR of argument list indexed by valA

const static byte CALC_AVGV = 0x0C;			// Average of argument list indexed by valA; arg list elements index evaluations
const static byte CALC_ANDV = 0x0D;			// Logical AND of argument list indexed by valA
const static byte CALC_ORV = 0x0E;			// Logical OR of argument list indexed by valA

const static byte CALC_EVAL = 0x0F;			// Result of evaluation indexed by valA

// VAL_EXP values - what to do with valA and valB - LSnibble of valCalcExp in HA_evaluation
//						a) Calc A then perform expression on destination (valB).  Return result and store in valB
//						b) Evaluate A and B (no destination)
//						c) Do nothing, just return valA

const static byte EXP_SET  = 0x00;			// If valA == variable, non-zero single device or calculated, then set valB = valA or TRUE (if binary valB)
const static byte EXP_UNSET = 0x01;			// If valA then set valB = FALSE

const static byte EXP_ADD = 0x02;				// valA + valB
const static byte EXP_SUB = 0x03;				// valA - valB 
const static byte EXP_MULT = 0x04;			// valA * valB
const static byte EXP_DIV = 0x05;				// valA / valB
const static byte EXP_AND = 0x06;				// valA && valB
const static byte EXP_OR = 0x07;				// valA || valB
const static byte EXP_EQ = 0x08;				// valA == valB
const static byte EXP_NEQ = 0x09;				// valA != valB
const static byte EXP_GT = 0x0A;				// valA > valB
const static byte EXP_LT = 0x0B;				// valA < valB
const static byte EXP_BTW = 0x0C;				// True if now falls between valA and valB inclusive (dhm format)
const static byte EXP_NOT_BTW = 0x0D;  	// True if now is outside valA to valB (dhm format)

const static byte EXP_NOOP = 0x0E;			// Don't do evaluation or set dest, just return valA

// IF conditions:
//
// IF . . THEN statements held in argLists as follows:
// Byte n & n+1: {<IF_test> << 4 || <devTypeA>} {<devNumA>}
// Byte n+2 & n+3: {<devTypeB>} {<devNumB>}		(certain IF_tests only)

// 	a) Test against device - no arglist
const static byte IF_ON 				= 0x00;		// Immediate test against device
const static byte IF_OFF 				= 0x01;

//	b) Test against arglist
const static byte IF_TRUE				= 0x02;		// Test against arglist
const static byte IF_FALSE			= 0x03;

const static byte IF_AND				= 0x04;
const static byte IF_OR					= 0x05;
const static byte IF_NAND				= 0x06;
const static byte IF_NOR				= 0x07;

const static byte THEN_SET			= 0x00;		// Immmediate set (== on) of device
const static byte THEN_UNSET		= 0x01;		
const static byte THEN_ACTIONS	= 0x02;



// ************************ Global methods

byte getCalcIdx(char calcTypeChar);
char getCalcChar(byte calcTypeIdx); 
byte getExpIdx(char expTypeChar);
char getExpChar(byte expTypeIdx);


// ****************** HA_evaluation ******************
// Instantiated as an array in dynamic memory pointed to by root.ptrEvaluation
// Contains instructions on how to perform evaluation

class HA_evaluation {			
	public:
		// Methods
		void put(byte valType, byte val);
		void put(byte valCalc, byte valAType, byte valA, byte valExp, byte valBType, byte valB);
		byte get(byte valType);
		void get(byte *valCalc, byte *valAType, byte *valA, byte *valExp, byte *valBType, byte *valB);
		
	protected:
		// Methods
		
	  // Properties
	  union {
	  	byte evalMap[4];  	
	  	struct {
				byte valA;					// Primary argument - CALC_ determines how to work out
				byte valB;					// Secondary argument or destination - device/variable type determined by LSnibble valTypes 
				byte valCalcExp;		// MSnibble determines how to calculate valA (CALC_); LSnibble is the expression to evaluate (EXP_
				byte valTypes;			// MSnibble determines valA device type; LSnibble determines valB device type.  Values per DEV_TYPE_
			} eval;
  	} _map;
		
		// Constants
		const static byte MASK_EVAL_MAP_IDX = 0x07;            // This &'ed with VAL_xxx gives index into evalMap
			
};

// ****************** HA_arglist ********************
// Instantiated as an array in dynamic memory pointed to by root.ptrArgList
// Contains a list of arguments comprising either indexes into the HA_entities array or indexes into the HA_evaluation array
// Arg list is normally allocated dynamically (unless 3 or fewer args when held locally)

class HA_argList {
	public:
		HA_argList(byte argLen);
		~HA_argList();
		
		boolean create(byte numArgs);
		void init(byte *addr, byte numArgs);
		byte numArgs();
		void put(byte argNum, byte argVal);
		byte get(byte argNum);
		
	protected:
		byte _numArgs;				// Max 256
		union {
			struct {
				byte dummy;				// To pad out alignment to a word boundary
				byte *argList;		// Ptr to memory _numArgs bytes in length.  Structure determined by calling routine
			} argPtr;
			byte _argElem[3];		// If 3 or less arguments, then hold locally
		};				
};

#endif