/* Clock with a display on a 1602 LCD display connected via an I2C adapter.
 * Time is set using NTP and synchronised every hour.  Version 8th Sept 2016
 */
#include <NTPtimeESP.h>
#include <hd44780_I2Cexp.h>
#include <Wire.h>  // can be left out in arduino.cc IDE 1.6.7 and later
#include <hd44780.h>  // can be left out in arduino.cc IDE 1.6.7 and later
uint8_t hr = 8;
uint8_t mn = 20;
uint8_t sec = 35;
unsigned long startMillis;
boolean needToGetTime = true;

hd44780_I2Cexp lcd; // declare lcd object: auto locate & config exapander chip

// LCD geometry
const int LCD_ROWS = 2;
const int LCD_COLS = 16;

const char* dayText[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

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
  Wire.begin(0,2); // GPIO0 for SDA and GPIO2 SCL
  lcd.begin(LCD_COLS, LCD_ROWS);

  WiFi.mode(WIFI_STA);
  WiFi.begin ("Kercem2", "E0E3106433F4");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.begin(9600);
  
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
   if (h < 10 ) {
    Serial.print("0");  
  }
  
  Serial.print(h);
  Serial.print(":");

  
  if (m < 10 ) {
    Serial.print("0");
  }
  
  Serial.print(m);
  Serial.print(":");

  
  if (s < 10 ) {
    Serial.print("0");
  }
  
  Serial.print(s);

  int actualYear = dateTime.year;
  byte actualMonth = dateTime.month;
  byte actualDay =dateTime.day;
  byte actualDayofWeek = dateTime.dayofWeek-1;
    
  Serial.print (dayText[actualDayofWeek]);
  Serial.print (" ");
  Serial.print (actualDay);
  Serial.print ("/");
  Serial.print (actualMonth);
  Serial.print ("/");
  Serial.print (actualYear);
  Serial.print ("  ");
  if (needToGetTime) {
    Serial.print ("*"); //put a * at the end of
                     //the display if need to get a new NTP time  
  }
  else {
    Serial.print (" "); // blank out * is NTP has been updated
  }
  
//  lcd.setCursor(4, 0); // column 5 of row 1
//
//  if (h < 10 ) {
//    lcd.print("0");  
//  }
//  
//  lcd.print(h);
//  lcd.print(":");
//
//  
//  if (m < 10 ) {
//    lcd.print("0");
//  }
//  
//  lcd.print(m);
//  lcd.print(":");
//
//  
//  if (s < 10 ) {
//    lcd.print("0");
//  }
//  
//  lcd.print(s);
//
//  int actualYear = dateTime.year;
//  byte actualMonth = dateTime.month;
//  byte actualDay =dateTime.day;
//  byte actualDayofWeek = dateTime.dayofWeek-1;
//    
//  lcd.setCursor(1,1);
//  lcd.print (dayText[actualDayofWeek]);
//  lcd.print (" ");
//  lcd.print (actualDay);
//  lcd.print ("/");
//  lcd.print (actualMonth);
//  lcd.print ("/");
//  lcd.print (actualYear);
//  lcd.print ("  ");
//  if (needToGetTime) {
//    lcd.print ("*"); //put a * at the end of
//                     //the display if need to get a new NTP time  
//  }
//  else {
//    lcd.print (" "); // blank out * is NTP has been updated
//  }
//  
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
