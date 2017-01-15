/*
 * This sketch uses the Wemos D1 with a soil moisture sensor amd a DS18B20
 * temperature sensor. It has a pump to add water.
 * The sensor is turned on and off as required to reduce electrolytic decay 
 * D3 is used to turn on a pump if the moisture level is below a threshold
 * No serial debug information is provided.
 * 
 * Robin Harris 10th September 2016
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiUdp.h>

//=======================================================================================
// initialise variables
const char* ssid = "Kercem2";
const char* wifiPassword = "E0E3106433F4";
const char* mqtt_server = "52.25.138.129";
const char* user = "mqtt_user";
const char* mqttPassword = "pubsub";
char msg[70];
unsigned int localPort = 2390;      // local port to listen for UDP packets
const char* topicToPublish = {"Sensor2"}; //mqtt topic to publish to
uint8_t sensor = A0;  //Connect sensor to A0 - the only analogue input
uint8_t sensorEnable = 0;  //connect moisture sensor +ve supply to D1
uint8_t pumpPin = 6; // pin that controls the pump
uint8_t pumpRunS = 5;  //number of seconds the pump will run for 
const int sleepTimeS = 600; //number of seconds to deep sleep
int temp = 0;      //integer to hold temperature
int sensorValue = 0; //integer to hold moisture sensor value
unsigned long currentUnixTime;
unsigned long previousUnixTime = 0UL;
//========================================================================================

IPAddress timeServerIP; // time.nist.gov NTP server address
const char* ntpServerName = "0.uk.pool.ntp.org";

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

WiFiClient espClient;
PubSubClient client(espClient);

// Data wire is plugged into GPIO 2 on the ESP8266
#define ONE_WIRE_BUS 5

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//prototype function declarations
//========================================================================================
void publish(const char *topic, int data1,int data2);
void getSensorReadings();
unsigned long sendNTPpacket(IPAddress& address);
void getTime();

//========================================================================================
void setup() {
  Serial.begin(9600);
  Serial.println("Starting....");
  // Pin configuration
  sensors.begin();  //start sensor

  // Pin configuration
  pinMode(sensorEnable, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);  //make sure the pump is OFF
  
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

  getTime();
  getSensorReadings(); //get the readings and publish them
  ESP.deepSleep(sleepTimeS * 1000000); //go to sleep
}  //end of setup

//=========================================================

void loop() {
}  //end of main loop

//=========================================================
void publish(const char *topic, int data1, int data2) {
    
    // connect to the mqtt broker
    while (!client.connected()) {
    client.connect("ESP8266Client",user, mqttPassword);
    delay(5000); //wait then retry until connected
    }
    
    //construct the JSON string to send
    snprintf (msg, 70, "{\"Unix Time\": %ld,\"Temperature\": %d,\"Moisture\": %d}", currentUnixTime, data1,data2);
    
    // send the message
    client.publish(topic, msg);

} //end of publish

//=========================================================

void getSensorReadings() {
  digitalWrite(sensorEnable, HIGH);
  delay(10000);
  sensors.requestTemperatures(); // Send the command to get temperatures

  // Use the function ByIndex - 0 is the first device
  temp = sensors.getTempCByIndex(0);
  sensorValue = analogRead(sensor);
  digitalWrite(sensorEnable, LOW);
 Serial.print("Temperature: ");
 Serial.println(temp);
 Serial.print("Moisture: ");
 Serial.println(sensorValue);
  if (sensorValue < 300 ) {
    addwater();
  }
  //send the values to the mqtt broker
;
  publish(topicToPublish, temp, sensorValue);
} //end of getSensorReadings

//=========================================================

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

//=========================================================
//===============================================================================================
void addwater(){
  digitalWrite(pumpPin, HIGH);
  delay(pumpRunS *1000);
  digitalWrite(pumpPin, LOW);
}

//===============================================================================================
//Function to get NTP time with some simple checking for invalid times.
//Variables are global so no parameters passed or returned.

void getTime(){
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
  Serial.print ("Unix time: ");
  Serial.print (currentUnixTime);
  if (previousUnixTime < 1) {
    previousUnixTime = currentUnixTime;  //first time round set previousUnixTime
  }
} while (currentUnixTime > previousUnixTime + 800000UL || currentUnixTime == previousUnixTime);
                                                        // invalid times return a very high date
                                                        // so check current time is not more than
                                                        // about one week ahead of previous time
                                                         
}//end of getTime
