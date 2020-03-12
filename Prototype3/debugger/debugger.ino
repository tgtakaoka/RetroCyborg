/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
/**
   Cyborg09 controller

   This sketch is designed for Adafruit ItsyBitsy 32u4 16MHz 5V and
   controll vanilla HD63C09E MPU.

   USB Console port is used as console. So one can connect with
   $ screen /dev/<tty USB> 9600
*/

#include "commands.h"
#include "libcli.h"
#include "pins.h"

void setup() {
  Pins.begin();
  Commands.begin();
  interrupts();
}

void loop() {
  if (Pins.isRunning()) {
    Pins.loop();
  } else {
    Cli.loop();
    Commands.loop();
  }
}
