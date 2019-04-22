/*
  The Controller can accept commands represented by one letter.

   R - reset 6309.
   s - print 6309 hardware signal status.
   v - set reset/interrupt vector.
   i - execute one instruction, input hex numbers.
   ? - print version.
*/

#include <Arduino.h>

#include "commands.h"
#include "input.h"
#include "pins.h"

#define VERSION F("* Cyborg09 Prototype1 0.4")

class Commands Commands;

static void handleInstruction(char command, uint8_t values[], uint8_t num) {
  struct Pins::Status pins;
  Pins.halt(HIGH);
  do {
    Pins.cycle(pins);
    pins.print();
    Pins.halt(LOW);
  } while (!pins.running() || !pins.lastInstCycle());

  for (uint8_t i = 0; i < num; i++) {
    Pins.setData(values[i]);
    Pins.cycle(pins);
    pins.print(false);
    Serial.println('I');
  }
  while (!pins.lastInstCycle()) {
    Pins.cycle(pins);
    pins.print();
  }
}

static void handleVector(char command, uint8_t values[], uint8_t num) {
  Pins.setVector(values[0], values[1]);
}

static void handleReset() {
  Pins::Status pins;
  Pins.reset(LOW);
  Pins.cycle(pins);
  pins.print();
  Pins.reset(HIGH);
  Pins.cycle(pins);
  pins.print();
  Pins.halt(HIGH);
  Pins.cycle(pins);
  pins.print();
  Pins.halt(LOW);
  do {
    Pins.cycle(pins);
    pins.print();
  } while (!pins.inHalt());
}

void Commands::loop() {
  const char c = Serial.read();
  if (c == -1) return;
  if (c == 's') Pins.print();
  if (c == 'R') handleReset();
  if (c == 'h') Pins.halt(LOW);
  if (c == 'H') Pins.halt(HIGH);
  if (c == 'i') Input.readHex(c, handleInstruction);
  if (c == 'v') Input.readHex(c, handleVector);
  if (c == '?') Serial.println(VERSION);
}
