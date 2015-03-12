#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include "SODA.h"
#include <SdFat.h>
#include <Wire.h>

SODA soda;
float temp[4];
int num_sensors = 4;
void setup(){
  soda.begin();
  pinMode(1,OUTPUT);
  pinMode(4,INPUT);
  digitalWrite(1,HIGH);
}

void loop(){
  soda.communicate();
  getDS18B20(4,4,temp);
  soda.dataLineBegin();
  for(int k = 0; k<num_sensors; k++){
    soda.dataLineAdd(temp[k]);
  }
  soda.dataLineEnd();
  soda.setWake(10,2);
  soda.turnOff();
}

/////////////////////////////////////////////////////////////////////////////////
void getDS18B20(int chan, int ns,float temp[]){  
  //setup oneWire buses 
  OneWire bus1 (chan) ;
  DallasTemperature tempA(&bus1);
  delay(50);
  tempA.begin();
  tempA.requestTemperatures(); // Send the command to get temperatures
  for(int k = 0; k < ns; k++){
    temp[k] = tempA.getTempCByIndex(k); 
  } 
}
