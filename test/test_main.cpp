#include <Arduino.h>

void setup()
{                
  Serial.begin(9600);
}

int val;

void loop()                     
{
  val = analogRead(1);
  Serial.println(val);
  delay(25);
}