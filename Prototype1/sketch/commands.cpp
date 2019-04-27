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
#include "hex.h"
#include "input.h"
#include "pins.h"

#define VERSION F("* Cyborg09 Prototype1 0.4")

class Commands Commands;

static void handleInstruction(uint8_t values[], uint8_t len) {
  Pins.execInst(values, len);
}

static void handleVector(uint8_t values[], uint8_t len) {
  if (len != 2) return;
  Pins.setVector(toUint16(values));
}

void Commands::loop() {
  const char c = Serial.read();
  if (c == -1) return;
  if (c == 's') Pins.print();
  if (c == 'R') Pins.reset();
  if (c == 'i') Input.readUint8(c, handleInstruction);
  if (c == 'v') Input.readUint8(c, handleVector);
  if (c == '?') Serial.println(VERSION);
}
