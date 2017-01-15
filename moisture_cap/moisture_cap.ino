/*
 * This sketch measures soil moisture using a pair of electrodes to create a
 * capacitor.  The capacitance is part of the frequency determining components
 * of a 555 astable multivibrator.  This code measures the duration of the
 * positive pulse which is directly related to the capacitance and hence soil
 * moisture
 * 4th January 2017
 */

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiUdp.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

//=======================================================================================
// initialise variables
const char* ssid = "workshop";
const char* wifiPassword = "workshop";
const char* mqtt_server = "192.168.0.36";
const char* user = "mqtt_user";
const char* mqttPassword = "pubsub";
char msg[70];
unsigned int localPort = 2390;      // local port to listen for UDP packets
const char* topicToPublish = {"moisture_cap"}; //mqtt topic to publish to
const uint8_t sensorPin = D6;  //Connect sensor to D6
uint8_t sensorEnable = D5;  //connect moisture sensor +ve supply to D5
unsigned long pulseDuration = 0; //integer to hold positive pulse width
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


//prototype function declarations
//========================================================================================
void publish(const char *topic, unsigned long data1);
void getSensorReadings();
unsigned long sendNTPpacket(IPAddress& address);
void getTime();


//========================================================================================

void setup() {
  Serial.begin(9600);
  Serial.println("Starting....");
  // Pin configuration
  pinMode(sensorEnable, OUTPUT);
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorEnable, LOW); //ensure sensor is off

  WiFi.begin(ssid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Port defaults to 8266
  //ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("moisture");

  // No authentication by default
  //ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {

  });
  ArduinoOTA.onEnd([]() {

  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {

  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  udp.begin(localPort);
  client.setServer(mqtt_server, 1883);
  WiFi.hostByName(ntpServerName, timeServerIP); 

}//end setup

void loop() {
    getTime();
    getSensorReadings(); //get the readings and publish them
    delay(15000);
}//end loop

//=========================================================
void publish(const char *topic, unsigned long data1) {
    
    // connect to the mqtt broker
    while (!client.connected()) {
    client.connect("ESP8266Client",user, mqttPassword);
    delay(5000); //wait then retry until connected
    }
    
    //construct the JSON string to send
    snprintf (msg, 70, "{\"Unix Time\": %ld,\"Moisture\": %ld}", currentUnixTime, data1);
    
    // send the message
    client.publish(topic, msg);

} //end of publish

//=========================================================

void getSensorReadings() {
 digitalWrite(sensorEnable, HIGH);
 delay(1000);
 pulseDuration = pulseIn(sensorPin, HIGH);
 digitalWrite(sensorEnable, LOW);
 Serial.print("Moisture: ");
 Serial.println(pulseDuration);
//send the values to the mqtt broker
 publish(topicToPublish, pulseDuration);
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

//===============================================================================================

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
