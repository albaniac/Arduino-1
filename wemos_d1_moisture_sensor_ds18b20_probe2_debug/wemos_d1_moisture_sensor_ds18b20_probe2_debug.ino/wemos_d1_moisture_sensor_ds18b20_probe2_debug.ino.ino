/*
 * This sketch is the debug version (with Serial output) for a Wemos D1 to 
 * sense soil moisture and temperature. The sensors are turned on and off as 
 * required to reduce electrolytic decay and power consumption.  The temperature
 * sensor is a DS18B20 one wire device.
 * If the moisture level is below a threshold D5 is used to turn on a pump for 
 * a set interval of time
 * Robin Harris 14th September 2016
 */

 
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>  // required for the communication with the DS18B20
#include <DallasTemperature.h> //handles the data from the DS18B20
#include <WiFiUdp.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS D2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void publish(const char *topic, int data1, int data2);
void getSensorReadings(); // read the sensors.  Calls publish to upload to AWS
void addwater();  // run the pump to water the plant

const char* ssid = "Kercem2";
const char* password = "E0E3106433F4";
const char* mqtt_server = "52.25.138.129";
const char* user = "mqtt_user";
const char* mqttPassword = "pubsub";
const char* topicToPublish = {"Temperature"}; //mqtt topic to publish to
unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "0.uk.pool.ntp.org";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

WiFiClient espClient;
PubSubClient client(espClient);

char msg[70];
int temp = 78;
uint8_t moisturePin = A0;  //Connect sensor to A0 - the only analogue input
uint8_t sensorEnable = D1;  //connect moisture sensor +ve supply to D1
uint8_t pumpPin = D5; // pin that controls the pump
const int sleepTimeS = 5; //number of seconds to deep sleep
uint8_t pumpRunS = 5;  //number of seconds the pump will run for 
unsigned long currentUnixTime;
unsigned long previousUnixTime = 0UL;

void publish(const char *topic, int data1);
void getSensorReadings();
unsigned long sendNTPpacket(IPAddress& address);



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", user, mqttPassword)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}//end of reconnect


void setup() {

  Serial.begin(9600);
// Pin configuration
  pinMode(sensorEnable, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);  //make sure the pump is OFF

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
     Serial.print(".");
     delay(50);
  }
  Serial.print("WiFi Connected");
  client.setServer(mqtt_server, 1883);

  WiFi.hostByName(ntpServerName, timeServerIP); 

  do {
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  
  int cb = udp.parsePacket();
  // We've received a packet, read the data from it
  udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

  //the timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, esxtract the two words:

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  // now convert NTP time into everyday time:
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  currentUnixTime = secsSince1900 - seventyYears;
  Serial.print("Unix time: ");
  Serial.println(currentUnixTime);
  if (previousUnixTime < 1) {
    previousUnixTime = currentUnixTime;  //first time round set previousUnixTime
  }
} while (currentUnixTime > previousUnixTime + 800000UL || currentUnixTime == previousUnixTime);
                                                        // invalid times return a very high date
                                                        // so check current time is not more than
                                                        // about one week ahead of previous time
  // getSensorReadings();
  //ESP.deepSleep(sleepTimeS * 1000000);

}//end setup

void loop() {
   if (!client.connected()) {
    reconnect();
  }
  delay(5000);
  publish(topicToPublish, 56);
}// end of loop

//=========================================================
void publish(const char *topic, int data1) {
    
    // connect to the mqtt broker
    // Loop until we're reconnected
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect("Probe2", user, mqttPassword)) {
        Serial.println("connected");
        // Once connected, publish an announcement...
        snprintf (msg, 70, "{\"Unix Time\": %ld,\"Temperature\": %d}", topic, data1);
        // send the message
        Serial.println(topic);
        Serial.println(msg);
        client.loop();
      client.publish(topic, msg);
      }
      else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
    
    Serial.println("sent message");

} //end of publish

//=========================================================

void getSensorReadings() {
  sensors.requestTemperatures(); // Send the command to get temperatures

  // Use the function ByIndex - 0 is the first device
  //temp = sensors.getTempCByIndex(0);
  Serial.println(temp);
  //send the values to the mqtt broker
;
  publish(topicToPublish, temp);
} //end of getSensorReadings

//=========================================================

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
} //end sendNTPpacket

//=========================================================



