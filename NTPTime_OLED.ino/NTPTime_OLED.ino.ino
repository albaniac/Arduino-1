/* Clock with a display on 0.96" OLED I2C display  
 * Time is set using NTP and synchronised every hour.  
 * Version 15th Sept 2016
 */
#include <NTPtimeESP.h>
#include <Wire.h>  // can be left out in arduino.cc IDE 1.6.7 and later
#include "SSD1306.h"
#include "OLEDDisplayUi.h"
#include "fontclock.h"

uint8_t hr = 8;
uint8_t mn = 20;
uint8_t sec = 35;
unsigned long startMillis;
boolean needToGetTime = true;
String timeString ="";
String dateString ="";
const char* dayText[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, D3, D5);

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

void setup() {
  display.init();
  display.flipScreenVertically();
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  //WiFi.begin ("C4Di_Members", "c4d1day0ne");
  WiFi.begin ("Kercem2", "E0E3106433F4");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("Connected");
}

void loop() {
  startMillis = millis();  //used to time the updates
  if (needToGetTime) {
    // first parameter: Time zone in floating point (for India); second parameter: 1 for European summer time; 2 for US daylight saving time (not implemented yet)
    dateTime = NTPuk.getNTPtime(0.0, 1);
    //NTPuk.printDateTime(dateTime);
    hr = dateTime.hour;
    mn = dateTime.minute;
    sec = dateTime.second;
    needToGetTime = false;
   } // end if that gets new NTP time
  
  updateTime(hr, mn, sec);
  displayTime(hr, mn, sec);
  
  while (millis() < startMillis + 1000) {
    delay(10); // loop
  }
  sec = updateSecond(sec); // main clock ticker
  
} //end of loop


void displayTime(uint8_t h, uint8_t m, uint8_t s) {
  timeString="";
  dateString="";
  if (h < 10 ) {   //insert a 0 if hours are one digit
    timeString="0";
  }
  timeString+=String(h); //insert hours
  timeString+=":";  // insert colon
  if (m < 10 ) {//insert a 0 if minutes are one digit
    timeString+="0";
  }
  timeString+=String(m); //insert minutes
  timeString+=":";  // insert colon
  
  if (s < 10 ) {//inser a 0 if seconds are one digit
    timeString+="0";
  }
  timeString+=String(s); //insert minutes

  int actualYear = dateTime.year;
  byte actualMonth = dateTime.month;
  byte actualDay = dateTime.day;
  byte actualDayofWeek = dateTime.dayofWeek-1;
    
  dateString=(dayText[actualDayofWeek]);
  dateString+= " ";
  dateString+=String(actualDay);
  dateString+= "/";
  dateString+=String(actualMonth);
  dateString+= "/";
  dateString+=String(actualYear-2000);

  if (needToGetTime) {
    dateString+= "  *"; //put a * at the end of
                     //the display if need to get a new NTP time  
  }
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(DejaVu_LGC_Sans_Mono_Bold_20);
  display.drawString(17,12,timeString);
  display.setFont(DejaVu_LGC_Sans_Mono_16);
  display.drawString(0,40,dateString);
  display.display();
} //end displayTime

uint8_t updateSecond(uint8_t s) {
  s += (millis() - startMillis)/1000; // increment seconds
  return s;
} //end updateSeconds

void updateTime(uint8_t h, uint8_t m, uint8_t s) {
  
  if (sec > 59) {
    sec = sec % 60;
    mn++;
  }

  if (mn > 59) {
    mn = mn % 60;
    needToGetTime = true;
    hr++;
  }

  if (hr > 23) {
    hr = hr % 24;
  }
  
}
