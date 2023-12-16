/**
 *  BionicF3850 controller
 *
 * This sketch is designed to controll and debug INS8070 MPU.
 */

#include <libcli.h>

#include <string.h>

#include "commands.h"
#include "pins.h"
#include "unio_bus.h"
#include "unio_eeprom.h"

#define PIN_ID 34 /* P7.29 */

libcli::Cli cli;
unio::UnioBus unioBus{PIN_ID, 16};

void writeIdentity(const char *identity) {
    const auto len = strlen(identity);
    unio::Eeprom rom{unioBus};
    const auto *buffer = reinterpret_cast<const uint8_t *>(identity);
    rom.write(0, len + 1, buffer);
}

void setup() {
    Console.begin(115200);
    cli.begin(Console);
    unioBus.begin();
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
