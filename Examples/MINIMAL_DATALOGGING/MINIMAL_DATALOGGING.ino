#include <EEPROM.h>
#include "SODA.h"
#include <SdFat.h>
#include <Wire.h>
SODA soda;

void setup(){
  soda.begin();
}

void loop(){
  soda.communicate();
  float t = soda.getClockTemp();
  int a1 = analogRead(A1);
  soda.dataLineBegin();
  soda.dataLineAdd(t);
  soda.dataLineAdd(A1);
  soda.dataLineEnd();
  soda.setWake(10,1);
  soda.turnOff();
}


