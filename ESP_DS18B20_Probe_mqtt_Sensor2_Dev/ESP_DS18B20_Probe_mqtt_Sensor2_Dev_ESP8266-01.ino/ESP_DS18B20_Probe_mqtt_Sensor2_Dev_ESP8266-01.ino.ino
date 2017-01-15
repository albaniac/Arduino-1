/*
 * This sketch is the development version for an ESP8266-01 to 
 * sense temperature. The temperature
 * sensor is a DS18B20 one wire device.
 * Robin Harris 25th September 2016
 */

#include <ESP8266WiFi.h>  //wifi library
#include <PubSubClient.h> // mqtt library
#include <WiFiUdp.h> //udp library needed to get time
#include <OneWire.h>  // required for the communication with the DS18B20
#include <DallasTemperature.h> //handles the data from the DS18B20

//NTP time setup stuff
unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "0.uk.pool.ntp.org";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;
unsigned long currentUnixTime;
unsigned long previousUnixTime = 0UL;
//=================================================================================


// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2

WiFiClient espClient;
PubSubClient client(espClient);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//=================================================================================
//Prototype function declarations
void publish(const char *topic, unsigned long timestamp, int data1);
void getSensorReadings(); // read the sensors.  
void reconnect(); //mqtt reconnect
unsigned long sendNTPpacket(IPAddress& address);
void getTime();

//=================================================================================

// network and nqtt broker values
const char* ssid = "Kercem2";
const char* wifiPassword = "E0E3106433F4";
const char* mqtt_server = "52.25.138.129";
const char* mqttUser = "mqtt_user";
const char* mqttPassword = "pubsub";
const char* topicToPublish = {"ESP8266 Temp"}; //mqtt topic to publish to

char msg[70];//max message length
const int sleepTimeS = 900; //number of seconds to deep sleep


//=================================================================================

void setup() {
  client.setServer(mqtt_server, 1883);  //set up the mqtt server
  udp.begin(localPort);
  WiFi.begin(ssid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
     delay(200);
  }
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  getSensorReadings();
  delay(2000);
  ESP.deepSleep(sleepTimeS * 1000000);
}

//=================================================================================

void loop() {

//  if (!client.connected()) {
//  reconnect();
//  }
//  client.loop();
//
//  //for debugging this section is omitted
//  getSensorReadings();
//  delay(10000);
//
//  int counter = 1;
//  int data1 = 100;
//  for (int data2 = 0; data2 < 30; data2++) {
//    for (int data1 = 100; data1 < 400; data1+=50) {
//        getTime();
//        client.loop();
//        publish(topicToPublish, currentUnixTime, data1);
//        delay (2000);
//    }
//  }
// client.loop();
}

//=================================================================================

void getSensorReadings() {
 //get temperature from DS18B20
  int temp = 100;
  do {
    delay(1000);
    sensors.requestTemperatures(); // Send the command to get temperatures
    delay(20);
    // After we got the temperatures, we can print them here.
    // We use the function ByIndex, and as an example get the temperature from the first sensor only.
    temp = sensors.getTempCByIndex(0);
    } while (temp < -20 || temp > 84);
  getTime();
// publish
  publish(topicToPublish, currentUnixTime, temp);
} //end of getSensorReadings

//=================================================================================

void publish(const char *topic, unsigned long timestamp, int data1){
  
  // connect to the mqtt broker
  if (!client.connected()) {
    reconnect();
  }
    
  //construct the JSON string to send
  snprintf (msg, 70, "{\"Timestamp\": %ld,\"Temperature\": %d}",timestamp, data1);
  
  // send the message
  client.publish(topic, msg);

} //end of publish

//=================================================================================

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("ESP8266Client",mqttUser,mqttPassword)) {
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}  // end reconnect

//=================================================================================

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
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

//=================================================================================


void getTime(){
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
  if (previousUnixTime < 1) {
    previousUnixTime = currentUnixTime;  //first time round set previousUnixTime
  }
} while (currentUnixTime > previousUnixTime + 800000UL || currentUnixTime == previousUnixTime);
                                                        // invalid times return a very high date
                                                        // so check current time is not more than
                                                        // about one week ahead of previous time
} //end getTime
