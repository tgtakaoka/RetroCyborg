/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
/**
   CyborgHD6309E controller

   This sketch is designed for ATmega1284p/MegaCoreX and
   controll vanilla HD6309E MPU.

   USB Console port is used as console. So one can connect with
   $ screen /dev/<tty USB> 115200
*/

#include <Arduino.h>
#include <libcli.h>

#include "commands.h"
#include "pins.h"

libcli::Cli Cli;

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
