#include "Eeprom.h"
#include <Arduino.h>

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  PresetFabrica();
  Serial.println("EEPROM GRABADA");
}

void loop() {
  // put your main code here, to run repeatedly:
}
