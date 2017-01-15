/*
 * This is a test sketch to set up the Wemos D1 with a soil moisture sensor.
 * The sensor is turned on and off as required to reduce electrolytic decay 
 * 
 * Robin Harris 25th July 2016
 */

#include <AmazonIOTClient.h>
#include <Esp8266AWSImplementations.h>
#include <AWSFoundationalTypes.h>
#include "keys.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS D2


// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


void publish(const char *topic, uint8_t data1, uint8_t data2);
void getSensorReadings(); // read the sensors.  Calls publish to upload to AWS
void addwater();  // run the pump to water the plant

uint8_t sensor = A0;  //Connect sensor to A0 - the only analogue input
uint8_t sensorEnable = D1;  //connect moisture sensor +ve supply to D1
uint8_t pumpPin = D5; // pin that controls the pump
const int sleepTimeS = 3600; //number of seconds to deep sleep
uint8_t pumpRunS = 5;  //number of seconds the pump will run for 


void setup() {

  Serial.begin(9600);
  
  WiFi.begin(wifiSsid, wifiPwd);
  while (WiFi.status() != WL_CONNECTED) {
     delay(50);
  }
  Serial.println("Connected");
  // Pin configuration
  pinMode(sensorEnable, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);  //make sure the pump is OFF
  Serial.println("pumpPin LOW - Setup");
 
  Serial.println("Started!");

  getSensorReadings();
  ESP.deepSleep(sleepTimeS * 1000000);
}

void loop() {
}

void getSensorReadings() {
  digitalWrite(sensorEnable, HIGH);
  delay(10000);
  //get temperature from DS18B20
  Serial.println("Checking temperature ");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  Serial.print("Temperature for the device 1 (index 0) is: ");
  int temp = sensors.getTempCByIndex(0);
  Serial.println(temp);
  Serial.println("Checking soil moisture level");
  if (temp >84) temp = 0;

  int sensorValue = analogRead(sensor);
  digitalWrite(sensorEnable, LOW);

  Serial.print("Sensor value: ");
  Serial.println(sensorValue);

  uint8_t moistureLevel = sensorValue / 1023.0f * 100;
 
  Serial.print("Moisture level: ");
  Serial.println(moistureLevel);

  if (moistureLevel < 25 ) {
    addwater();
  }

 // post values to AWS IoT
  publish("$aws/things/moisture_temp_1", temp, moistureLevel);
} //end of getSensorReadings

void addwater(){
  digitalWrite(pumpPin, HIGH);
  Serial.println("pumpPin HIGH");
  delay(pumpRunS *1000);
  digitalWrite(pumpPin, LOW);
  Serial.println("pumpPin LOW");  
}

void publish(const char *topic, uint8_t data1, uint8_t data2) {
  
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
  Serial.print("result: ");
  Serial.println(result);
} //end of publish to AWS IoT

