/* This sketch reads temperature and humidity from a DS18DS sensor
 * Output is provided to a serial monitor and values are posted to an AWS IoT shadow
 * Robin Harris 21st July 2016
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


const int sleepTimeS = 600; //number of seconds to deep sleep

uint8_t temp = 0;      //integer to hold temperature

void printWiFiData();
void printCurrentNetwork();
void publish(const char *topic, uint8_t data1);
void getSensorReadings();

void setup() {
  Serial.begin(9600);
  sensors.begin();  //start sensor
  Serial.println("Started!");

  Serial.println();
  Serial.print("Connecting to ");
  Serial.print(wifiSsid);
  Serial.println("...");

  WiFi.begin(wifiSsid, wifiPwd);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(50);
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  
  printCurrentNetwork();
  printWiFiData();
  getSensorReadings();
  delay(2000);                      // Wait for two seconds (to demonstrate the active low LED)
  
  ESP.deepSleep(sleepTimeS * 1000000);
}  //end of setup

void loop() {
}  //end of main loop

void printWiFiData() {
  
  // IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
}

void printCurrentNetwork() {
  
  // SSID
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // signal strength:
  Serial.print("signal strength (RSSI): ");
  Serial.println(WiFi.RSSI());
}

void publish(const char *topic, uint8_t data1) {
  
  AmazonIOTClient iotClient;
  ActionError actionError;

  Esp8266HttpClient httpClient;
  Esp8266DateTimeProvider dateTimeProvider;



  Serial.println("Initializing IoT client...");

  iotClient.setAWSRegion(awsIotRegion);
  iotClient.setAWSEndpoint(awsIotEndpoint);
  iotClient.setAWSDomain(awsIotDomain);
  iotClient.setAWSPath("/things/soil_sensor_one/shadow");
  iotClient.setAWSKeyID(awsKeyID);
  iotClient.setAWSSecretKey(awsSecKey);
  iotClient.setHttpClient(&httpClient);
  iotClient.setDateTimeProvider(&dateTimeProvider);

  delay(50);

  Serial.println("Updating thing shadow...");
  
  MinimalString shadow = ("{\"state\":{\"reported\":{\"Temperature\":" + String(data1, DEC) + "}}}").c_str();
  char* result = iotClient.update_shadow(shadow, actionError);

  Serial.print("result: ");
  Serial.println(result);
} //end of publish to AWS IoT

void getSensorReadings() {

  Serial.println("Checking sensor readings ");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  Serial.print("Temperature for the device 1 (index 0) is: ");
  temp = sensors.getTempCByIndex(0);
  while (temp > 80) {
    delay (2000);
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
  }
  Serial.println(temp);  
  //post values to AWS IoT
  publish("soil/moisture", temp);
} //end of getSensorReadings

