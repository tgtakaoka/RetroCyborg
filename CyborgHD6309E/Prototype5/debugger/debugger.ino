/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
/**
 *  CyborgHD6309E controller
 *
 * This sketch is designed to controll and debug MC6809E/HD6309E MPU.
 */

#include <libcli.h>

#include "commands.h"
#include "pins.h"

libcli::Cli &cli = libcli::Cli::instance();

void setup() {
    Pins.begin();
    Commands.begin();
    interrupts();
}

void loop() {
    if (Pins.isRunning()) {
        Pins.loop();
    } else {
        cli.loop();
        Commands.loop();
    }
}
