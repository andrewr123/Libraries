#define old

#ifdef old
    #include <OneWire.h>
    #include <DallasTemperature.h>
#else
    #include "HA_globals.h"
    #include "wakeup.h"
    #include "TimerOne.h"
    #include "HA_temperature.h"
#endif

// Data wire is plugged into port 5 on the Arduino
#define TEMP_SENSOR_POWER_PIN 2
#define TEMP_SENSOR_PIN 7

#ifdef old
    // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
    OneWire oneWire(TEMP_SENSOR_PIN);

    // Pass our oneWire reference to Dallas Temperature. 
    DallasTemperature sensors(&oneWire);
#else
    HA_temperature tSensor;
#endif

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.println("Dallas Temperature IC Control Library Demo");
#ifdef old
#else
  // Provide power to sensors
  pinMode(TEMP_SENSOR_POWER_PIN, OUTPUT);
  digitalWrite(TEMP_SENSOR_POWER_PIN, HIGH);
#endif

#ifdef old
  // Start up the library
  sensors.begin();
#else
  tSensor.init(TEMP_SENSOR_PIN, 10, 2000);
#endif
}

void loop(void)
{ 
#ifdef old
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");

  Serial.print("Temperature for the device is: ");
  Serial.println(sensors.getTempCByIndex(0));
#else
  const byte bufLen = 32;
  char buffer[bufLen];
  if (listErrors(buffer, bufLen)) Serial.println(buffer);
  Serial.print("Temp: ");
  Serial.println(tSensor.getTempC()); 
#endif

  delay(2000);
}
