  /*
Workshop heating controller.
Serves an HTML page which shows temperature and allows the heater to be turned on and off.
There is a minimum temperature below which the heater is turned on automatically.
Robin Harris  21st November 2016
Modified on 13th December for a Wemos D1.  Uses Ticker to generate 1 second
count to increment watchdog.  Also a 60 second counter for updating the
temperature.
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <OneWire.h>  // required for the communication with the DS18B20
#include <DallasTemperature.h> //handles the data from the DS18B20
#include <Ticker.h>

const char *ssid = "workshop";
const char *password = "workshop";

ESP8266WebServer server(80); //create a webserver object 'server'

Ticker secondTick; //create a Ticker object 'secondTick'
Ticker minuteTick; //create a Ticker object 'minuteTick'


#define ONE_WIRE_BUS D7 // pin 7 for DS18B20

// Setup a oneWire instance
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);//DS18B20 object

#define relay D5 //relay on pin 5
char page[600];//the html page to send
int temperature = 100;// holds the current temperature
boolean heater_status = FALSE;//tells us whether the heater is on or off
boolean manual_control = FALSE;// used to track if the heater was turned on manually
int watchdogCount = 0;

void ISRwatchdog(){
  watchdogCount ++;
  if (watchdogCount == 5) {
    ESP.reset();
  }
}

void ISRgetTemperature(){
  sensors.requestTemperatures(); // Send the command to get temperatures
  delay(20);
  temperature = sensors.getTempCByIndex(0);
  if (temperature < -20 || temperature > 84){ //check for in range temperature value
    temperature = 20;
  }
}

void handleRoot() {
	buildPage(heater_status);
	server.send (200, "text/html",page);
}//end handleRoot

void handleNotFound() {
	String message = "File Not Found\n\n";
	message += "URI: ";
	message += server.uri();
	message += "\nMethod: ";
	message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
	message += "\nArguments: ";
	message += server.args();
	message += "\n";

	for ( uint8_t i = 0; i < server.args(); i++ ) {
		message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
	}

	server.send ( 404, "text/plain", message );
}//end handleNotFound

void buildPage(boolean on_off) {
    const char* text = "";
    if (on_off) {
      text = "ON ";
    }
    else {
      text = "OFF";
    }
    
    snprintf (page, 600,

"<html>\
  <head>\
    <title>Workshop Temperature Controller Test</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Workshop Temperature Controller Test</h1>\
    <p>Temperature: %02d</p>\
    <p>Heater status: ",
    temperature
  );
  strcat(page, text);
  strcat(page, "<p>\
  <a href=\"HEATER_ON\"><button style=\"font-size:30;width:150;height:100;\
  background-color:red\">ON</button></a>\
  <a href=\"HEATER_OFF\"><button style=\"font-size:30;width:150;height:100;\
  background-color:green\">OFF</button></a>\
  </p></body></html>");
}//end buildPage


void setup () {
	pinMode (relay, OUTPUT);
	digitalWrite (relay, LOW); //make sure relay is off
  sensors.begin();
	Serial.begin (9600);
	WiFi.begin (ssid, password);
	Serial.println ("");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED ) {
		delay (500);
		Serial.print (".");
	}

	Serial.println ("");
	Serial.print ("Connected to ");
	Serial.println (ssid);
	Serial.print ("IP address: ");
	Serial.println (WiFi.localIP());

	if (MDNS.begin ( "workshop2") ) {
		Serial.println ("MDNS responder started" );
	}

//=================================================================================
  //setup server
  server.on ("/", handleRoot );

  //Deal with turning heater on
  server.on("/HEATER_ON", [](){
  digitalWrite(relay, HIGH);
  heater_status = TRUE;
  manual_control = TRUE;
  buildPage(heater_status);
  server.send(200, "text/html", page);
  });

//deal with turning heater off
  server.on("/HEATER_OFF", [](){
  digitalWrite(relay, LOW);
  heater_status = FALSE;
  manual_control = FALSE;
  buildPage(heater_status);
  server.send(200, "text/html", page);
  });

//deal with page not found
  server.onNotFound (handleNotFound);
  
  server.begin();
  Serial.println ("HTTP server started");

//start the watchDog timer
  secondTick.attach(1, ISRwatchdog);

  ISRgetTemperature(); //get the temperature 
//start the getTemperature timer to update every minute
  minuteTick.attach(60, ISRgetTemperature);

//=================================================================================
}//end of setup

void loop() {
	server.handleClient();
  watchdogCount = 0;
  if (temperature < 10  && heater_status == FALSE && manual_control == FALSE) {
    heater_status = TRUE;
    digitalWrite(relay, HIGH);
  }
  if (temperature >= 12 && heater_status == TRUE && manual_control == FALSE) {
    heater_status = FALSE;
    digitalWrite(relay, LOW);
  }
  //turn heater off if temperature reaches the upper set point
  if (temperature > 22){
    heater_status = FALSE;
    digitalWrite(relay, LOW);
  }

}//end of main loop

