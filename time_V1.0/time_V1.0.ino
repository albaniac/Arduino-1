/* Sketch to display time - Version 1.0
 *  Robin Harris 31st August 2016
 */

uint8_t hour = 8;
uint8_t minute = 20;
uint8_t second = 35;
unsigned long startMillis;


void setup() {
Serial.begin(9600);

} //end setup


void loop() {
  startMillis = millis();
  
  updateTime(hour, minute, second);
  displayTime(hour, minute, second);
  
  while (startMillis < millis() + 1000) {
    ; // loop
  }
  
  second = updateSecond(second); // main clock ticker
} //end loop


void displayTime(uint8_t h, uint8_t m, uint8_t s) {


  if (hour < 10 ) {
    Serial.print("0");  
  }
  
  Serial.print(hour);
  Serial.print(":");

  
  if (minute < 10 ) {
    Serial.print("0");
  }
  
  Serial.print(minute);
  Serial.print(":");

  
  if (second < 10 ) {
    Serial.print("0");
  }
  
  Serial.print(second);
  Serial.print("\t");
  
} //end displayTime

uint8_t updateSecond(uint8_t s) {
  s += (millis() - startMillis)/1000; // increment seconds
  return s;
} //end updateSeconds

void updateTime(uint8_t h, uint8_t m, uint8_t s) {
  
  if (s > 59) {
    s = 0;
    m += 1;
  }

  if (m>59) {
    m = 0;
    h += 1;
  }

  if (h > 23) {
    h = 0;
  }
  
}

