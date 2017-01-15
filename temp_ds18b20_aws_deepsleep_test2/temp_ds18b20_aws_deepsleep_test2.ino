/* This sketch reads temperature and humidity from a DS18DS sensor
 * Output is provided to a serial monitor and values are posted to an AWS IoT shadow
 * Robin Harris 21st July 2016
 */

#include <AmazonIOTClient.h>
#include <Esp8266AWSImplementations.h>
#include <AWSFoundationalTypes.h>
#include "keys.h"
#include <OneWire.h>
#include <DallasTemperature.h>


const int sleepTimeS = 10; //number of seconds to deep sleep

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because 
                                    // it is acive low on the ESP-01)
  delay(1000);                      // Wait for a second
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000); 
  digitalWrite(LED_BUILTIN, HIGH);
  // Wait for two seconds (to demonstrate the active low LED)
  ESP.deepSleep(sleepTimeS * 1000000);
}  //end of setup

void loop() {
}  //end of main loop


