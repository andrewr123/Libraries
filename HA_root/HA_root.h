 /*
    Copyright (C) 2011  Andrew Richards
 
    Part of home automation suite
    
    Contains the HA_root class

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

 
#ifndef HA_root_h
#define HA_root_h

#include "HA_globals.h"
#include "HA_device_bases.h"
#include "HA_zone.h"
#include "HA_devices.h"
#include "HA_devMotion.h"
#include "HA_devHeat.h"
#include "HA_variables.h"
#include "HA_evaluations.h"
#include "HA_channels.h"


		
		
// *********** Global root for all data structures ************
// Comprises pointers to malloc arrays of classes

class HA_root {

	
	public:
	
		friend void ISR1();
		
		// Methods
		HA_root();
		~HA_root();

		// Methods for entities - devices, variables & zones
		boolean createEntArray(byte entType, byte numEntities);
		byte numEnts(byte entType);
		byte classSize(byte entType);
		byte entBufLen(byte entType);	
	  
		void putEnt(byte entType, byte entNum, byte valType, unsigned int val);
		unsigned int getEnt(byte entType, byte entNum, byte valType);
		
		byte putRef(byte entType, byte entNum, char *device);
		void getRef(byte entType, byte entNum, char *device);
		void getSnapshot(byte entType, byte entNum);
		boolean logChange(byte entType, byte entNum, byte val);
		
		void putEnt(byte entType, byte entNum, byte valType, void *valPtr);
		void getEnt(byte entType, byte entNum, byte valType, void *valPtr);
		byte *getBufPtr(byte entType, byte entNum, byte valtype);
		
		void putEntElem(byte entType, byte entNum, byte valType, byte elemIdx, byte elemVal);
		byte getEntElem(byte entType, byte entNum, byte valType, byte elemIdx);
		
		void initDev(byte devType, byte devNum, byte channel = 0, byte pin = 0, byte handler = 0);
		void resetDev(byte devType, byte devNum);
		void readDev(byte devType, byte devNum);
		void setDev(byte devType, byte devNum, byte val);
		
		void processDevInt(byte devType, byte devNum, byte alertingPin = 0);
		void devISR(byte devType, byte devNum, byte alertingPin = 0);
		
		int findZone(byte regionZone);
		
		// Methods for channels 
		boolean createChanArray(byte numChans);
		byte numChans();
		
		boolean initChan(byte chanNum, byte protocolType, byte maxPins, byte ioPin);	
		boolean initChanAccess(byte chanNum, byte accessType, byte latchPin, byte dataPin, byte clockPin);
		boolean initChanAlert(byte chanNum, byte alertType, byte intNum, byte intMode, byte resetPin = 0, byte slaveSelectPin = 0, byte slaveSelectPinBank0 = 0, byte slaveSelectPinBank1 = 0);
		byte getChan(byte chanNum, byte type);
		HA_channel *getChanObj(byte chanNum);
		void chanISR(byte chanNum);
		boolean registerDevRange(byte chanNum, byte rangeNum, byte intPin, byte devType, byte devNum);
			  		
		// Methods for evaluations
		boolean createEvalArray(byte numEvals);
		byte numEvals();
		
		void putEval(byte evalNum, byte valType, byte val);
		byte getEval(byte evalNum, byte valType);
		
		void putEval(byte evalNum, byte valCalc, byte valAType, byte valA, byte valExp, byte valBType, byte valB);
		void getEval(byte evalNum, byte *valCalc, byte *valAType, byte *valA, byte *valExp, byte *valBType, byte *valB);
		
		// Methods for arg lists
		boolean createArgListArray(byte numArgLists);
		byte numArgLists();
		
		boolean createArgList(byte argListNum, byte numArgs);
		byte numArgs(byte argListNum);
		
		void putArg(byte argListNum, byte argNum, byte val);
		byte getArg(byte argListNum, byte argNum);
				
		// Perform evaluations
		void runEval(byte evalNum, unsigned int *evalPtr);
		unsigned int getValA(byte valCalc, byte valAType, byte valA);
		void runExp(byte valCalc, byte valAType, unsigned int evalA, byte valExp, byte valBType, byte valB, unsigned int *evalPtr);
		boolean equal(byte valAType, byte *evalAPtr, byte *evalBPtr);
		
		// Debug
		void printRoot();
		
	protected:
		// Properties
		
		//HA_zone							*zonePtr;
		
		// Entities - devices & variables
		byte _numEnts[NUM_DEV_TYPES + NUM_VAR_TYPES + NUM_ZONE_TYPES];
		byte _classSize[NUM_DEV_TYPES + NUM_VAR_TYPES + NUM_ZONE_TYPES];
		union {
			void *_entPtrs[NUM_DEV_TYPES + NUM_VAR_TYPES + NUM_ZONE_TYPES];
			struct {		
				HA_devTouch 		*ptrTouch;
				HA_devFire 			*ptrFire;
				HA_devHeat 			*ptrHeat;
				HA_devLuminance *ptrLuminance;
				HA_devMotion 		*ptrMotion;
				HA_devPresence 	*ptrPresence;
				HA_devRFID 			*ptrDevRFID;
				HA_devOpen 			*ptrOpen;
				HA_dev5APower 	*ptr5APower;
				HA_dev13APower 	*ptr13APower;
				HA_devLock 			*ptrLock;
				HA_devLight 		*ptrLight;
				HA_devRelay 		*ptrRelay;
				HA_varByte			*ptrByte;
				HA_var2Byte			*ptr2Byte;
				HA_varRFID			*ptrVarRFID;
				HA_zone					*ptrZone;
			} entPtrs;
		};

		// Evaluations & arg lists
		byte _numEvals;
		HA_evaluation				*ptrEvaluation;
		
		byte _numArgLists;
		HA_argList					*ptrArgList;
		
		// Channels
		byte _numChannels;
		HA_channel					*ptrChannel; 
		
};

extern HA_root root;



#endif
