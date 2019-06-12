/**
   Cyborg09 controller

   This sketch is designed for Adafruit ItsyBitsy 32u4 16MHz 5V and
   controll vanilla HD63C09E MPU.

   USB Serial port is used as console. So one can connect with
     $ screen /dev/<tty USB> 9600
*/

#include "input.h"
#include "pins.h"

void setup() {
  Serial.begin(115200);
  Pins.begin();
}

void loop() {
  Input.loop();
}
