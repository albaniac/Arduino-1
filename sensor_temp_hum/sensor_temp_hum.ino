/* This sketch reads temperature and humidity from a DHT11 sensor
 * Output is provided to a serial monitor and values are posted to an AWS IoT shadow
 * Robin Harris 19th July 2016
 */

#include <AmazonIOTClient.h>
#include <Esp8266AWSImplementations.h>
#include <AWSFoundationalTypes.h>
#include "keys.h"
#include "DHT.h"

#define DHTPIN 2  //connect DHT11 to GPIO2 on ESP8266-01
#define DHTTYPE DHT11   //using a DHT11
const int sleepTimeS = 30; //number of seconds to deep sleep

uint8_t temp = 0;      //integer to hold temperature from DHT11
uint8_t humidity = 0;  //integer to hold humidty from DHT11

//Initialise DHT11 sensor
DHT dht(DHTPIN, DHTTYPE, 15);

void printWiFiData();
void printCurrentNetwork();
void publish(const char *topic, uint8_t data1, uint8_t data2);
void getSensorReadings();

void setup() {
  Serial.begin(9600);
  // Pin configuration
  dht.begin();  //start dht object
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

  delay(50);
}  //end of setup

void loop() {
  getSensorReadings();
  delay(300000);
  //ESP.deepSleep(sleepTimeS * 1000000);
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

void publish(const char *topic, uint8_t data1, uint8_t data2) {
  
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
  
  MinimalString shadow = ("{\"state\":{\"reported\":{\"Temperature\":" + String(data1, DEC) + ",\"Humidity\":" + String(data2, DEC) + "}}}").c_str();
  char* result = iotClient.update_shadow(shadow, actionError);

  Serial.print("result: ");
  Serial.println(result);
} //end of publish to AWS IoT

void getSensorReadings() {

  Serial.println("Checking sensor readings ");
  delay(100);
  temp = dht.readTemperature();   //read temperature as degrees Celcius
  delay(100);
  humidity = dht.readHumidity();; //read humidity
  if (humidity==255 || temp==255) {
  Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.print(" C\tHumidity: ");
  Serial.print(humidity);
  Serial.println("%");
  
 // post values to AWS IoT
  publish("soil/moisture", temp, humidity);
} //end of getSensorReadings

