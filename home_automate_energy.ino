#include <Wire.h>

const byte address = 55; //37 hex

unsigned long pre_tm = millis();
int impulseRead = 0;
byte state = HIGH;
byte pin = 3;

void setup() {
  pinMode(pin, INPUT);
  digitalWrite(pin, HIGH);
  Wire.begin(address);
  Wire.onRequest(requestEvent);
}

void requestEvent() {
  Wire.write(impulseRead);
  
  impulseRead = 0;
}

void loop() {
  
  byte value = digitalRead(pin);
  if( value != state) {
    impulseRead = impulseRead + 1;
    state = value;
  }
}

