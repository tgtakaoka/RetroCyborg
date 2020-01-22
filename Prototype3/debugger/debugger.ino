/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
/**
   Cyborg09 controller

   This sketch is designed for Adafruit ItsyBitsy 32u4 16MHz 5V and
   controll vanilla HD63C09E MPU.

   USB Console port is used as console. So one can connect with
   $ screen /dev/<tty USB> 9600
*/

#include "commands.h"
#include "console.h"
#include "input.h"
#include "pins.h"

void setup() {
  Console.begin(115200);
  Pins.begin();
  Input.begin();
}

void loop() {
  if (Pins.isRunning()) {
    Pins.loop();
  } else {
    Input.loop();
  }
}
