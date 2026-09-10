#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Setup - Start");
  Serial.println("Program: Paffee");
  Serial.println("Setup - End"); 

}

void loop() {
  Serial.println("Hello World");
}