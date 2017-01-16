//7 segment shack clock with two thermometers.

/* Pin assignments:

5  Time bus
6  Time bus
7  Time bus
8  One wire bus
1  Temp bus
2  Temp bus
3  Temp bus
0  Led Colon
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <NTPtimeESP.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include <ArduinoOTA.h>
#include <WiFiUdp.h>

const char *ssid = "workshop";
const char *password = "workshop";
uint8_t hr = 19;
uint8_t mn = 54;
volatile uint8_t sec = 0;
uint8_t oldhr = 0;

Ticker secondTick; //create a Ticker object to count seconds

//===================================================================================
//Prototype function definitions

void updateTime();
void ISRclockTick ();
void timeDisplay();
//===================================================================================

NTPtime NTPuk("0.pool.ntp.org"); // UK NTP pool
/*
 * The structure contains following fields:
 * struct strDateTime
{
  byte hour;
  byte minute;
  byte second;
  int year;
  byte month;
  byte day;
  byte dayofWeek;
  boolean valid;
};
 */
strDateTime dateTime;

//set up temperature sensors:
// Data wire is plugged into D8 on the Wemos for the temperature sensor
#define ONE_WIRE_BUS D6

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//two DS18B20 sensors used:
DeviceAddress insideThermometer = {0x28, 0x7F, 0x15, 0x9D, 0x04, 0x00, 0x00, 0xEE};
DeviceAddress outsideThermometer = {0x28, 0x11, 0x54, 0x3B, 0x05, 0x00, 0x00, 0x11};
DeviceAddress testThermometer = {0x28, 0x39, 0x19, 0x08, 0x00, 0x00, 0x80, 0x7A};

//define pattern to send for selecting digits
byte digit_pattern [8] =
              {
              B00000001,  // 1
              B00000010,  // 2
              B00000100,  // 3
              B00001000,  // 4
              B00010000,  // 5
              B00100000,  // 6
              B01000000,  // 7
              B10000000   // 8
              };
// define segment patterns
byte segment_pattern [23]=
              {
              B01110111,  // 0
              B01000001,  // 1
              B01101110,  // 2
              B01101011,  // 3
              B01011001,  // 4
              B00111011,  // 5
              B00011111,  // 6
              B01100001,  // 7
              B01111111,  // 8
              B01111001,  // 9
              B11110111,  // 0.
              B11000001,  // 1.
              B11101110,  // 2.
              B11101011,  // 3.
              B11011001,  // 4.
              B10111011,  // 5.
              B10011111,  // 6.
              B11100001,  // 7.
              B11111111,  // 8.
              B11111001,  // 9.
              B00110110,  // C
              B00001000,  // minus
              B00000000   // blank
              };

              
int tens_hour = 0;
int ones_hour = 0;
int tens_minute = 0;
int ones_minute = 0;
//Pin connected to ST_CP of 74HC595 time
#define latchPinTime D5
//Pin connected to SH_CP of 74HC595 time
#define clockPinTime D1
////Pin connected to DS of 74HC595 time
#define dataPinTime D7
//Pin connected to ST_CP of 74HC595 temp
#define latchPinTemp D8
//Pin connected to SH_CP of 74HC595 temp
#define clockPinTemp D2
////Pin connected to DS of 74HC595 temp
#define dataPinTemp D4
//Pin connected to the leds used to make the colon display
#define ledPin D3
// used to hold the inside temperature from the sensor
float inside_temp= 0;
// used to hold the outside temperature from the sensor
float outside_temp = 0;
//used to hold the integer value of temperature  x 100 - inside
int inside_int_temp = 0;
//used to hold the integer value of temperature  x 100 - outside
int outside_int_temp = 0;
// initialise an array to hold the character to display in each of the 8 digits
int output [8];
//used to set the led colon display on or off
boolean ledState = false;
// used to derive a temperature check every minute
int minuteCounter = 0;


