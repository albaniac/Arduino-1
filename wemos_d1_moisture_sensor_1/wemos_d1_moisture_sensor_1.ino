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

void publish(const char *topic, uint8_t data1);
void checkSoilMoisture();

uint8_t sensor = A0;
uint8_t sensorEnable = D1;

const int sleepTimeS = 30;


void setup() {

  Serial.begin(9600);
  
  WiFi.begin(wifiSsid, wifiPwd);
  while (WiFi.status() != WL_CONNECTED) {
     delay(50);
  }
  // Pin configuration
  pinMode(sensorEnable, OUTPUT);
 
  Serial.println("Started!");

  checkSoilMoisture();
  ESP.deepSleep(sleepTimeS * 1000000);
}

void loop() {
}

void checkSoilMoisture() {

  Serial.println("Checking soil moisture level");

  digitalWrite(sensorEnable, HIGH);
  delay(100);

  int sensorValue = analogRead(sensor);
  digitalWrite(sensorEnable, LOW);

  Serial.print("Sensor value: ");
  Serial.println(sensorValue);

  uint8_t moistureLevel = sensorValue / 1023.0f * 100;
 
  Serial.print("Moisture level: ");
  Serial.println(moistureLevel);
  publish("soil/moisture", moistureLevel);
}

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

  MinimalString shadow = ("{\"state\":{\"reported\":{\"Moisture\":" + String(data1, DEC) + "}}}").c_str();
  char* result = iotClient.update_shadow(shadow, actionError);
  Serial.print("result: ");
  Serial.println(result);
} //end of publish to AWS IoT

