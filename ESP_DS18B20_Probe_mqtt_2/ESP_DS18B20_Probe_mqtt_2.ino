/* This sketch reads temperature DS18DS sensor
 * Values are posted to an EC2 Mosquitto Broker.  
 * TEMP holds the temp and is an integer so can hold negative numbers.
 * ESP8266 goes into deep sleep between readings.  Sleep time is set using sleepTimeS.
 * Robin Harris 1st September 2016
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid = "Kercem2";
const char* password = "E0E3106433F4";
const char* mqtt_server = "52.25.138.129";
const char* user = "mqtt_user");
const char* password = "pubsub");
char msg[50];

WiFiClient espClient;
PubSubClient client(espClient);

// Data wire is plugged into GPIO 2 on the ESP8266
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

const int sleepTimeS = 30; //number of seconds to deep sleep
int temp = 0;      //integer to hold temperature

void publish(const char *topic, int data1);
void getSensorReadings();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(9600);
  // Pin configuration
  sensors.begin();  //start sensor

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
  }
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  delay(10000); //wait for DS18B20 to be ready

  getSensorReadings(); //get the readings and publish them
  ESP.deepSleep(sleepTimeS * 1000000); //go to sleep
}  //end of setup

void loop() {
}  //end of main loop


void publish(const char *topic, int data1) {
    
    // connect to the mqtt broker
    while (!client.connected()) {
    client.connect("ESP8266Client",user, password);
    delay(5000); //wait then retry until connected
    }
    
    //construct the JSON string to send
    snprintf (msg, 50, "{\"Temperature\": %d :}", data1);
    
    // send the message
    client.publish("Temperature", msg);

} //end of publish

void getSensorReadings() {
  sensors.requestTemperatures(); // Send the command to get temperatures

  // Use the function ByIndex - 0 is the first device
  temp = sensors.getTempCByIndex(0);
  //send the values to the mqtt broker
  publish("temperature", temp);
} //end of getSensorReadings

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print(" ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}



