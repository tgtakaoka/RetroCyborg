/*
  The Controller can accept commands represented by one letter.

   R - reset 6309.
   p - print 6309 hardware signal status.
   v - set reset/interrupt vector.
   i - execute one instruction, input hex numbers.
   d - dump memory. AH AL [LH] LL
   m - write memory. AH AL D0 [D1...]
   s - step one instruction.
   ? - print version.
*/

#include <Arduino.h>

#include "commands.h"
#include "hex.h"
#include "input.h"
#include "pins.h"

#define VERSION F("* Cyborg09 Prototype1 0.6")

class Commands Commands;

static void handleInstruction(uint8_t values[], uint8_t len) {
  Pins.execInst(values, len, true /* show */);
}

static void handleDumpMemory(uint8_t values[], uint8_t len) {
  if (len < 3 || len > 4) return;
  const uint16_t addr = toUint16(values);
  const uint16_t num = (len == 3) ? values[2] : toUint16(values + 2);
  const uint8_t ldx_imm[] = { 0x8E, addr >> 8, addr };
  const uint8_t lda_xpp[] = { 0xA6, 0x80 };
  Pins.execInst(ldx_imm, sizeof(ldx_imm));
  for (uint16_t i = 0; i < num; i++) {
    Pins.execInst(lda_xpp, sizeof(lda_xpp));
    if (i % 16 == 0) {
      if (i) Serial.println();
      printHex4(addr + i);
      Serial.print(':');
    }
    printHex2(Pins.dbus());
    Serial.print(' ');
  }
  Serial.println();
}

static void handleMemoryWrite(uint8_t values[], uint8_t len) {
  if (len < 3) return;
  const uint16_t addr = toUint16(values);
  const uint8_t ldx_imm[] = { 0x8E, addr >> 8, addr };
  uint8_t lda_imm[] = { 0x86, 0x00 };
  const uint8_t sta_xpp[] = { 0xA7, 0x80 };
  Pins.execInst(ldx_imm, sizeof(ldx_imm));
  for (uint8_t i = 2; i < len; i++) {
    lda_imm[1] = values[i];
    Pins.execInst(lda_imm, sizeof(lda_imm));
    Pins.execInst(sta_xpp, sizeof(sta_xpp));
  }
  values[2] = len - 2;
  handleDumpMemory(values, 3);
}

static void handleVector(uint8_t values[], uint8_t len) {
  if (len != 2) return;
  Pins.setVector(toUint16(values));
}

void Commands::loop() {
  const char c = Serial.read();
  if (c == -1) return;
  if (c == 'p') Pins.print();
  if (c == 'R') Pins.reset();
  if (c == 'i') Input.readUint8(c, handleInstruction);
  if (c == 'd') Input.readUint8(c, handleDumpMemory);
  if (c == 'm') Input.readUint8(c, handleMemoryWrite);
  if (c == 's') Pins.step(true /* show */);
  if (c == 'v') Input.readUint8(c, handleVector);
  if (c == '?') Serial.println(VERSION);
}
