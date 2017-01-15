#include <DallasTemperature.h>
#include <OneWire.h>

#define outputPin D6
#define togglePin D8

int inside_temp;
int outside_temp;
boolean pinState = TRUE;


// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(outputPin);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//two DS18B20 sensors used:
DeviceAddress insideThermometer = {0x28, 0x7F, 0x15, 0x9D, 0x04, 0x00, 0x00, 0xEE};
DeviceAddress outsideThermometer = {0x28, 0x11, 0x54, 0x3B, 0x05, 0x00, 0x00, 0x11};
DeviceAddress testThermometer = {0x28, 0x39, 0x19, 0x08, 0x00, 0x00, 0x80, 0x7A};



void setup() {
    pinMode(togglePin, OUTPUT);

    sensors.begin();
    Serial.begin(9600);
    //pinMode(outputPin, OUTPUT);
    Serial. println("Starting setup");
    //analogWrite(outputPin, 800);
    sensors.setResolution(insideThermometer, 10);
    sensors.setResolution(outsideThermometer, 10);
    sensors.setWaitForConversion (TRUE);
    Serial.println("End of setup");
}//end setup

void loop() {
    sensors.requestTemperatures();
    inside_temp = sensors.getTempC(testThermometer);
    outside_temp = sensors.getTempC(testThermometer);
    Serial.print("Test temperature: ");
    Serial.println(inside_temp);
    pinState=!pinState;
    digitalWrite(togglePin, pinState);
    delay(2000);

}
