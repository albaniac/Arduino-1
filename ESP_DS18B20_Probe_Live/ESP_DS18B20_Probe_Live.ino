/* This sketch reads temperature DS18DS sensor
 * Values are posted to an AWS IoT shadow.  The THING SHADOW is defined on the PUBLISH 
 * routine.  TEMP holds the temp and is an integer so can hold negative numbers.
 * ESP8266 goes into deep sleep between readings.  Sleep time is set using sleepTimeS.
 * Robin Harris 8th August 2016
 */

#include <AmazonIOTClient.h>
#include <Esp8266AWSImplementations.h>
#include <AWSFoundationalTypes.h>
#include "keys.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


const int sleepTimeS = 1800; //number of seconds to deep sleep

int temp = 0;      //integer to hold temperature

void printWiFiData();
void printCurrentNetwork();
void publish(const char *topic, int data1);
void getSensorReadings();

void setup() {
  // Pin configuration
  sensors.begin();  //start sensor

  WiFi.begin(wifiSsid, wifiPwd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
  }
 
  delay(10000); //wait for DS18B20 to be ready

  getSensorReadings();
  ESP.deepSleep(sleepTimeS * 1000000);
}  //end of setup

void loop() {
}  //end of main loop


void publish(const char *topic, int data1) {
  
  AmazonIOTClient iotClient;
  ActionError actionError;

  Esp8266HttpClient httpClient;
  Esp8266DateTimeProvider dateTimeProvider;

  iotClient.setAWSRegion(awsIotRegion);
  iotClient.setAWSEndpoint(awsIotEndpoint);
  iotClient.setAWSDomain(awsIotDomain);
  iotClient.setAWSPath("/things/Room_Temp/shadow");
  iotClient.setAWSKeyID(awsKeyID);
  iotClient.setAWSSecretKey(awsSecKey);
  iotClient.setHttpClient(&httpClient);
  iotClient.setDateTimeProvider(&dateTimeProvider);
  delay(50);

  MinimalString shadow = ("{\"state\":{\"reported\":{\"Temperature\":" + String(data1, DEC) + "}}}").c_str();
  char* result = iotClient.update_shadow(shadow, actionError);
} //end of publish to AWS IoT

void getSensorReadings() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  temp = sensors.getTempCByIndex(0);
  // post values to AWS IoT
  publish("temperature", temp);
} //end of getSensorReadings

