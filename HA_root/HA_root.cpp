 /*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
    
    Contains HA_root methods

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


#include "HA_root.h"
#include "Time.h"

          
HA_root::HA_root() {
	for (int i = 0; i < (NUM_DEV_TYPES + NUM_VAR_TYPES + 1); i++) {					// +1 for zones
		_numEnts[i] = 0;
		_classSize[i] = 0;
		_entPtrs[i] = NULL;
	}
	
	_numEvals = 0;
	ptrEvaluation = NULL;
	
	_numArgLists = 0;
	ptrArgList = NULL;
	
	_numChannels = 0;
	ptrChannel = NULL;
};

HA_root::~HA_root() {};


int HA_root::findZone(byte regionZone) {
	for (int zoneNum = 0; zoneNum < _numEnts[OBJ_TYPE_ZONE]; zoneNum++) {
		if (entPtrs.ptrZone[zoneNum].get(VAL_REGION_ZONE) == regionZone) return zoneNum;
	}
	
	return -1;	
}


// *************  Entities - device & variables  ********************

boolean HA_root::createEntArray(byte entType, byte numEntities) {		// Finds space for array of entities
	int classSize = 0;

	// Avoid duplicates
	if (_numEnts[entType] != 0) {Serial.println("HA_root: dup Ent array"); return false;}
		
	switch (entType) {
		case DEV_TYPE_TOUCH:			classSize = sizeof(HA_devTouch); break;
		case DEV_TYPE_FIRE:				classSize = sizeof(HA_devFire); break;
		case DEV_TYPE_HEAT:			  classSize = sizeof(HA_devHeat); break;
		case DEV_TYPE_LUMINANCE:  classSize = sizeof(HA_devLuminance); break;
		case DEV_TYPE_MOTION:			classSize = sizeof(HA_devMotion); break;
		case DEV_TYPE_PRESENCE:		classSize = sizeof(HA_devPresence); break;
		case DEV_TYPE_RFID:				classSize = sizeof(HA_devRFID); break;
		case DEV_TYPE_OPEN:				classSize = sizeof(HA_devOpen); break;
		case DEV_TYPE_5APWR:			classSize = sizeof(HA_dev5APower); break;
		case DEV_TYPE_13APWR:			classSize = sizeof(HA_dev13APower); break;
		case DEV_TYPE_LOCK:				classSize = sizeof(HA_devLock); break;
		case DEV_TYPE_LIGHT:			classSize = sizeof(HA_devLight); break;
		case DEV_TYPE_RELAY:			classSize = sizeof(HA_devRelay); break;
		case VAR_TYPE_BYTE:				classSize = sizeof(HA_varByte); break;
		case VAR_TYPE_2BYTE:			classSize = sizeof(HA_var2Byte); break;
		case VAR_TYPE_RFID:				classSize = sizeof(HA_varRFID); break;
		case OBJ_TYPE_ZONE:				classSize = sizeof(HA_zone); break;
	}
	
	_entPtrs[entType] = malloc(classSize * numEntities);					// Get some space
	
	if (_entPtrs[entType] != NULL) {														// Attach arrray of devices to root pointer
		_numEnts[entType] = numEntities;
		_classSize[entType] = classSize;
		
		if (entType == DEV_TYPE_RFID || entType == VAR_TYPE_RFID) {		// If multi-byte entity, get space for buffer
			unsigned int bufLen = entBufLen(entType) * numEntities * ((entType == DEV_TYPE_RFID) ? 2 : 1);  
			byte *space = (byte*)malloc(bufLen);		
		  if (space != NULL) {
			  switch (entType) {																
					case DEV_TYPE_RFID:		for (int i = 0; i < numEntities; i++) entPtrs.ptrDevRFID[i].init(space + (i * bufLen * 2)); break;
					case VAR_TYPE_RFID:		for (int i = 0; i < numEntities; i++) entPtrs.ptrVarRFID[i].init(space + (i * bufLen)); break;
				}
  		}
  		else {		// No space for values - free array space
	  		free(_entPtrs[entType]);
	  		_numEnts[entType] = 0;
	  		return false;
  		}
		}
		for (int i = 0; i < numEntities; i++) putEnt(entType, i, VAL_DEVIDX, i);				// Initialise device number
		return true;
	}
	else {
		_numEnts[entType] = 0; 
		return false;
	}
}

byte HA_root::numEnts(byte entType) {
	return _numEnts[entType];
}

byte HA_root::classSize(byte entType) {
	return _classSize[entType];
}

byte HA_root::entBufLen(byte entType) {
	switch (entType) {
		case DEV_TYPE_RFID:				return entPtrs.ptrDevRFID[0].bufLen(); 
		case VAR_TYPE_RFID:				return entPtrs.ptrVarRFID[0].bufLen();
		default:									return 0;
	}
}

void HA_root::putEnt(byte entType, byte entNum, byte valType, unsigned int val) {		// Not suitable for multi-byte buffers
	// Check bounds
	if (_numEnts[entType] < entNum) Serial.println("HA_root: put OO bounds");
	
	switch (entType) {
		case DEV_TYPE_TOUCH:			entPtrs.ptrTouch[entNum].put(valType, val); break;
		case DEV_TYPE_FIRE:				entPtrs.ptrFire[entNum].put(valType, val); break;
		case DEV_TYPE_HEAT:			  entPtrs.ptrHeat[entNum].put(valType, val); break;
		case DEV_TYPE_LUMINANCE:  entPtrs.ptrLuminance[entNum].put(valType, val); break;
		case DEV_TYPE_MOTION:			entPtrs.ptrMotion[entNum].put(valType, val); break;
		case DEV_TYPE_PRESENCE:		entPtrs.ptrPresence[entNum].put(valType, val); break;
		case DEV_TYPE_RFID:				entPtrs.ptrDevRFID[entNum].put(valType, val); break;		// Only underlying HA_device data available
		case DEV_TYPE_OPEN:				entPtrs.ptrOpen[entNum].put(valType, val); break;
		case DEV_TYPE_5APWR:			entPtrs.ptr5APower[entNum].put(valType, val); break;
		case DEV_TYPE_13APWR:			entPtrs.ptr13APower[entNum].put(valType, val); break;
		case DEV_TYPE_LOCK:				entPtrs.ptrLock[entNum].put(valType, val); break;
		case DEV_TYPE_LIGHT:			entPtrs.ptrLight[entNum].put(valType, val); break;
		case DEV_TYPE_RELAY:			entPtrs.ptrRelay[entNum].put(valType, val); break;
		case VAR_TYPE_BYTE:			  entPtrs.ptrByte[entNum].put(val); break;
		case VAR_TYPE_2BYTE:			entPtrs.ptr2Byte[entNum].put(val); break;		
		case VAR_TYPE_RFID:				break;		// No underlying data for multi-byte variable
		case OBJ_TYPE_ZONE:				entPtrs.ptrZone[entNum].put(valType, val); break;		
	}
}

unsigned int HA_root::getEnt(byte entType, byte entNum, byte valType) {								// Not suitable for multi-byte buffers
	// Check bounds
	if (_numEnts[entType] < entNum) {
		Serial.println("HA_root: get OO bounds1");
		Serial.println(entType);
		Serial.println(_numEnts[entType]);
		Serial.println(entNum);
	}
	
	switch (entType) {
		case DEV_TYPE_TOUCH:			return entPtrs.ptrTouch[entNum].get(valType); 
		case DEV_TYPE_FIRE:				return entPtrs.ptrFire[entNum].get(valType); 
		case DEV_TYPE_HEAT:			  return entPtrs.ptrHeat[entNum].get(valType); 
		case DEV_TYPE_LUMINANCE:  return entPtrs.ptrLuminance[entNum].get(valType); 
		case DEV_TYPE_MOTION:			return entPtrs.ptrMotion[entNum].get(valType); 
		case DEV_TYPE_PRESENCE:		return entPtrs.ptrPresence[entNum].get(valType); 
		case DEV_TYPE_RFID:				return entPtrs.ptrDevRFID[entNum].get(valType); 			// Only underlying HA_device data available
		case DEV_TYPE_OPEN:				return entPtrs.ptrOpen[entNum].get(valType); 
		case DEV_TYPE_5APWR:			return entPtrs.ptr5APower[entNum].get(valType); 
		case DEV_TYPE_13APWR:			return entPtrs.ptr13APower[entNum].get(valType); 
		case DEV_TYPE_LOCK:				return entPtrs.ptrLock[entNum].get(valType); 
		case DEV_TYPE_LIGHT:			return entPtrs.ptrLight[entNum].get(valType); 
		case DEV_TYPE_RELAY:			return entPtrs.ptrRelay[entNum].get(valType); 
		case VAR_TYPE_BYTE:				return entPtrs.ptrByte[entNum].get(); 
		case VAR_TYPE_2BYTE:			return entPtrs.ptr2Byte[entNum].get(); 
		case VAR_TYPE_RFID:				break;		// No underlying data for multi-byte variable
		case OBJ_TYPE_ZONE:				return entPtrs.ptrZone[entNum].get(valType);
		
	}
}

byte HA_root::putRef(byte entType, byte entNum, char *device) {
	// Check bounds
	if (_numEnts[entType] < entNum) Serial.println("Putref: put OO bounds");
	
	switch (entType) {
		case DEV_TYPE_TOUCH:			return entPtrs.ptrTouch[entNum].putRef(device); 
		case DEV_TYPE_FIRE:				return entPtrs.ptrFire[entNum].putRef(device); 
		case DEV_TYPE_HEAT:			  return entPtrs.ptrHeat[entNum].putRef(device); 
		case DEV_TYPE_LUMINANCE:  return entPtrs.ptrLuminance[entNum].putRef(device); 
		case DEV_TYPE_MOTION:			return entPtrs.ptrMotion[entNum].putRef(device); 
		case DEV_TYPE_PRESENCE:		return entPtrs.ptrPresence[entNum].putRef(device); 
		case DEV_TYPE_RFID:				return entPtrs.ptrDevRFID[entNum].putRef(device); 	
		case DEV_TYPE_OPEN:				return entPtrs.ptrOpen[entNum].putRef(device); 
		case DEV_TYPE_5APWR:			return entPtrs.ptr5APower[entNum].putRef(device); 
		case DEV_TYPE_13APWR:			return entPtrs.ptr13APower[entNum].putRef(device); 
		case DEV_TYPE_LOCK:				return entPtrs.ptrLock[entNum].putRef(device); 
		case DEV_TYPE_LIGHT:			return entPtrs.ptrLight[entNum].putRef(device); 
		case DEV_TYPE_RELAY:			return entPtrs.ptrRelay[entNum].putRef(device);  		
	}
}
 
 
void HA_root::getRef(byte entType, byte entNum, char *device) {
	// Check bounds
	if (_numEnts[entType] < entNum) Serial.println("Getref: put OO bounds");
	
	switch (entType) {
		case DEV_TYPE_TOUCH:			entPtrs.ptrTouch[entNum].getRef(device); break;
		case DEV_TYPE_FIRE:				entPtrs.ptrFire[entNum].getRef(device); break;
		case DEV_TYPE_HEAT:			  entPtrs.ptrHeat[entNum].getRef(device); break;
		case DEV_TYPE_LUMINANCE:  entPtrs.ptrLuminance[entNum].getRef(device); break;
		case DEV_TYPE_MOTION:			entPtrs.ptrMotion[entNum].getRef(device); break;
		case DEV_TYPE_PRESENCE:		entPtrs.ptrPresence[entNum].getRef(device); break;
		case DEV_TYPE_RFID:				entPtrs.ptrDevRFID[entNum].getRef(device); break;	
		case DEV_TYPE_OPEN:				entPtrs.ptrOpen[entNum].getRef(device); break;
		case DEV_TYPE_5APWR:			entPtrs.ptr5APower[entNum].getRef(device); break;
		case DEV_TYPE_13APWR:			entPtrs.ptr13APower[entNum].getRef(device); break;
		case DEV_TYPE_LOCK:				entPtrs.ptrLock[entNum].getRef(device); break;
		case DEV_TYPE_LIGHT:			entPtrs.ptrLight[entNum].getRef(device); break;
		case DEV_TYPE_RELAY:			entPtrs.ptrRelay[entNum].getRef(device); break;
		case VAR_TYPE_BYTE:			  device[0] = 'v'; device[1] = 'B'; device[2] = '.'; device[3] = entNum / 10 + 0x30; device[4] = entNum % 10 + 0x30; device[5] = 0x00; break;
		case VAR_TYPE_2BYTE:			device[0] = 'v'; device[1] = 'I'; device[2] = '.'; device[3] = entNum / 10 + 0x30; device[4] = entNum % 10 + 0x30; device[5] = 0x00; break;
		case VAR_TYPE_RFID:				device[0] = 'v'; device[1] = 'R'; device[2] = '.'; device[3] = entNum / 10 + 0x30; device[4] = entNum % 10 + 0x30; device[5] = 0x00; break;
		case OBJ_TYPE_ZONE:				device[0] = entPtrs.ptrZone[entNum].get(VAL_REGION); device[1] = entPtrs.ptrZone[entNum].get(VAL_ZONE) + 0x30; device[2] = 0x00; break;
	}  
}

void HA_root::getSnapshot(byte entType, byte entNum) {
	// Check bounds
	if (_numEnts[entType] < entNum) Serial.println("Getref: put OO bounds");
	
	switch (entType) {
		case DEV_TYPE_TOUCH:			entPtrs.ptrTouch[entNum].getSnapshot(); break;
		case DEV_TYPE_FIRE:				entPtrs.ptrFire[entNum].getSnapshot(); break;
		case DEV_TYPE_HEAT:			  entPtrs.ptrHeat[entNum].getSnapshot(); break;
		case DEV_TYPE_LUMINANCE:  entPtrs.ptrLuminance[entNum].getSnapshot(); break;
		case DEV_TYPE_MOTION:			entPtrs.ptrMotion[entNum].getSnapshot(); break;
		case DEV_TYPE_PRESENCE:		entPtrs.ptrPresence[entNum].getSnapshot(); break;
		case DEV_TYPE_RFID:				entPtrs.ptrDevRFID[entNum].getSnapshot(); break;	
		case DEV_TYPE_OPEN:				entPtrs.ptrOpen[entNum].getSnapshot(); break;
		case DEV_TYPE_5APWR:			entPtrs.ptr5APower[entNum].getSnapshot(); break;
		case DEV_TYPE_13APWR:			entPtrs.ptr13APower[entNum].getSnapshot(); break;
		case DEV_TYPE_LOCK:				entPtrs.ptrLock[entNum].getSnapshot(); break;
		case DEV_TYPE_LIGHT:			entPtrs.ptrLight[entNum].getSnapshot(); break;
		case DEV_TYPE_RELAY:			entPtrs.ptrRelay[entNum].getSnapshot(); break;
		case OBJ_TYPE_ZONE:				entPtrs.ptrZone[entNum].getSnapshot(); break; 
		default: Serial.println();
	}  
}

boolean HA_root::logChange(byte entType, byte entNum, byte val) {
	if (entType != DEV_TYPE_TOUCH) Serial.println("Wrong type");
	
	entPtrs.ptrTouch[entNum].logChange(val);
}

void HA_root::putEnt(byte entType, byte entNum, byte valType, void *valPtr) {		// Suitable for all devices & variables
	// Check bounds
	if (_numEnts[entType] < entNum) Serial.println("HA_root: put OO bounds");

	switch (entType) {
		case DEV_TYPE_RFID:				entPtrs.ptrDevRFID[entNum].put(valType, (byte*)valPtr); break;
		case VAR_TYPE_RFID:				entPtrs.ptrVarRFID[entNum].put((byte*)valPtr); break;
		default:									putEnt(entType, entNum, valType, *(unsigned int*)valPtr);
	}
}

void HA_root::getEnt(byte entType, byte entNum, byte valType, void *valPtr) {		// Suitable for all devices & variables
	// Check bounds
	if (_numEnts[entType] < entNum) Serial.println("HA_root: get OO bounds2");

	switch (entType) {
		case DEV_TYPE_RFID:				entPtrs.ptrDevRFID[entNum].get(valType, (byte*)valPtr); break;
		case VAR_TYPE_RFID:				entPtrs.ptrVarRFID[entNum].get((byte*)valPtr); break;
		default:									*(unsigned int*)valPtr = getEnt(entType, entNum, valType);
	}
}

byte *HA_root::getBufPtr(byte entType, byte entNum, byte valType) {			// Only relevant for multi-byte devices/variables
	// Check bounds
	if (_numEnts[entType] < entNum) Serial.println("HA_root: put OO bounds");
	
	switch (entType) {
		case DEV_TYPE_RFID:				return entPtrs.ptrDevRFID[entNum].getBufPtr(valType);
		case VAR_TYPE_RFID:				return entPtrs.ptrVarRFID[entNum].getBufPtr();
	}
}


void HA_root::putEntElem(byte entType, byte entNum, byte valType, byte elemIdx, byte elemVal) {		// Only relevant for multi-byte devices/variables
	// Check bounds
	if (_numEnts[entType] < entNum) Serial.println("HA_root: put OO bounds");
	
	switch (entType) {
		case DEV_TYPE_RFID:				entPtrs.ptrDevRFID[entNum].putElem(valType, elemIdx, elemVal); break;
		case VAR_TYPE_RFID:				entPtrs.ptrVarRFID[entNum].putElem(elemIdx, elemVal); break;
		default:									Serial.println("Err: putEntElem");
	}
}

byte HA_root::getEntElem(byte entType, byte entNum, byte valType, byte elemIdx) {		// Only relevant for multi-byte devices/variables
	// Check bounds
	if (_numEnts[entType] < entNum) Serial.println("HA_root: get OO bounds3");
	
	switch (entType) {
		case DEV_TYPE_RFID:				return entPtrs.ptrDevRFID[entNum].getElem(valType, elemIdx); 
		case VAR_TYPE_RFID:				return entPtrs.ptrVarRFID[entNum].getElem(elemIdx); 
		default:									Serial.println("Err: getEntElem");
	}
}

void HA_root::initDev(byte devType, byte devNum, byte channel, byte pin, byte handler) {   
	// Check bounds
	if (_numEnts[devType] < devNum) {
		Serial.println("initDev: OO bounds");
		Serial.println(devType);
		Serial.println(_numEnts[devType]);
		Serial.println(devNum);
	}
	
	switch (devType) {
		case DEV_TYPE_TOUCH:			entPtrs.ptrTouch[devNum].initDev(devNum, channel, pin, handler); break;
		case DEV_TYPE_FIRE:				entPtrs.ptrFire[devNum].initDev(devNum, channel, pin, handler); break; 
		case DEV_TYPE_HEAT:			  entPtrs.ptrHeat[devNum].initDev(devNum, channel, pin, handler); break; 
		case DEV_TYPE_LUMINANCE:  entPtrs.ptrLuminance[devNum].initDev(devNum, channel, pin, handler); break; 
		case DEV_TYPE_MOTION:			entPtrs.ptrMotion[devNum].initDev(devNum, channel, pin, handler); break; 
		case DEV_TYPE_PRESENCE:		entPtrs.ptrPresence[devNum].initDev(devNum, channel, pin, handler); break; 
		case DEV_TYPE_RFID:				entPtrs.ptrDevRFID[devNum].initDev(devNum, channel, pin, handler); break; 	
		case DEV_TYPE_OPEN:				entPtrs.ptrOpen[devNum].initDev(devNum, channel, pin, handler); break; 
		case DEV_TYPE_5APWR:			entPtrs.ptr5APower[devNum].initDev(devNum, channel, pin, handler); break; 
		case DEV_TYPE_13APWR:			entPtrs.ptr13APower[devNum].initDev(devNum, channel, pin, handler); break; 
		case DEV_TYPE_LOCK:				entPtrs.ptrLock[devNum].initDev(devNum, channel, pin, handler); break; 
		case DEV_TYPE_LIGHT:			entPtrs.ptrLight[devNum].initDev(devNum, channel, pin, handler); break; 
		case DEV_TYPE_RELAY:			entPtrs.ptrRelay[devNum].initDev(devNum, channel, pin, handler); break; 
		default:									Serial.println("Err: initDev");		
	}
};

void HA_root::resetDev(byte devType, byte devNum) {
	// Check bounds
	if (_numEnts[devType] < devNum) Serial.println("initDev: OO bounds");
	
	switch (devType) {
//		case DEV_TYPE_TOUCH:			entPtrs.ptrTouch[devNum].resetDev(); break;
//		case DEV_TYPE_FIRE:				entPtrs.ptrFire[devNum].resetDev(); break; 
		case DEV_TYPE_HEAT:			  entPtrs.ptrHeat[devNum].resetDev(); break; 
//		case DEV_TYPE_LUMINANCE:  entPtrs.ptrLuminance[devNum].resetDev(); break; 
//		case DEV_TYPE_MOTION:			entPtrs.ptrMotion[devNum].resetDev(); break; 
//		case DEV_TYPE_PRESENCE:		entPtrs.ptrPresence[devNum].resetDev(); break; 
//		case DEV_TYPE_RFID:				entPtrs.ptrDevRFID[devNum].resetDev(); break; 	
//		case DEV_TYPE_OPEN:				entPtrs.ptrOpen[devNum].resetDev(); break; 
//		case DEV_TYPE_5APWR:			entPtrs.ptr5APower[devNum].resetDev(); break; 
//		case DEV_TYPE_13APWR:			entPtrs.ptr13APower[devNum].resetDev(); break; 
//		case DEV_TYPE_LOCK:				entPtrs.ptrLock[devNum].resetDev(); break; 
//		case DEV_TYPE_LIGHT:			entPtrs.ptrLight[devNum].resetDev(); break; 
//		case DEV_TYPE_RELAY:			entPtrs.ptrRelay[devNum].resetDev(); break; 
		default:									Serial.println("Err: resetDev");		
	}
};

void HA_root::readDev(byte devType, byte devNum) {						// Sensors
	// Check bounds
	if (_numEnts[devType] < devNum) Serial.println("readDev: OO bounds");
	
	switch (devType) {
		case DEV_TYPE_TOUCH:			entPtrs.ptrTouch[devNum].readDev(); break;
		case DEV_TYPE_FIRE:				entPtrs.ptrFire[devNum].readDev(); break; 
		case DEV_TYPE_HEAT:			  entPtrs.ptrHeat[devNum].readDev(); break; 
		case DEV_TYPE_LUMINANCE:  entPtrs.ptrLuminance[devNum].readDev(); break; 
		case DEV_TYPE_MOTION:			entPtrs.ptrMotion[devNum].readDev(devNum); break; 
		case DEV_TYPE_PRESENCE:		entPtrs.ptrPresence[devNum].readDev(); break; 
		case DEV_TYPE_RFID:				entPtrs.ptrDevRFID[devNum].readDev(); break; 	
		case DEV_TYPE_OPEN:				entPtrs.ptrOpen[devNum].readDev(); break;  
		default:									Serial.println("Err: readDev");		
	}
};

void HA_root::setDev(byte devType, byte devNum, byte val) {			// Actors
	// Check bounds
	if (_numEnts[devType] < devNum) Serial.println("setDev: OO bounds");
	
	switch (devType) { 
		case DEV_TYPE_5APWR:			entPtrs.ptr5APower[devNum].setDev(val); break; 
		case DEV_TYPE_13APWR:			entPtrs.ptr13APower[devNum].setDev(val); break; 
		case DEV_TYPE_LOCK:				entPtrs.ptrLock[devNum].setDev(val); break; 
		case DEV_TYPE_LIGHT:			entPtrs.ptrLight[devNum].setDev(val); break; 
		case DEV_TYPE_RELAY:			entPtrs.ptrRelay[devNum].setDev(val); break; 
		default:									Serial.println("Err: setDev");		
	}
};

void HA_root::processDevInt(byte devType, byte devNum, byte alertingPin) {
	// Check bounds
	if (_numEnts[devType] < devNum) Serial.println("devISR: OO bounds");
	
	switch (devType) {
//		case DEV_TYPE_TOUCH:			entPtrs.ptrTouch[devNum].readPin(alertingPin); break;
//		case DEV_TYPE_FIRE:				entPtrs.ptrFire[devNum].readPin(alertingPin); break; 
//		case DEV_TYPE_HEAT:			  entPtrs.ptrHeat[devNum].readPin(alertingPin); break; 
//		case DEV_TYPE_LUMINANCE:  entPtrs.ptrLuminance[devNum].readPin(alertingPin); break; 
		case DEV_TYPE_MOTION:			entPtrs.ptrMotion[devNum].readPort(alertingPin); break; 
//		case DEV_TYPE_PRESENCE:		entPtrs.ptrPresence[devNum].readPin(alertingPin); break; 
//		case DEV_TYPE_RFID:				entPtrs.ptrDevRFID[devNum].readPin(alertingPin); break; 	
//		case DEV_TYPE_OPEN:				entPtrs.ptrOpen[devNum].readPin(alertingPin); break;  
		default:									Serial.println("Err: readDev");		
	}
};

void HA_root::devISR(byte devType, byte devNum, byte alertingPin) {
	// Check bounds
	if (_numEnts[devType] < devNum) Serial.println("devISR: OO bounds");
	
	switch (devType) {
//		case DEV_TYPE_TOUCH:			entPtrs.ptrTouch[devNum].devISR(alertingPin); break;
//		case DEV_TYPE_FIRE:				entPtrs.ptrFire[devNum].devISR(alertingPin); break; 
//		case DEV_TYPE_HEAT:			  entPtrs.ptrHeat[devNum].devISR(alertingPin); break; 
//		case DEV_TYPE_LUMINANCE:  entPtrs.ptrLuminance[devNum].devISR(alertingPin); break; 
		case DEV_TYPE_MOTION:			entPtrs.ptrMotion[devNum].devISR(alertingPin); break; 
//		case DEV_TYPE_PRESENCE:		entPtrs.ptrPresence[devNum].devISR(alertingPin); break; 
//		case DEV_TYPE_RFID:				entPtrs.ptrDevRFID[devNum].devISR(alertingPin); break; 	
//		case DEV_TYPE_OPEN:				entPtrs.ptrOpen[devNum].devISR(alertingPin); break;  
		default:									Serial.println("Err: readDev");		
	}
};
	
// ********************** Channels **********************

boolean HA_root::createChanArray(byte numChans) {		// Finds space for array of channels
	int classSize = sizeof(HA_channel);
		
	// Avoid duplicates
	if (_numChannels != 0) {Serial.println("HA_root: dup Chan array"); return false;}
	
	ptrChannel = (HA_channel*)malloc(classSize * numChans);					// Get some space
	
	if (ptrChannel != NULL) {														// Attach array of channels to root pointer
		_numChannels = numChans;
		for (int chan = 0; chan < numChans; chan++) ptrChannel->put(VAL_CHAN_TYPE, 0);
		return true;
	}
	else {
		_numChannels = 0; 
		return false;
	}
}

byte HA_root::numChans() {
	return _numChannels;
}

boolean HA_root::initChan(byte chanNum, byte protocolType, byte maxPins, byte ioPin) {
	if (_numChannels <= chanNum) Serial.print("Err: putChan ");
	else return ptrChannel[chanNum].initChan(protocolType, maxPins, ioPin);
}

boolean HA_root::initChanAccess(byte chanNum, byte accessType, byte latchPin, byte dataPin, byte clockPin) {
	return ptrChannel[chanNum].initChanAccess(accessType, latchPin, dataPin, clockPin);
}

boolean HA_root::initChanAlert(byte chanNum, byte alertType, byte intNum, byte intMode, byte resetPin, byte slaveSelectPin, byte slaveSelectPinBank0, byte slaveSelectPinBank1) {
	return ptrChannel[chanNum].initChanAlert(alertType, chanNum, intNum, intMode, resetPin, slaveSelectPin, slaveSelectPinBank0, slaveSelectPinBank1);
}

byte HA_root::getChan(byte chanNum, byte type) {
	if (_numChannels <= chanNum) Serial.print("Err: getChan");
	else return ptrChannel[chanNum].get(type);
}

HA_channel* HA_root::getChanObj(byte chanNum) { 
	return ptrChannel[chanNum].getChanObj(); 
};

void HA_root::chanISR(byte chanNum) {
	ptrChannel[chanNum].chanISR();
};

boolean HA_root::registerDevRange(byte chanNum, byte rangeNum, byte intPin, byte devType, byte devNum) {
	if (_numChannels <= chanNum) Serial.print("Err: regDevISR");
	else ptrChannel[chanNum].registerDevRange(rangeNum, intPin, devType, devNum);
}
	

// ********************** Evaluations **********************

boolean HA_root::createEvalArray(byte numEvals) {		// Finds space for array of evaluations
	int classSize = sizeof(HA_evaluation);
		
	// Avoid duplicates
	if (_numEvals != 0) {Serial.println("HA_root: dup Eval array"); return false;}
	
	ptrEvaluation = (HA_evaluation*)malloc(classSize * numEvals);					// Get some space
	
	if (ptrEvaluation != NULL) {														// Attach arrray of evaluations to root pointer
		_numEvals = numEvals;
		return true;
	}
	else {
		_numEvals = 0; 
		return false;
	}
}

byte HA_root::numEvals() {
	return _numEvals;
}

void HA_root::putEval(byte evalNum, byte valType, byte val) {
	if (_numEvals <= evalNum) Serial.print("Err: putEval ");
	else ptrEvaluation[evalNum].put(valType, val); 
}

void HA_root::putEval(byte evalNum, byte valCalc, byte valAType, byte valA, byte valExp, byte valBType, byte valB) {
	if (_numEvals <= evalNum) Serial.print("Err: putEval ");
	else ptrEvaluation[evalNum].put(valCalc, valAType, valA, valExp, valBType, valB);
}

byte HA_root::getEval(byte evalNum, byte valType) {
	if (_numEvals <= evalNum) Serial.print("Err: getEval ");
	else return ptrEvaluation[evalNum].get(valType); 
}

void HA_root::getEval(byte evalNum, byte *valCalc, byte *valAType, byte *valA, byte *valExp, byte *valBType, byte *valB) {
	if (_numEvals <= evalNum) Serial.print("Err: getEval");
	else ptrEvaluation[evalNum].get(valCalc, valAType, valA, valExp, valBType, valB);
}
	

// ********************** Arg Lists **********************

boolean HA_root::createArgListArray(byte numArgLists) {		// Finds space for array of arg lists
	int classSize = sizeof(HA_argList);
	
  // Avoid duplicates
	if (_numArgLists != 0) {Serial.println("HA_root: dup ArgList array"); return false;}
	
	ptrArgList = (HA_argList*)malloc(classSize * numArgLists);					// Get some space
	
	if (ptrArgList != NULL) {														// Attach arrray of evaluations to root pointer
		_numArgLists = numArgLists;
		return true;
	}
	else {
		_numArgLists = 0; 
		return false;
	}
}

byte HA_root::numArgLists() {
	return _numArgLists;
}

boolean HA_root::createArgList(byte argListNum, byte numArgs) {
	if (_numArgLists <= argListNum) Serial.print("Err: createArgList");
	else return ptrArgList[argListNum].create(numArgs);
}


byte HA_root::numArgs(byte argListNum) {
	return ptrArgList[argListNum].numArgs();
}

void HA_root::putArg(byte argListNum, byte argNum, byte val) {
	if (_numArgLists <= argListNum || numArgs(argListNum) <= argNum) Serial.print("Err: putArg ");
	else ptrArgList[argListNum].put(argNum, val); 
}

byte HA_root::getArg(byte argListNum, byte argNum) {
	if (_numArgLists <= argListNum || numArgs(argListNum) <= argNum) Serial.print("Err: getArg ");
	else return ptrArgList[argListNum].get(argNum); 
}



// **************** Perform evaluation  ******************


boolean HA_root::equal(byte valAType, byte *evalAPtr, byte *evalBPtr) {
	for (int i = 0; i < entBufLen(valAType); i++) if (evalAPtr[i] != evalBPtr[i]) return false;  
	
	return true;
}


void HA_root::runEval(byte evalNum, unsigned int *evalPtr) { 
  byte valCalc, valAType, valA, valExp, valBType, valB;
  
	if (_numEvals <= evalNum) {
		Serial.print("Err: runEval1 - ");
		Serial.println(evalNum);
	}
  
  getEval(evalNum, &valCalc, &valAType, &valA, &valExp, &valBType, &valB);
  
  switch (valAType) {
  	case VAR_TYPE_RFID: 
  	case DEV_TYPE_RFID: {
	  		if (valBType != DEV_TYPE_RFID && valBType != VAR_TYPE_RFID) { Serial.println("Err: runEvalMulti"); return; }
	
				// Calc determines how to interpret valA - either current (always if VAR) or previous
				byte *evalAPtr = getBufPtr(valAType, valA, (valAType == VAR_TYPE_RFID || valCalc == CALC_DEV) ? VAL_CURR : VAL_PREV);	// Pointer to current/previous buffer of device indexed by valA
				
				// valB always current
				byte *evalBPtr = getBufPtr(valBType, valB, VAL_CURR);
				
				// Limited range of valid expressions for multi-byte
				switch (valExp) {
					case EXP_SET:			putEnt(valBType, valB, VAL_PUSH, evalAPtr); *evalPtr = 1; break;
					case EXP_EQ:			*evalPtr = equal(valAType, evalAPtr, evalBPtr); break;
					case EXP_NEQ: 		*evalPtr = !equal(valAType, evalAPtr, evalBPtr); break;
					case EXP_NOOP:		break;				// Ignore
					default:					Serial.println("Err: runEval2");		
				}
  		}
			break;
  	default: {
  		unsigned int evalA = getValA(valCalc, valAType, valA); 	
			runExp(valCalc, valAType, evalA, valExp, valBType, valB, evalPtr);
		}
	}		
}



unsigned int HA_root::getValA(byte valCalc, byte valAType, byte valA) {	
	unsigned int temp = 0, evalA = 0;
	
  // Calc determines how to interpret valA
	switch (valCalc) {
		case CALC_DEV: 			return getEnt(valAType, valA, VAL_CURR); break;			// Current value of device/variable indexed by valA
		case CALC_NDEV: 		return !getEnt(valAType, valA, VAL_CURR); break;			// Inverse of current value of device/variable indexed by valA		
		case CALC_PDEV: 		return getEnt(valAType, valA, VAL_PREV); break;			// Previous value of device (not variable) indexed by valA
		case CALC_ROFC: {
			int valCurr = getEnt(valAType, valA, VAL_CURR), valPrev = getEnt(valAType, valA, VAL_PREV);
			int diff = (valCurr - valPrev);
			int rofc = (diff * 100) / valPrev;
			return (unsigned int)rofc;
			break;
		}
		case CALC_ON: 			return getEnt(valAType, valA, VAL_CURR) && !getEnt(valAType, valA, VAL_PREV);	// Curr vs prev of device indexed by valA
		case CALC_OFF: 			return !getEnt(valAType, valA, VAL_CURR) && getEnt(valAType, valA, VAL_PREV); 	// Curr vs prev of device indexed by valA
		case CALC_YEAR: 		return year(); 																// Current year
		case CALC_MONTH: 		return month();
		case CALC_NOW: 			return dhmNow();															// Current time (in day, hour, month format)				
		case CALC_AVG: {				// Average of entities in argument list indexed by valA
			for (int i = 0; i < numArgs(valA); i++) evalA += getEnt(valAType, getArg(valA, i), VAL_CURR);
			return evalA / numArgs(valA);
		}
		case CALC_AND: {				// Logical AND of entities in argument list indexed by valA
			evalA = getEnt(valAType, getArg(valA, 0), VAL_CURR);
			for (int i = 1; i < numArgs(valA) && evalA != 0; i++) evalA = getEnt(valAType, getArg(valA, i), VAL_CURR);            // For AND, quit on a false
			return evalA;
		}
		case CALC_OR: {					// Logical OR of entities in argument list indexed by valA
			evalA = getEnt(valAType, getArg(valA, 0), VAL_CURR);
			for (int i = 1; i < numArgs(valA) && evalA == 0; i++) evalA = getEnt(valAType, getArg(valA, i), VAL_CURR);            // For OR, quit on a true
			return evalA;
		}
		case CALC_AVGV: {				// Average of evaluations in argument list indexed by valA
			for (int i = 0; i < numArgs(valA); i++) {
				runEval(getArg(valA, i), &temp);
				evalA += temp;
			}
			return evalA / numArgs(valA);
		}
		case CALC_ANDV: {				// Logical AND of evaluations in argument list indexed by valA
			runEval(getArg(valA, 0), &evalA);
			for (int i = 1; i < numArgs(valA) && evalA != 0; i++) runEval(getArg(valA, i), &evalA);            // For AND, quit on a false
			return evalA;
		}
		case CALC_ORV: {					// Logical OR of evaluations in argument list indexed by valA
			runEval(getArg(valA, 0), &evalA);
			for (int i = 1; i < numArgs(valA) && evalA == 0; i++) runEval(getArg(valA, i), &evalA);            // For OR, quit on a true
			return evalA;
		}
		case CALC_EVAL:			
			runEval(valA, &evalA); 
			return evalA;
	}
}

void HA_root::runExp(byte valCalc, byte valAType, unsigned int evalA, byte valExp, byte valBType, byte valB, unsigned int *evalPtr) {
	unsigned int evalB = getEnt(valBType, valB, VAL_CURR);
	
	switch (valExp) {
		case EXP_SET:						// If valA == variable, non-zero single device or calculated, then set valB = valA or TRUE (if binary valB)
			if (((BINARY_DEV >> valBType) & 1) && evalA > 0) evalA = 1;			// If destination is on/off then ensure result is either 0 or 1
			
			switch (valCalc) {		// Figure out if valA is a variable or device
				case CALC_DEV: 			
				case CALC_NDEV: 		
				case CALC_PDEV: 		
				case CALC_ROFC: 		
				case CALC_ON: 			
				case CALC_OFF: 			// valA based on variable or non-zero single device 
					if (valAType ==  VAR_TYPE_BYTE || valAType ==  VAR_TYPE_2BYTE || valAType ==  VAR_TYPE_RFID || evalA > 0) putEnt(valBType, valB, VAL_PUSH, evalA); 
					break;
				default:						// valA has been calculated - use valA (including 0 value)
					putEnt(valBType, valB, VAL_PUSH, evalA); 
			} 
			
			*evalPtr = evalA;
			break;
		case EXP_UNSET:			if (evalA > 0) putEnt(valBType, valB, VAL_PUSH, OFF); *evalPtr = 0; break;
		case EXP_ADD:				*evalPtr =  evalA + evalB; break;
		case EXP_SUB:  			*evalPtr =  evalA - evalB; break;
		case EXP_MULT:			*evalPtr =  evalA * evalB; break;
		case EXP_DIV:				*evalPtr =  evalA / evalB; break;
		case EXP_AND:				*evalPtr =  evalA && evalB; break;
		case EXP_OR:				*evalPtr =  evalA || evalB; break;
		case EXP_EQ:				*evalPtr =  evalA == evalB; break;
		case EXP_NEQ:				*evalPtr =  evalA != evalB; break;
		case EXP_GT:				*evalPtr =  evalA > evalB; break;
		case EXP_LT:				*evalPtr =  evalA < evalB; break;
		case EXP_BTW:
		case EXP_NOT_BTW: {
      unsigned int dhmADay = dhmGet(evalA, VAL_DAY), dhmBDay = dhmGet(evalB, VAL_DAY), timeNow = dhmMake(1,22,35);
      boolean between = false, nextWeek = false;
     
      switch (dhmADay) {			// NB default case results in a return 
        case 0:      dhmADay = 1; dhmBDay = 7; break;        			// Daily
        case 8:      dhmADay = 2; dhmBDay = 5; break;        			// Mon-Thu
        case 9:      dhmADay = 2; dhmBDay = 6; break;        			// Mon-Fri
        case 10:     dhmADay = 7; dhmBDay = 1; nextWeek = true; break;        			// Weekend 
        default:		 							// Specific days. dhmADay and dhmBDay already set - single test														
					// Test if now is between the times stated - allow for going into next week
					nextWeek = evalB < evalA;
      		between = (nextWeek) ? timeNow >= evalA || timeNow <= evalB : timeNow >= evalA && timeNow <= evalB;
    			*evalPtr = (valExp == EXP_BTW) ? between : !between;    // Swap logic if not between  
      }

      // Only here if range of days to be tested - cycle through same times each day
      for (int i = dhmADay; i <= ((nextWeek) ? 7 : dhmBDay); i++) {
	      dhmPut (&evalA, VAL_DAY, i);
	      dhmPut (&evalB, VAL_DAY, i);
	      
	      // Test if time now is between the times stated; if so break out of loop
	      if (timeNow >= evalA && timeNow <= evalB) {
		      between = true;
		      break;
	      }
      }
      // Second loop needed if end date is next week
      if (nextWeek) {
      	for (int i = 1; i <= dhmBDay; i++) {
	      	dhmPut (&evalA, VAL_DAY, i);
	      	dhmPut (&evalB, VAL_DAY, i);
	      
		      // Test if time now is between the times stated; if so break out of loop
		      if (timeNow >= evalA && timeNow <= evalB) {
			      between = true;
			      break;
		      }
	      }
      }
	      
    	*evalPtr = (valExp == EXP_BTW) ? between : !between;    // Swap logic if not between  
      break;
    }
		case EXP_NOOP:			
			*evalPtr = evalA;
			break;
	}
}


void HA_root::printRoot() {
	char devTypeChar[3];
	char buffer[10];
	
	Serial.println();
	Serial.println("Print Root");
	Serial.println("Entities");
	for (int devType = 0; devType < NUM_DEV_TYPES + NUM_VAR_TYPES + NUM_ZONE_TYPES; devType++) {
		Serial.print("devType = ");
		Serial.print(devType);
		Serial.print(" - ");
		getDevTypeChar(devType, devTypeChar);   
		Serial.print(devTypeChar);
		
		Serial.print(" class size = ");
		Serial.print(classSize(devType), DEC);
		
		Serial.print(" num ents = ");
		Serial.print(numEnts(devType), DEC);
		
		Serial.print(" addr = ");
		Serial.print((unsigned int)_entPtrs[devType],DEC);
		
		if (devType == DEV_TYPE_RFID || devType == VAR_TYPE_RFID) {
			unsigned int bufLen = entBufLen(devType) * numEnts(devType) * ((devType == DEV_TYPE_RFID) ? 2 : 1); 
			Serial.print(" buffer len = ");
			Serial.print(bufLen);
			Serial.print(" space = ");
			Serial.print((unsigned int)getBufPtr(devType, 0, VAL_CURR), DEC);
		}
		Serial.println();
		
		for (int devNum = 0; devNum < numEnts(devType); devNum++) {
			Serial.print("  Ent ");
			Serial.print(getEnt(devType, devNum, VAL_DEVIDX));
			Serial.print(" = ");
			getRef(devType, devNum, buffer);
			Serial.print(buffer);
			Serial.print(" ");
			getSnapshot(devType, devNum);
		}
		Serial.println();
	}
	
	Serial.println();
	Serial.println("Evaluations");
	Serial.print("Num evals = ");
	Serial.print(_numEvals, DEC);
	Serial.print(" addr = ");
	Serial.println((unsigned int)ptrEvaluation,DEC);
	
	Serial.println();
	Serial.println("Arglists");
	Serial.print("Num arglists = ");
	Serial.print(_numArgLists, DEC);
	Serial.print(" addr = ");
	Serial.println((unsigned int)ptrArgList,DEC);
	
	Serial.println();
	Serial.print("Channels");
	Serial.print(" class size = ");
	Serial.print(sizeof(HA_channel), DEC);
	Serial.print(" num channels = ");
	Serial.print(_numChannels, DEC);
	Serial.print(" addr = ");
	Serial.println((unsigned int)ptrChannel,DEC);
	
}


HA_root root;



