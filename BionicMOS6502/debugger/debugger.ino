/**
 *  BionicMOS6502 controller
 *
 * This sketch is designed to controll and debug MOS6502 MPU and variants.
 */

#include <libcli.h>

#include "commands.h"
#include "pins.h"

libcli::Cli cli;

void setup() {
    Console.begin(115200);
    cli.begin(Console);
    Pins.begin();
    Commands.begin();
}

void loop() {
    if (Pins.isRunning()) {
        Pins.loop();
    } else {
        cli.loop();
        Commands.loop();
    }
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
