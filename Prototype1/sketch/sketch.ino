/**
   Cyborg09 controller

   This sketch is designed for SparkFun Pro Micro 16MHz 5V and controll
   vanilla HD63C09E MPU.

   USB Serial port is used as console. So one can connect with
     $ screen /dev/<tty USB> 9600

*/

#include "input.h"
#include "pins.h"

static struct Pins::Status previous_cycle;

void setup() {
  Serial.begin(9600);
  Pins.begin(previous_cycle);
}

void loop() {
  Input.loop();
  struct Pins::Status pins;
  Pins.cycle(pins);
  if (!pins.inHalt() && !pins.equals(previous_cycle))
    pins.print();
  previous_cycle = pins;
}
