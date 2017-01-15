/* This sketch reads temperature and humidity from a DS18DS sensor
 * Output is provided to a serial monitor and values are posted to an AWS IoT shadow
 * Robin Harris 21st July 2016
 * 
 * This version has no serial monitor
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


const int sleepTimeS = 30; //number of seconds to deep sleep

uint8_t temp = 0;      //integer to hold temperature

void publish(const char *topic, uint8_t data1);
void getSensorReadings();

void setup() {
  sensors.begin();  //start sensor
  WiFi.begin(wifiSsid, wifiPwd);
  while (WiFi.status() != WL_CONNECTED) {
     delay(50);
  }
}  //end of setup

void loop() {
  getSensorReadings();
  delay(60000);
 }  //end of main loop


void publish(const char *topic, uint8_t data1) {
  
  AmazonIOTClient iotClient;
  ActionError actionError;

  Esp8266HttpClient httpClient;
  Esp8266DateTimeProvider dateTimeProvider;

  iotClient.setAWSRegion(awsIotRegion);
  iotClient.setAWSEndpoint(awsIotEndpoint);
  iotClient.setAWSDomain(awsIotDomain);
  iotClient.setAWSPath("/things/soil_sensor_one/shadow");
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
  publish("soil/moisture", temp);
} //end of getSensorReadings