void setup() {
  WiFi.mode(WIFI_STA);
  WiFi.begin (ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  sensors.begin();
  Serial.begin(9600);
  // set the resolution to 10 bit is 0.25 degree and 9 bit 0.5 degree.  
  sensors.setResolution(insideThermometer, 10);
  sensors.setResolution(outsideThermometer, 10);
  sensors.setWaitForConversion (FALSE);
  sensors.requestTemperatures();
  inside_temp = sensors.getTempC(testThermometer);
  outside_temp = sensors.getTempC(testThermometer);
  Serial.println(inside_temp);
  //set pins to output so you can control the shift register
  pinMode(latchPinTime, OUTPUT);
  pinMode(clockPinTime, OUTPUT);
  pinMode(dataPinTime, OUTPUT);
  pinMode(latchPinTemp, OUTPUT);
  pinMode(clockPinTemp, OUTPUT);
  pinMode(dataPinTemp, OUTPUT);
  pinMode(ledPin, OUTPUT);

  secondTick.attach(1, ISRclockTick); // main clock tick

  //get the time
  // first parameter: Time zone in floating point (for India); second parameter: 1 for European summer time; 2 for US daylight saving time (not implemented yet)
  dateTime = NTPuk.getNTPtime(0.0, 1);
  hr = dateTime.hour;
  mn = dateTime.minute;
  sec = dateTime.second;
//  timeDisplay();
  MDNS.begin ( "workshop_clock");

//=======================================================================================
//Set up OTA

// Hostname defaults to esp8266-[ChipID]
ArduinoOTA.setHostname("workshop_clock");

// No authentication by default
//ArduinoOTA.setPassword((const char *)"123");

ArduinoOTA.onStart([]() {
  noInterrupts();
});
ArduinoOTA.onEnd([]() {
  interrupts();
});

ArduinoOTA.onError([](ota_error_t error) {
  ESP.reset();
});
ArduinoOTA.begin();

//=================================================================================   
  
}//end setup

void loop() {
    ArduinoOTA.handle();
    //refresh the temperature display as often as possible
    for (int i = 0; i < 8; i++){
      // take the latchPin low so 
      // the LEDs don't change while you're sending in bits:
      digitalWrite(latchPinTemp, LOW);
      //shift out the byte patterns one digit at a time 1 to 8
      shiftOut(dataPinTemp,clockPinTemp, MSBFIRST,digit_pattern[(i % 8)]);
      shiftOut(dataPinTemp,clockPinTemp, MSBFIRST,segment_pattern[output[i % 8]]);
      // take the latchPin high to light the display
      digitalWrite (latchPinTemp, HIGH);
      //delay(2);
      //turn off the last digit so it is not over bright
      digitalWrite (latchPinTemp, LOW);
      shiftOut (dataPinTemp, clockPinTemp, MSBFIRST, digit_pattern[7]);
      shiftOut (dataPinTemp, clockPinTemp, MSBFIRST, segment_pattern[22]);
      digitalWrite (latchPinTemp, HIGH);
    }//end of refreshing temperature display

     updateTime(); // increment sec, mn and hr
    
    if (oldhr < hr) {//reset NTP time each hour
      dateTime = NTPuk.getNTPtime(0.0, 1);
      hr = dateTime.hour;
      mn = dateTime.minute;
      sec = dateTime.second;
      oldhr = hr;
    }//end of resetting time to NTP
 
    //every minute read the temperature and update the clock display
    if (mn != minuteCounter){
       minuteCounter = mn;
       
//update the temperatures
       sensors.requestTemperatures();
       inside_temp = sensors.getTempC(testThermometer);
       outside_temp = sensors.getTempC(testThermometer);
       Serial.print("test thermometer: ");
       Serial.println(inside_temp);
       if ( inside_temp > 99.9 ) {
          inside_temp = 99.9;  
        }
        if ( outside_temp < -9.9 ) {
          outside_temp = -9.9;
        }
        inside_int_temp = int (inside_temp*10);
        outside_int_temp = int (outside_temp*10);
        // check if temperature is negative and if it is make the first character a
        // minus sign and change the value of integer_temperature to a positive value
        if (inside_int_temp < 0 ) {
          output [0] = 21; inside_int_temp = -1* inside_int_temp;
        }
        // if it is not negative derive the tens 
        else {
          output [0] = inside_int_temp / 100;
        }
        if ( output [0] == 0 ) {
          output[0] = 22;
        }
        if (outside_int_temp < 0 ) {
          output [4] = 21; outside_int_temp = -1* outside_int_temp;
        }
        else {
          output [4] = outside_int_temp / 100;
        }
        if ( output [4] == 0 ) {
          output [4] = 22;
        }
        // derive the ones
        output [1] = ((inside_int_temp / 10) % 10) +10;
        output [5] = ((outside_int_temp / 10) % 10) +10;
        // derive the tenths
        output [2] = inside_int_temp % 10;
        output [6] = outside_int_temp % 10;
        // the last digit is always a "C"
        output [3] = 20;
        output [7] = 20;

       //update the time display
        timeDisplay(); 
    }// end of dealing with a new minute
    //separate the two digits of the hour
    tens_hour = int (hr/10);
    ones_hour = hr % 10;
    //separate the two digits of the minute
    tens_minute = int (mn/10);
    ones_minute = mn % 10; 
    // take the latchPinTime low so 
    // the LEDs don't change while you're sending in bits:
    Serial.println("Start of display update");
    digitalWrite(latchPinTime, LOW);
//    delay(2);
    //shift out the bits:
    shiftOut(dataPinTime,clockPinTime, MSBFIRST,segment_pattern[tens_hour]);    
//    delay(1);
    shiftOut(dataPinTime,clockPinTime, MSBFIRST,segment_pattern[ones_hour]);
//    delay(1);
    shiftOut(dataPinTime,clockPinTime, MSBFIRST,segment_pattern[tens_minute]);
//    delay(1);
    shiftOut(dataPinTime,clockPinTime, MSBFIRST,segment_pattern[ones_minute]);
//    delay(1);
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPinTime, HIGH);
    Serial.println("End of loop");
    delay(5000);

}//end loop


void timeDisplay () {
    //separate the two digits of the hour
    tens_hour = int (hr/10);
    ones_hour = hr % 10;
    //separate the two digits of the minute
    tens_minute = int (mn/10);
    ones_minute = mn % 10; 
    // take the latchPinTime low so 
    // the LEDs don't change while you're sending in bits:
    digitalWrite(latchPinTime, HIGH);
    //shift out the bits:
    shiftOut(dataPinTime,clockPinTime, MSBFIRST,segment_pattern[tens_hour]);    
    shiftOut(dataPinTime,clockPinTime, MSBFIRST,segment_pattern[ones_hour]);
    shiftOut(dataPinTime,clockPinTime, MSBFIRST,segment_pattern[tens_minute]);
    shiftOut(dataPinTime,clockPinTime, MSBFIRST,segment_pattern[ones_minute]);
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPinTime, LOW);
}//end timeDisplay


void ISRclockTick () {
  sec += 1;
  ledState = !ledState;
  if (ledState){
    digitalWrite (ledPin, HIGH);
  }
  else{
    digitalWrite (ledPin, LOW); digitalWrite (ledPin, ledState);
  }
}//end ISRclockTick



void updateTime() {
  
  if (sec > 59) {
    sec = sec % 60;
    mn++;
  }

  if (mn > 59) {
    mn = mn % 60;
    hr++;
  }

  if (hr > 23) {
    hr = hr % 24;
    oldhr = hr;
  }
  
}//end updateTime
