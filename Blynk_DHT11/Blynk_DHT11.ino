/**************************************************************
This is a test sketch to evaluate Blynk.  
 **************************************************************/

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "DHT.h"
//#include <SimpleTimer.h>

SimpleTimer timer;

//prototype function declarations
void readDHT11();

//define virtual pins for Blynk
#define PIN_TEMP V4
#define PIN_HUMIDITY V5
#define PIN_HEATINDEX V3

//define DHT11 parameters
#define DHTPIN D1     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

char auth[] = "731f1a3c123a406aacf45de3fd0ca9e7";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Kercem2";
char pass[] = "E0E3106433F4";
float temp = 0.0;
float humidity = 0.0;
float heatIndex = 0.0;
long startMillis = 0;

void myTimerEvent()
{
  // You can send any value at any time.
  Blynk.virtualWrite(V4, temp);
  Blynk.virtualWrite(V5, humidity);
}

void setup()
{
  Blynk.begin(auth, ssid, pass,IPAddress(52,25,138,129));
  dht.begin();
  readDHT11();
  // Setup a function to be called every minute
  timer.setInterval(60000L, myTimerEvent);
}//end setup

void loop(){
  Blynk.run();
  timer.run(); // Initiates SimpleTimer
  readDHT11();
}// end loop

void readDHT11(){
  do{ 
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    // Read humidity
    humidity = dht.readHumidity();
    // Read temperature as Celsius (the default)
    temp = dht.readTemperature();
    // Compute heat index in Celsius (isFahreheit = false)
    heatIndex = dht.computeHeatIndex(temp, humidity, false);
   } while (isnan(humidity) || isnan(temp));
}//end readDHT11

//BLYNK_READ(PIN_TEMP){
//  // This command writes Arduino's uptime in seconds to Virtual Pin (5)
//  Blynk.virtualWrite(PIN_TEMP,temp);
//}
//
//BLYNK_READ(PIN_HUMIDITY){
//  // This command writes Arduino's uptime in seconds to Virtual Pin (5)
//  Blynk.virtualWrite(PIN_HUMIDITY,humidity);
//}
//
//BLYNK_READ(PIN_HEATINDEX){
//  // This command writes Arduino's uptime in seconds to Virtual Pin (5)
//  Blynk.virtualWrite(PIN_HEATINDEX,heatIndex);
//}

