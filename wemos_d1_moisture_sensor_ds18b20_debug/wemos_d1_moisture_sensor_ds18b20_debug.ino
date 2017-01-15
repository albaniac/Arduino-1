/*
 * This sketch is the debug version (with Serial output) for a Wemos D1 to 
 * sense soil moisture and temperature. The sensors are turned on and off as 
 * required to reduce electrolytic decay and power consumption.  The temperature
 * sensor is a DS18B20 one wire device.
 * If the moisture level is below a threshold D5 is used to turn on a pump for 
 * a set interval of time
 * Robin Harris 27th August 2016
 */

#include <AmazonIOTClient.h>
#include <Esp8266AWSImplementations.h>
#include <AWSFoundationalTypes.h>
#include "keys.h"
#include <OneWire.h>  // required for the communication with the DS18B20
#include <DallasTemperature.h> //handles the data from the DS18B20

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS D2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void publish(const char *topic, int data1, int data2);
void getSensorReadings(); // read the sensors.  Calls publish to upload to AWS
void addwater();  // run the pump to water the plant

uint8_t moisturePin = A0;  //Connect sensor to A0 - the only analogue input
uint8_t sensorEnable = D1;  //connect moisture sensor +ve supply to D1
uint8_t pumpPin = D5; // pin that controls the pump
const int sleepTimeS = 3600; //number of seconds to deep sleep
uint8_t pumpRunS = 5;  //number of seconds the pump will run for 


void setup() {
  //Serial.begin(9600);
  WiFi.begin(wifiSsid, wifiPwd);
  while (WiFi.status() != WL_CONNECTED) {
     delay(50);
  }
  // Pin configuration
  pinMode(sensorEnable, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);  //make sure the pump is OFF
 
  getSensorReadings();
  ESP.deepSleep(sleepTimeS * 1000000);
}

void loop() {
}

void getSensorReadings() {
  digitalWrite(sensorEnable, HIGH);
  //get temperature from DS18B20
  int temp;
  do {
    delay(1000);
    sensors.requestTemperatures(); // Send the command to get temperatures
    delay(20);
    // After we got the temperatures, we can print them here.
    // We use the function ByIndex, and as an example get the temperature from the first sensor only.
    temp = sensors.getTempCByIndex(0);
    } while (temp < -20 || temp > 84);
  int sensorValue = analogRead(moisturePin);
  digitalWrite(sensorEnable, LOW);
  //Serial.print("Temp: ");
//  Serial.println(temp);
//  Serial.print ("sensorValue: ");
//  Serial.println(sensorValue);
  //int moistureLevel = map(sensorValue, 0, 1023, 0, 100);
  //moistureLevel = constrain (moistureLevel, 0, 100);
  //Serial.print ("moistureLevel");
  //Serial.println(moistureLevel);
  // check if moisture level is low enough to need water adding
  if (sensorValue < 300 ) {
    addwater();
  }

 // post values to AWS IoT
  publish("$aws/things/moisture_temp_1", temp, sensorValue);
} //end of getSensorReadings

void addwater(){
//  Serial.print("Starting to add water: ");
//  Serial.println(millis());
  digitalWrite(pumpPin, HIGH);
  delay(pumpRunS *1000);
  digitalWrite(pumpPin, LOW);
//  Serial.print("Finshed adding water: ");
//  Serial.println(millis());
}


void publish(const char *topic, int data1, int data2) {
  
  AmazonIOTClient iotClient;
  ActionError actionError;

  Esp8266HttpClient httpClient;
  Esp8266DateTimeProvider dateTimeProvider;

  iotClient.setAWSRegion(awsIotRegion);
  iotClient.setAWSEndpoint(awsIotEndpoint);
  iotClient.setAWSDomain(awsIotDomain);
  iotClient.setAWSPath("/things/moisture_temp_1/shadow");
  iotClient.setAWSKeyID(awsKeyID);
  iotClient.setAWSSecretKey(awsSecKey);
  iotClient.setHttpClient(&httpClient);
  iotClient.setDateTimeProvider(&dateTimeProvider);
  delay(50);

  MinimalString shadow = ("{\"state\":{\"reported\":{\"Temperature\":" + String(data1, DEC) + ",\"Moisture\":" + String(data2, DEC) + "}}}").c_str();
  char* result = iotClient.update_shadow(shadow, actionError);
} //end of publish to AWS IoT

