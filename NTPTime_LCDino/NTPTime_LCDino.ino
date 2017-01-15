/*
 * 
 */
#include <NTPtimeESP.h>
#include <hd44780_I2Cexp.h>
uint8_t hr = 8;
uint8_t mn = 20;
uint8_t sec = 35;
unsigned long startMillis;
uint8_t oldhr = 0;

hd44780_I2Cexp lcd; // declare lcd object: auto locate & config exapander chip

// LCD geometry
const int LCD_ROWS = 2;
const int LCD_COLS = 16;

NTPtime NTPuk("0.pool.ntp.org");
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
  Serial.begin(9600);
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.print("Hello, World!");
  Serial.println();
  Serial.println("Booted");
  Serial.println("Connecting to Wi-Fi");

  WiFi.mode(WIFI_STA);
  WiFi.begin ("Kercem2", "E0E3106433F4");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi connected");
}

void loop() {

  startMillis = millis();  //used to time the updates
  if (oldhr < hr) {
    // first parameter: Time zone in floating point (for India); second parameter: 1 for European summer time; 2 for US daylight saving time (not implemented yet)
    dateTime = NTPuk.getNTPtime(0.0, 1);
    //NTPuk.printDateTime(dateTime);
    hr = dateTime.hour;
    mn = dateTime.minute;
    sec = dateTime.second;
    int actualyear = dateTime.year;
    byte actualMonth = dateTime.month;
    byte actualday =dateTime.day;
    byte actualdayofWeek = dateTime.dayofWeek;
    oldhr = hr;
  }
  
  updateTime(hr, mn, sec);
  displayTime(hr, mn, sec);
  
  while (millis() < startMillis + 1000) {
    delay(10); // loop
  }
  sec = updateSecond(sec); // main clock ticker
  
} //end of loop


void displayTime(uint8_t h, uint8_t m, uint8_t s) {
  lcd.setCursor(0, 1);

  if (h < 10 ) {
    lcd.print("0");  
  }
  
  lcd.print(h);
  lcd.print(":");

  
  if (m < 10 ) {
    lcd.print("0");
  }
  
  lcd.print(m);
  lcd.print(":");

  
  if (s < 10 ) {
    lcd.print("0");
  }
  
  lcd.println(s);
    
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
    hr++;
  }

  if (hr > 23) {
    hr = hr % 24;
    oldhr = hr;
  }
  
}
