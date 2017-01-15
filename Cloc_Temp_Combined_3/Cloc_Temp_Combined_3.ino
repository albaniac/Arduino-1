//Rob's clock with two thermometers - development version

/* Pin assignments:

0  Unused
1  Unused
2  Time bus
3  Time bus
4  Time bus
5  One wire bus
8  Temp bus
9  Temp bus
10 Temp bus
11 Led Colon
*/

// include the required libraries
//for temperature sensors
#include <OneWire.h>
#include <DallasTemperature.h>
//for real time clock DS1307
#include "RTClib.h"
#include <Wire.h>

//set up temperature sensors:
// Data wire is plugged into pin 5 on the Arduino for the temperature sensor
#define ONE_WIRE_BUS 5

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//two DS18B20 sensors used:
DeviceAddress insideThermometer = {0x28, 0x7F, 0x15, 0x9D, 0x04, 0x00, 0x00, 0xEE};
DeviceAddress outsideThermometer = {0x28, 0x11, 0x54, 0x3B, 0x05, 0x00, 0x00, 0x11};

// define the RTC type:
RTC_DS1307 rtc;

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

ONE_WIRE_BUS
//Pin connected to ST_CP of 74HC595 time
int latchPinTime = 2;
//Pin connected to SH_CP of 74HC595 time
int clockPinTime = 3;
////Pin connected to DS of 74HC595 time
int dataPinTime = 4;
//Pin connected to ST_CP of 74HC595 temp
int latchPinTemp = 8;
//Pin connected to SH_CP of 74HC595 temp
int clockPinTemp = 9;
////Pin connected to DS of 74HC595 temp
int dataPinTemp = 10;
//Pin connected to the leds used to make the colon display
int ledPin = 11;
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
//used to count the number of millis and to change the state of the led colon every second
unsigned long time = 0;
//used to set the led colon display on or off
boolean ledState = false;
// used to derive a temperature check every minute
int start = 0;


void setup() {
  sensors.begin();
  // set the resolution to 10 bit is 0.25 degree and 9 bit 0.5 degree.  
  sensors.setResolution(insideThermometer, 10);
  sensors.setResolution(outsideThermometer, 10);
  Wire.begin();
  rtc.begin();
  // following line sets the RTC to the date & time this sketch was compiled
  //rtc.adjust(DateTime(__DATE__, __TIME__));
 
  //set pins to output so you can control the shift register
  pinMode(latchPinTime, OUTPUT);
  pinMode(clockPinTime, OUTPUT);
  pinMode(dataPinTime, OUTPUT);
  pinMode(latchPinTemp, OUTPUT);
  pinMode(clockPinTemp, OUTPUT);
  pinMode(dataPinTemp, OUTPUT);
  pinMode(ledPin, OUTPUT);
  sensors.setWaitForConversion (FALSE);
  sensors.setWaitForConversion
  
}

void loop() {
    tempdisplay ();
    DateTime now = rtc.now();
    tempdisplay ();
    if (int(now.minute()) != start)
    {
       start = (int(now.minute()));
       sensors.requestTemperatures();
       for (int i = 0; i<200; i++) tempdisplay();
       inside_temp = sensors.getTempC(insideThermometer);
       for (int i = 0; i<200; i++) tempdisplay();
       outside_temp = sensors.getTempC(outsideThermometer);
       for (int i = 0; i<200; i++) tempdisplay();
    }
     if ( inside_temp > 99.9 ) inside_temp = 99.9;  
    tempdisplay ();
    if ( outside_temp < -9.9 ) outside_temp = -9.9;
    tempdisplay ();
    inside_int_temp = int (inside_temp*10);
    tempdisplay ();
    outside_int_temp = int (outside_temp*10);
    tempdisplay ();
    // check if temperature is negative and if it is make the first character a
    // minus sign and change the value of integer_temperature to a positive value
    if (inside_int_temp < 0 ) {output [0] = 21; inside_int_temp = -1* inside_int_temp;}
    // if it is not negative derive the tens 
    else output [0] = inside_int_temp / 100;
    if ( output [0] == 0 ) output[0] = 22;
    tempdisplay ();
    if (outside_int_temp < 0 ) {output [4] = 21; outside_int_temp = -1* outside_int_temp;}
    // if it is not negative derive the tens 
    else output [4] = outside_int_temp / 100;
    if ( output [4] == 0 ) output [4] = 22;
    // derive the ones
    tempdisplay ();
    output [1] = ((inside_int_temp / 10) % 10) +10;
    tempdisplay ();
    output [5] = ((outside_int_temp / 10) % 10) +10;
    tempdisplay ();
    // derive the tenths
    output [2] = inside_int_temp % 10;
    tempdisplay ();
    output [6] = outside_int_temp % 10;
    tempdisplay ();
    // the last digit is always a "C"
    output [3] = 20;
    tempdisplay ();
    output [7] = 20;
    tempdisplay ();
  
  
    //update the time display
    //separate the two digits of the hour
    tens_hour = int (now.hour()/10);
    tempdisplay ();
    ones_hour = now.hour() % 10;
    //separate the two digits of the minute
    tempdisplay ();
    tens_minute = int (now.minute()/10);
    ones_minute = now.minute() % 10; 
    tempdisplay ();
    // take the latchPinTime low so 
    // the LEDs don't change while you're sending in bits:
    digitalWrite(latchPinTime, LOW);
    tempdisplay ();
    //shift out the bits:
    shiftOut(dataPinTime,clockPinTime, MSBFIRST,segment_pattern[tens_hour]);    
    tempdisplay ();
    shiftOut(dataPinTime,clockPinTime, MSBFIRST,segment_pattern[ones_hour]);
    tempdisplay ();
    shiftOut(dataPinTime,clockPinTime, MSBFIRST,segment_pattern[tens_minute]);    
    tempdisplay ();
    shiftOut(dataPinTime,clockPinTime, MSBFIRST,segment_pattern[ones_minute]);
    tempdisplay ();
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPinTime, HIGH);
    tempdisplay ();

}
void tempdisplay () {
   //display each digit then return
    for (int i = 0; i < 8; i++)
    {
    // take the latchPin low so 
    // the LEDs don't change while you're sending in bits:
    digitalWrite(latchPinTemp, LOW);
    //shift out the byte patterns one digit at a time 1 to 8
    shiftOut(dataPinTemp,clockPinTemp, MSBFIRST,digit_pattern[(i % 8)]);
    shiftOut(dataPinTemp,clockPinTemp, MSBFIRST,segment_pattern[output[i % 8]]);
    // take the latchPin high to light the display
    digitalWrite (latchPinTemp, HIGH);
    delay(2);
    //turn off the last digit so it is not over bright
    digitalWrite (latchPinTemp, LOW);
    shiftOut (dataPinTemp, clockPinTemp, MSBFIRST, digit_pattern[7]);
    shiftOut (dataPinTemp, clockPinTemp, MSBFIRST, segment_pattern[22]);
    digitalWrite (latchPinTemp, HIGH);
    }
    if (millis () > time + 1000)
    {
    ledState = !ledState;
    time = millis();
    digitalWrite (ledPin, ledState);
    }
    
}

