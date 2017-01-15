unsigned long duration;

void setup() {
  
pinMode(1, INPUT);

}

void loop() {
duration = pulseIn(1, HIGH);
}
