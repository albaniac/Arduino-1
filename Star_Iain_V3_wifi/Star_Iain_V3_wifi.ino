#include <Adafruit_NeoPixel.h>

#define PIN 5

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(41,PIN, NEO_GRB + NEO_KHZ800);
int tail1 = 0;
int tail2 = 0;
int tail3 = 0;
int red = 000;
int blue = 100;
int green = 100;
int centreColor = 0;
const int switchPin = 2;
int pattern = 1;
int neopixel[40];
volatile boolean switchState = HIGH;
const int wifiSwitchPin = 3;
boolean wifiPin = LOW;
boolean oldwifiPin = LOW;
int oldPattern = 1;


void setup() {
  strip.begin();
  pinMode(switchPin, INPUT);
  digitalWrite(switchPin,HIGH);
  pinMode(wifiSwitchPin, INPUT);
  attachInterrupt (0,switchDetect,LOW);
  attachInterrupt (1,switchDetect,LOW);
  //turn all off
  for (int p = 0; p < 41; p++) strip.setPixelColor(p,0,0,0);
  strip.show();
}//end set up

void loop() {
  if (switchState == LOW) {pattern = (pattern +1) %4; switchState = HIGH; delay(500);}

  switch (pattern) {
                   case (0):
                   display_one();
                   break;
                   case (1):
                   display_two();
                   break;
                   case (2):
                   display_three();
                   break;
                   case(3):
                   display_four();
                   break;
                   }


}// end of loop

// first pattern
void display_one() {
                    for (centreColor = 0; centreColor < 256; centreColor = centreColor + 5) {for (int tail = 11; tail < 41; tail++)  {
                                                                                                                                    tail1 = tail + 1; tail2 = tail + 2; tail3 = tail + 3;
                                                                                                                                    if (tail == 38) tail3 = 11;
                                                                                                                                    if (tail == 39) {tail2 = 11; tail3 = 12;}
                                                                                                                                    if (tail == 40) {tail1 = 11; tail2 = 12; tail3 = 13;}
                                                                                                                                    strip.setPixelColor(tail, 0,0,0);
                                                                                                                                    strip.setPixelColor(tail1, red / 5,green / 5,blue / 5);
                                                                                                                                    strip.setPixelColor(tail2, red / 2,green / 2,blue / 2);
                                                                                                                                    strip.setPixelColor(tail3, red,green,blue);
                                                                                                                                    strip.show();
                                                                                                                                    delay(40);
                                                                                                                                    if (switchState == LOW) break;
                                                                                                                                    }//end chase for loop
                                                                                            if (switchState == LOW) break;
                                                                                            Centre (centreColor);
                                                                                            }// end of Color for loop
                     }//end of display_one
                                             
void display_two()  {
                    for (int i = 0; i < 42; i++) {
                                                 if (neopixel[i] > 0) neopixel [i] = neopixel[i] * .7;
                                                 strip.setPixelColor(i, neopixel[i],neopixel[i],neopixel[i]);
                                               }
                    neopixel[random(41)] = 200,200,200;
                    strip.show();
                    delay (50);
                    }


void display_three() {
                     //turn all off
                     for (int p = 0; p < 41; p++) strip.setPixelColor(p,0,0,0);
                     strip.show();
                     //turn on first 1
                     strip.setPixelColor(0,0,0,100);
                     strip.show();
                     delay(100);
                     //turn on second 5
                     for (int p = 1; p < 6; p++) strip.setPixelColor(p, 0, 0, 100);
                     // turn off first 1
                     strip.setPixelColor(0, 0 ,0, 0);
                     strip.show();
                     delay(100);
                     //turn on third 5
                     for (int p = 6; p < 11; p++) strip.setPixelColor(p, 0, 0, 100);
                     //turn off second 5
                     for (int p = 1; p < 6; p++) strip.setPixelColor(p, 0, 0, 0);
                     strip.show();
                     delay (100);
                     //turn on outer
                     for (int p = 11; p < 41; p++) strip.setPixelColor(p, 100, 100, 100);
                     // turn off second 5
                     for (int p = 6; p < 11; p++) strip.setPixelColor(p, 0, 0, 0);
                     strip.show();
                     delay(200);
                     }
                     
// pattern 4
void display_four() {
                    int off1;
                    int off2;                  
                     //turn on the middle LEDs
                    for (int p = 0; p < 11; p++) strip.setPixelColor(p, 60, 0, 0);
                    for (int rotate = 0; rotate < 5; rotate++) {
                                                           off1 = (((rotate * 6 + 2) % 30) + 11);
                                                           off2 = (((rotate * 6 + 14) % 30) + 11);
                                                           arrow (off1, off2);
                                                           strip.show();
                                                           if (switchState == LOW) return;
                                                           delay (800);
                                                           }
                    }

//turn of the two points not needed
void arrow (int firstOff, int secondOff) {
//turn all outside LEDs off
for (int q = 11; q < 41; q++) {strip.setPixelColor(q, 0, 0, 0); }
strip.show();
delay (1000);  
//turn all outside LEDs on  
for (int q = 11; q < 41; q++) {strip.setPixelColor(q, 100, 100, 100);}
//turn off one leg
for (int n = firstOff; n < (firstOff + 5); n++) {if (n == 40) strip.setPixelColor (11,0,0,0); strip.setPixelColor(n, 0, 0, 0);}
//turn off the other leg
for (int m = secondOff; m < (secondOff + 5); m++) {if (m == 40) strip.setPixelColor (11,0,0,0); strip.setPixelColor(m, 0, 0, 0);}
return;
}


 
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
  
 
void Centre (int Color) {
                        for (int i=0; i<11; i++) strip.setPixelColor (i,Wheel(Color));
                        strip.show();
                        }// end Centre

//ISR for interrupt 0 pin2 and pin3                       
void switchDetect() {
                    switchState = LOW;
                    delay (50);
                    }
