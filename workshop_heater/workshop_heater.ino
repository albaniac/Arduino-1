  /*
Workshop heating controller.
Serves an HTML page which shows temperature and allows the heater to be turned on and off.
There is a minimum temperature below which the heater is turned on automatically.
Robin Harris  21st November 2016
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <OneWire.h>  // required for the communication with the DS18B20
#include <DallasTemperature.h> //handles the data from the DS18B20

const char *ssid = "workshop";
const char *password = "workshop";

ESP8266WebServer server(80);

#define ONE_WIRE_BUS 2 // GPIO2 for DS18B20

// Setup a oneWire instance
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);//DS18B20 object

#define relay 0 //relay on GPIO0
char page[800];//the html page to send
int temperature = 100;// holds the current temperature
boolean heater_status = FALSE;//tells us whether the heater is on or off
unsigned long nextMillis=0;

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
      Serial.println(text);
    }
    else {
      text = "OFF";
      Serial.println(text);
    }
    
    snprintf (page, 400,

"<html>\
  <head>\
    <title>Workshop Temperature Controller</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Workshop Temperature Controller</h1>\
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
  Serial.println(page);
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

	if (MDNS.begin ( "workshop") ) {
		Serial.println ("MDNS responder started" );
	}

//=================================================================================
  //setup server
  server.on ("/", handleRoot );

  //Deal with turing heater on
  server.on("/HEATER_ON", [](){
  digitalWrite(relay, HIGH);
  heater_status = TRUE;   
  buildPage(heater_status);
  server.send(200, "text/html", page);
  });

//deal with turning heater off
  server.on("/HEATER_OFF", [](){
  digitalWrite(relay, LOW);
  heater_status = FALSE;
  buildPage(heater_status);
  server.send(200, "text/html", page);
  });

  server.onNotFound (handleNotFound);
  server.begin();
  Serial.println ("HTTP server started");

//=================================================================================
}//end of setup

void loop() {
	server.handleClient();
  if (millis() > nextMillis)
  {
    do { //loop until an in range temperature value is read
        sensors.requestTemperatures(); // Send the command to get temperatures
        delay(20);
        // After we got the temperatures, we can print them here.
        // We use the function ByIndex, and as an example get the temperature from the first sensor only.
        temperature = sensors.getTempCByIndex(0);
        Serial.print("Temp: ");
        Serial.println(temperature);
        delay(2000); 
    } while (temperature < -20 || temperature > 84); //check for in range temperature value
    nextMillis = millis()+60000;//60000 millis = 1 minutes - time between sensor readings
  }

}//end of main loop

