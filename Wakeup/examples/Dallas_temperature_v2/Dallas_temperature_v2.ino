 /*
    Copyright (C) 2011  Andrew Richards
    Example demonstrating use of WAKEUP library for non-blocking read of Dallas One-wire temperature sensor
    
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



#include "Wakeup.h"
#include "TimerOne.h"            // NB: modified version of public TimerOne library - updates submitted
#include <OneWire.h>             
#include <DallasTemperature.h>

// ************  Dallas temperature sensing ***************
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 10

OneWire oneWire(ONE_WIRE_BUS);          // Setup oneWire instance
DallasTemperature sensors(&oneWire);    // Pass oneWire reference to Dallas Temperature. 
DeviceAddress tempDeviceAddress;        // Holds device address


void setup(void) {
  Serial.begin(9600);
 
  wakeup.init();      // Initial setup, must be called before any other use of wakeup
   
  // Assumes single temperature sensor for testing purposes
  sensors.begin();                  // Start up the library
  sensors.getAddress(tempDeviceAddress, 0);      // Get address of first device
  sensors.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
  sensors.setWaitForConversion(false);      // Turn off blocking wait for temperature (async operation)
    
  Serial.println("Starting temperature readings in 2 secs");

  wakeup.wakeMeAfter(requestTemperature, 2000, NULL, TREAT_AS_NORMAL);    // Initiate a temperature reading in 2000ms
}


void loop() {

  wakeup.runAnyPending();           // Place this call sufficiently frequently in main code to allow pending sleepers to run


}


void requestTemperature(void *dummy) {      // Initiate read of temperature and create sleeper to do the actual read
  
  Serial.print("Temperature = ");
  
  // Initiate temperature detection
  sensors.requestTemperatures();

  // Which happens after 750ms for 10bit precision
  wakeup.wakeMeAfter(readTemperature, 750, NULL, TREAT_AS_NORMAL);      // tempC is volatile, so need to cast
       
}


void readTemperature(void *dummy) {                     // Get temperature after 750ms and in response to runAnyPending 


  // Should have the temperature, print it out
  Serial.print(sensors.getTempC(tempDeviceAddress));
  Serial.println("C");
  
  wakeup.wakeMeAfter(requestTemperature, 500, NULL, TREAT_AS_NORMAL);
}
  

