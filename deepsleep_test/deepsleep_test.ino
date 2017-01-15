
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


void setup() {
Serial.begin(9600);
Serial.println("Starting setup....");
delay(15000);
Serial.println("Going to sleep...");
ESP.deepSleep(15000000);
}

void loop() {
  // put your main code here, to run repeatedly:

}
