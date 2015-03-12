#include <EEPROM.h>
#include "SODA.h"
#include <SdFat.h>
#include <Wire.h>
SODA soda;

void setup(){
  soda.begin();
}

void loop(){
  float t = soda.getClockTemp();
  int k = soda.tcReadK(1);
  Serial.print("Reference Temp: ");
  Serial.println(t);
  Serial.print("Thermocouple Temp: ");
  Serial.println(k);
  Serial.println();
  Serial.println();
}


