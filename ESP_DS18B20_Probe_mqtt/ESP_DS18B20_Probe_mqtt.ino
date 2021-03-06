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
#include <WiFiUdp.h>


const char* ssid = "Kercem2";
const char* wifiPassword = "E0E3106433F4";
const char* mqtt_server = "52.25.138.129";
const char* user = "mqtt_user";
const char* mqttPassword = "pubsub";
char msg[70];
unsigned int localPort = 2390;      // local port to listen for UDP packets
const char* topicToPublish = {"Temperature"}; //mqtt topic to publish to

IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "0.uk.pool.ntp.org";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

WiFiClient espClient;
PubSubClient client(espClient);

// Data wire is plugged into GPIO 2 on the ESP8266
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

const int sleepTimeS = 600; //number of seconds to deep sleep
int temp = 0;      //integer to hold temperature
unsigned long currentUnixTime;
unsigned long previousUnixTime = 0UL;

void publish(const char *topic, int data1);
void getSensorReadings();
unsigned long sendNTPpacket(IPAddress& address);

void setup() {
  Serial.begin(9600);
  // Pin configuration
  sensors.begin();  //start sensor

  WiFi.begin(ssid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("Connected");
  udp.begin(localPort);
  client.setServer(mqtt_server, 1883);
  delay(5000); //wait for DS18B20 to be ready

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
  getSensorReadings(); //get the readings and publish them
  ESP.deepSleep(sleepTimeS * 1000000); //go to sleep
}  //end of setup

//=========================================================

void loop() {
}  //end of main loop

//=========================================================
void publish(const char *topic, int data1) {
    
    // connect to the mqtt broker
    while (!client.connected()) {
    client.connect("ESP8266Client",user, mqttPassword);
    delay(5000); //wait then retry until connected
    }
    
    //construct the JSON string to send
    snprintf (msg, 70, "{\"Unix Time\": %ld,\"Temperature\": %d}", currentUnixTime, data1);
    
    // send the message
    client.publish(topic, msg);

} //end of publish

//=========================================================

void getSensorReadings() {
  sensors.requestTemperatures(); // Send the command to get temperatures

  // Use the function ByIndex - 0 is the first device
  temp = sensors.getTempCByIndex(0);
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
