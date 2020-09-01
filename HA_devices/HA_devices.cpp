 /*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
    
    Contains the HA_device library to manage the directory of devices (sensors or actors)
		HA_devHeat and HA_devMotion held separately

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

#include "HA_devices.h"
#include "Wakeup.h"
#include "HA_root.h"
#include "HA_switcher.h"



// **************** HA_devTouch ***************
//
// Sensor - on or off

void HA_devTouch::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_TOUCH);
}

void HA_devTouch::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_TOUCH);
}

boolean HA_devTouch::logChange(byte val) {
	if (changeList.put(DEV_TYPE_TOUCH) && changeList.put(HA_device::get(VAL_DEVIDX)) && changeList.put(val)) return true;
	else return false;
}

// **************** HA_devFire ***************
//
// Sensor - on or off

void HA_devFire::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_FIRE);
}

void HA_devFire::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_FIRE);
}

// **************** HA_devLuminance ***************
//
// Light dependent resistor - yields value in 1/100th volts

void HA_devLuminance::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_LUMINANCE);
}

void HA_devLuminance::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_LUMINANCE);
}

// **************** HA_devPresence ***************
//
// Sensor - on or off

void HA_devPresence::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_PRESENCE);
}

void HA_devPresence::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_PRESENCE);
}

// **************** HA_devRFID ****************
//
// Sensor - 

HA_devRFID::HA_devRFID() : HA_devMultiByte(_bufLen) {
};

boolean HA_devRFID::create() {
	return HA_devMultiByte::create(_bufLen);
};

void HA_devRFID::init(byte *addr) {
	HA_devMultiByte::init(addr, _bufLen);
};

void HA_devRFID::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_RFID);
}

void HA_devRFID::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_RFID);
}

void HA_devRFID::initDev(byte devNum, byte channel, byte pin, byte handler) {
	pinMode(pin, INPUT); 
	put(VAL_PIN, pin);
	put(VAL_HANDLER, handler);
	put(VAL_STATUS, STATUS_READY);
	put(VAL_DEVIDX, devNum);
};

void HA_devRFID::readDev() {
};

// **************** HA_devOpen ***************
//
// Open sensor, based on Hall Effect switches - yields value in 1/100th volts, typically 2.5v (250) quiescent

void HA_devOpen::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_OPEN);
}

void HA_devOpen::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_OPEN);
}

void HA_devOpen::readDev(byte devType){
	HA_sensAnalog::readDev(DEV_TYPE_OPEN);
}

// **************** HA_dev5APower ***************

void HA_dev5APower::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_5APWR);
}

void HA_dev5APower::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_5APWR);
}

// **************** HA_dev13APower ***************

void HA_dev13APower::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_13APWR);
}

void HA_dev13APower::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_13APWR);
}

// **************** HA_devLock ***************

void HA_devLock::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_LOCK);
}

void HA_devLock::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_LOCK);
}

// **************** HA_devLight ***************

void HA_devLight::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_LIGHT);
}

void HA_devLight::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_LIGHT);
}

// **************** HA_devRelay ***************

void HA_devRelay::getRef(char *device) {
	HA_device::getRef(device, DEV_TYPE_RELAY);
}

void HA_devRelay::getSnapshot() {
	HA_device::getSnapshot(DEV_TYPE_RELAY);
}
