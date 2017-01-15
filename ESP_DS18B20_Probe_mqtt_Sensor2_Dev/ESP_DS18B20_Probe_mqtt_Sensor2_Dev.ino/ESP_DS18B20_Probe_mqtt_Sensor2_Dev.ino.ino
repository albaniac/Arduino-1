/*
 * This sketch is the development version for a Wemos D1 to 
 * sense soil moisture and temperature. The sensors are turned on and off as 
 * required to reduce electrolytic decay and power consumption.  The temperature
 * sensor is a DS18B20 one wire device.
 * If the moisture level is below a threshold D5 is used to turn on a pump for 
 * a set interval of time.
 * mDNS is used and a server is used to provide local viewing of temperature and humidity.
 * The pump can also be turned on and off from the web page /server
 * Robin Harris 6th November 2016
 */

#include <ESP8266WiFi.h>  //wifi library
#include <WiFiClient.h>
#include <PubSubClient.h> // mqtt library
#include <WiFiUdp.h> //udp library needed to get time
#include <OneWire.h>  // required for the communication with the DS18B20
#include <DallasTemperature.h> //handles the data from the DS18B20
#include <ESP8266WebServer.h>//server library
#include <ESP8266mDNS.h>//mDNS library

MDNSResponder mdns; //create mDNS object
ESP8266WebServer server(80);//create a server on port 80

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
unsigned long nextMillis=0;
//=================================================================================


// Data wire is plugged into port D6 on the Arduino
#define ONE_WIRE_BUS D6

WiFiClient espClient;
PubSubClient client(espClient);

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);//DS18B20 object

//=================================================================================
//Prototype function declarations
void publish(const char *topic, unsigned long timestamp, int data1, int data2);
void getSensorReadings(); // read the sensors.  
void addwater();  // run the pump to water the plant
void reconnect(); //mqtt reconnect
unsigned long sendNTPpacket(IPAddress& address);
void getTime();

//=================================================================================

// network and mqtt broker values
const char* ssid = "Kercem2";
const char* wifiPassword = "E0E3106433F4";
const char* mqtt_server = "52.25.138.129";
const char* mqttUser = "mqtt_user";
const char* mqttPassword = "pubsub";
const char* topicToPublish = {"debug"}; //mqtt topic to publish to
//=================================================================================


char msg[70];//max message length
String webPagePumpOn = "";//final page is made up of a static part and  built part
String webPagePumpOff = "";//final page is made up of a static part and  built part
String webPageStatic = "<h1>Robin's Plant Server</h1>";//static part of page
char buildWebPage[200];//used to build the webpage string

//=================================================================================
//pin assignments
uint8_t moisturePin = A0;  //Connect sensor to A0 - the only analogue input
uint8_t sensorEnable = D8;  //connect moisture sensor +ve supply to D8
uint8_t pumpPin = D5; // pin that controls the pump
uint8_t pumpRunS = 5;  //number of seconds the pump will run for 

//=================================================================================

void setup() {
  Serial.begin(9600);
  webPageStatic += "<p><a href=\"PUMPON\"><button style=\"font-size:30;width:150;height:100;background-color:red\">ON</button></a>&nbsp;<a href=\"PUMPOFF\"><button style=\"font-size:30;width:150;height:100;background-color:green\">OFF</button></a></p>";
  client.setServer(mqtt_server, 1883);  //set up the mqtt server
  udp.begin(localPort);
  WiFi.begin(ssid, wifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
     Serial.print(".");
     delay(200);
  }
  Serial.print("WiFi Connected   ");
  Serial.println(WiFi.localIP());

//Start mDNS responder
  if (mdns.begin("plant", WiFi.localIP())) {
  Serial.println("MDNS responder started");
  }
  sensors.begin();//start DS18B20 on oneWire
// Pin configuration
  pinMode(sensorEnable, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin, LOW);  //make sure the pump is OFF
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

//=================================================================================
  //setup server
  server.on("/", [](){
  server.send(200, "text/html", webPagePumpOff);
  });
  server.on("/PUMPON", [](){
  Serial.println("Pump on");
  server.send(200, "text/html", webPagePumpOn);
  addwater();
  });
  server.on("/PUMPOFF", [](){
  digitalWrite(pumpPin, LOW);
  Serial.println("Pump off");
  server.send(200, "text/html", webPagePumpOff);
  });
  server.begin();
  Serial.println("HTTP server started");

//=================================================================================
 

}//end of setup

//=================================================================================

void loop() {
  yield();
  server.handleClient();
  yield();
  if (!client.connected()) {
  reconnect();
  }
  client.loop();
  yield();
  if (millis() > nextMillis)
  {
      getSensorReadings();
      nextMillis = millis()+180000;//360000 millis = 1 hour - time between sensor readings
      getTime();
  }

}//end main loop

//=================================================================================

void getSensorReadings() {
  //turn on sensors
  digitalWrite(sensorEnable, HIGH);
  int temp = 100;
  int sensorValue = 0;
  delay(2000);

 //get temperature from DS18B20
 do { //loop until an in range temperature value is read
    sensors.requestTemperatures(); // Send the command to get temperatures
    delay(20);
    // After we got the temperatures, we can print them here.
    // We use the function ByIndex, and as an example get the temperature from the first sensor only.
    temp = sensors.getTempCByIndex(0);
    Serial.print("Temp: ");
    Serial.println(temp);  
  } while (temp < -20 || temp > 84); //check for in range temperature value

  //get sensor reading for moisture   
  sensorValue = analogRead(moisturePin);

  // turn off sensors
  digitalWrite(sensorEnable, LOW);

// create the variable part of the web page
  snprintf (buildWebPage, 200, "<p><font size=4 color = green>Humidity: %d, &nbsp Temperature: %d</font></p>",sensorValue, temp);
  webPagePumpOn=webPageStatic+buildWebPage+"<p><font size=5 color = \"red\">Pump ON</font></p>";
  webPagePumpOff=webPageStatic+buildWebPage+"<p><font size=5 color = \"green\">Pump OFF</font></p>";

  Serial.print ("sensorValue   ");
  Serial.println(sensorValue);
  // check if moisture level is low enough to need water adding
  if (sensorValue < 300 ) {
    addwater();
  }
  
// publish
  publish(topicToPublish, currentUnixTime, sensorValue, temp);
} //end of getSensorReadings

//=================================================================================

void addwater(){
  digitalWrite(pumpPin, HIGH);
  delay(pumpRunS *1000);
  digitalWrite(pumpPin, LOW);
}// end addwater

//=================================================================================

void publish(const char *topic, unsigned long timestamp, int data1, int data2){
  
  // connect to the mqtt broker
  if (!client.connected()) {
    reconnect();
  }
    
  //construct the string to send
  snprintf (msg, 70, "Timestamp: %ld,Humidity: %d,Temperature: %d",timestamp, data1, data2);
  
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
      // Wait 2 seconds before retrying
      delay(2000);
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
  Serial.print("currentUnixTime: ");
  Serial.println(currentUnixTime);
} while (currentUnixTime > previousUnixTime + 800000UL || currentUnixTime == previousUnixTime);
                                                        // invalid times return a very high date
                                                        // so check current time is not more than
                                                        // about one week ahead of previous time
} //end getTime
