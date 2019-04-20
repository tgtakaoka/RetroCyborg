/*
  The Controller can accept commands represented by one letter.

   r - assert #RES to LOW.
   R - negate #RES to HIGH.
   h - assert #HALT to LOW.
   H - negate #HALT to HIGH.
   k - stop negerating clock E and Q.
   K - start free running clock E and Q.
   c - one bus cycle. stopped at E=HIGH Q=LOW.
   s - print 6309 hardware signal status.
   S - print 6309 registers.
   d - set data bus, input 2 digits hex number.
   a - set A register, input 2 digits hex number.
   x - set X register, input 4 digits hex number.
   ? - print version.
*/

#include <Arduino.h>

#include "clock.h"
#include "commands.h"
#include "input.h"
#include "pins.h"

#define VERSION F("* Cyborg09 Prototype1 0.3")

class Commands Commands;

static void handleDataBus(char command, uint8_t values[], uint8_t num) {
  bool lic;
  for (uint8_t i = 0; i < num; i++) {
    lic = Pins.isLic();
    Pins.setDataBusDirection(OUTPUT);
    Pins.setDataBus(values[i]);
    Pins.printStatus();
    Clock.cycle(1);
  }
  if (command == 'd' || lic) {
    Pins.printStatus();
    return;
  }
  do {
    lic = Pins.isLic();
    Clock.cycle(1);
    Pins.printStatus();
  } while (!lic);
}

void Commands::loop() {
  const char c = Serial.read();
  if (c == 's') Pins.printStatus();
  if (c == 'r') Pins.reset(LOW);
  if (c == 'R') Pins.reset(HIGH);
  if (c == 'h') Pins.halt(LOW);
  if (c == 'H') Pins.halt(HIGH);
  if (c == 'k') Clock.stop();
  if (c == 'K') Clock.run();
  if (c == 'd' || c == 'i') {
    Input.readHex(c, handleDataBus);
  }
  if (c == 'c') {
    Clock.cycle();
    Pins.printStatus();
  } else if (Clock.isRunning()) {
    Clock.cycle(1000);
    Pins.printStatus();
  }
  if (c == '?') Serial.println(VERSION);
}
