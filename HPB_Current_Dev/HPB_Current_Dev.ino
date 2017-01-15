// This example drives the stepper motors directly
// Two LDRs are used to detect light level left, centre and right.
// During setup the LDRs are calibrated - move the robot around light and dark areas.
// Neopixels are set to red and green fixed.
// A smoothing constant is provided for weighted averaging of readings but set to 0
// results in no smoothing.
// The forward direction is set to run longer than a turn - each turn is about a quarter.
// After a turn the next direction must be forward to avoid left / right oscillations
// Robin Harris 11th September 2016
////////////////////////////////////////////////
#include <Adafruit_NeoPixel.h>
#include <NewPing.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 12 //Pin that neopixels are driven by
#define NUM_LEDS 2
#define TRIGGER_PIN 2  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN 3  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.


#define BRIGHTNESS 100
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, PIN, NEO_RGB + NEO_KHZ800);

int gamma[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

//declare variables for the motor pins
int rmotorPin1 = 8;    // Blue   - 28BYJ48 pin 1
int rmotorPin2 = 9;    // Pink   - 28BYJ48 pin 2
int rmotorPin3 = 10;   // Yellow - 28BYJ48 pin 3
int rmotorPin4 = 11;   // Orange - 28BYJ48 pin 4
                        // Red    - 28BYJ48 pin 5 (VCC)

int lmotorPin1 = 4;    // Blue   - 28BYJ48 pin 1
int lmotorPin2 = 5;    // Pink   - 28BYJ48 pin 2
int lmotorPin3 = 6;    // Yellint ldr_pin_centre = A0;ow - 28BYJ48 pin3
int lmotorPin4 = 7;   // Orange - 28BYJ48 pin 4
                        // Red    - 28BYJ48 pin 5 (VCC)

int ldr_pin_left = A0;
int ldr_pin_right = A1;

int obstacleDistance = 100; //used to store distance to any obstacle
boolean obstacle = false;
int ldr_left = 0;      // variable to hold the last reading of the  left LDR pin
int ldr_right = 0;     // variable to hold the last reading of the right LDR pin
int motorSpeed = 1000;  //variable to set stepper speed
int count = 0;         // count of steps made
int countsperrev = 512;// number of steps per full revolution
int lookup[8] = {B01000, B01100, B00100, B00110, B00010, B00011, B00001, B01001};
int lux_left = 0;      
int lux_right = 0;

const int STOP = 0;
const int FORWARD = 1;
const int BACK = 2;
const int LEFT = 3;
const int RIGHT = 4;

int moveState = FORWARD;

// intialise calibrate readings to zero
int cal_min_L = 0;
int cal_max_L = 0;
int cal_min_R = 0;
int cal_max_R = 0;

//initiaise min and max values to be 'opposite' extreme so can be updated by 'calibrate'
int min_L = 1023;
int max_L = 0;
int min_R = 1023;
int max_R = 0;

int k = 0; // smoothing factor

//////////////////////////////////////////////////////////////////////////////
 void setup() {
   //declare the motor pins as outputs
   pinMode(lmotorPin1, OUTPUT);
   pinMode(lmotorPin2, OUTPUT);
   pinMode(lmotorPin3, OUTPUT);
   pinMode(lmotorPin4, OUTPUT);
   pinMode(rmotorPin1, OUTPUT);
   pinMode(rmotorPin2, OUTPUT);
   pinMode(rmotorPin3, OUTPUT);
   pinMode(rmotorPin4, OUTPUT);
   Serial.begin(9600);
   strip.setBrightness(BRIGHTNESS);
   strip.begin();
   strip.setPixelColor(0,200,200,200);
   strip.setPixelColor(1,200,200,200);
   strip.show();
   calibrate ();
 }

void loop()
  {    
    obstacleDistance = sonar.ping_cm(); // Send ping, get distance in cm and print result (0 = outside set distance range)
    if (obstacleDistance < 20 && obstacleDistance > 0) {
      obstacle = true;
    }
    ldr_left = analogRead(ldr_pin_left);
    ldr_right = analogRead(ldr_pin_right);

    lux_left = (lux_left*10 * k)/1000 + (ldr_left*10 * (100-k))/1000;
    lux_right = (lux_right*10 * k)/1000 + (ldr_right*10 * (100-k))/1000;
    if (obstacle) {
        moveState = LEFT;
        obstacle = false;
        strip.setPixelColor(0,0,245,255);
        strip.setPixelColor(1,0,245,255);
        strip.show();
        for (count = 0; count < 50;  count++)
        {
          moveStep();
        }
        moveState = FORWARD;
      }
      else if (moveState == FORWARD) { // no obstacle so check light levels
           if (lux_left > (lux_right + 15))
           {
            moveState = RIGHT;
            strip.setPixelColor(0,200,0,0);
            strip.setPixelColor(1,200,0,0);
            strip.show();

           }
          else if (lux_right > (lux_left + 15))
            {
            moveState = LEFT;
            strip.setPixelColor(0,0,200,0);
            strip.setPixelColor(1,0,200,0);
            strip.show();
            }
        }
      else
        {
          moveState = FORWARD;
          strip.setPixelColor(0,255,255,0);
          strip.setPixelColor(1,255,255,0);
          strip.show();
        }
 
    if (moveState == FORWARD) {
       for (count = 0; count < 100;  count++) // if moving forward continue longer than turning
      {
        moveStep();
      }
    }
    else for (count = 0; count < 50;  count++)
      {
        moveStep();
      }
  } // end of loop

void moveStep()
{
   for(int i = 0; i < 8; i++)
   {
    switch(moveState)
    {
      case STOP:
        digitalWrite(lmotorPin1,0);
        digitalWrite(lmotorPin2,0);
        digitalWrite(lmotorPin3,0);
        digitalWrite(lmotorPin4,0);

        digitalWrite(rmotorPin1,0);
        digitalWrite(rmotorPin2,0);
        digitalWrite(rmotorPin3,0);
        digitalWrite(rmotorPin4,0);
 
        return;
      case FORWARD:
        setOutputDir(i,7-i);
        break;
      case BACK:
        setOutputDir(7-i,i);
        break;
      case LEFT:
        setOutputDir(7-i,7-i);
        break;
      case RIGHT:
        setOutputDir(i,i);
        break;
    }
   delayMicroseconds(motorSpeed);     
   }
}

void setOutputDir(int leftOut, int rightOut)
{
   digitalWrite(lmotorPin1, bitRead(lookup[leftOut], 0));
   digitalWrite(lmotorPin2, bitRead(lookup[leftOut], 1));
   digitalWrite(lmotorPin3, bitRead(lookup[leftOut], 2));
   digitalWrite(lmotorPin4, bitRead(lookup[leftOut], 3));
  
   digitalWrite(rmotorPin1, bitRead(lookup[rightOut], 0));
   digitalWrite(rmotorPin2, bitRead(lookup[rightOut], 1));
   digitalWrite(rmotorPin3, bitRead(lookup[rightOut], 2));
   digitalWrite(rmotorPin4, bitRead(lookup[rightOut], 3));
}

void calibrate()
{
  int start_millis = millis();
  Serial.println("Starting calibration....");
  for (int c = 1; c < 10000; c++) {
      ldr_left = analogRead(ldr_pin_left);
      ldr_right = analogRead(ldr_pin_right);
      if (ldr_left < min_L) 
        min_L = ldr_left;
      if (ldr_right < min_R) 
        min_R = ldr_right;
      if (ldr_left > max_L) 
        max_L = ldr_left;
      if (ldr_right > max_R) 
        max_R = ldr_right;  
  
  }
  cal_min_L = min_L;
  cal_max_L = max_L;
  cal_min_R = min_R;
  cal_max_R = max_R;
} //end of calibrate

