/*
  The Controller can accept commands represented by one letter.

   R - reset 6309.
   p - print 6309 hardware signal status.
   i - execute one instruction, input hex numbers.
   d - dump memory. AH AL [LH] LL
   m - write memory. AH AL D0 [D1...]
   s - step one instruction.
   r - print 6309 registers.
   ? - print version.
*/

#include <Arduino.h>

#include "commands.h"
#include "hex.h"
#include "input.h"
#include "pins.h"
#include "regs.h"

#define VERSION F("* Cyborg09 Prototype1 0.7")

class Commands Commands;

static void execInst2(uint8_t inst, uint8_t opr) {
  const uint8_t insn[] = { inst, opr };
  Pins.execInst(insn, sizeof(insn));
}

static void execInst3(uint8_t inst, uint16_t opr) {
  const uint8_t insn[] = { inst, opr>>8, opr };
  Pins.execInst(insn, sizeof(insn));
}

static void handleInstruction(uint8_t values[], uint8_t len) {
  Pins.execInst(values, len, true /* show */);
}

static void dumpMemory(uint16_t addr, uint16_t len) {
  Regs.save();
  execInst3(0x8E, addr); // LDX #addr
  for (uint16_t i = 0; i < len; i++) {
    execInst2(0xA6, 0x80); // LDA ,X+
    if (i % 16 == 0) {
      if (i) Serial.println();
      printHex4(addr + i);
      Serial.print(':');
    }
    printHex2(Pins.dbus());
    Serial.print(' ');
  }
  Serial.println();
  Regs.restore();
}

static void handleDumpMemory(uint8_t values[], uint8_t len) {
  if (len < 3 || len > 4) return;
  const uint16_t num = (len == 3) ? values[2] : toUint16(values + 2);
  dumpMemory(toUint16(values), num);
}

static void memoryWrite(uint16_t addr, const uint8_t *values, uint8_t len) {
  Regs.save();
  uint8_t lda_imm[] = { 0x86, 0x00 };
  const uint8_t sta_xpp[] = { 0xA7, 0x80 };
  execInst3(0x8E, addr); // LDX #addr
  for (uint8_t i = 0; i < len; i++) {
    execInst2(0x86, values[i]); // LDA #values[i]
    execInst2(0xA7, 0x80);      // STA ,X+
  }
  Regs.restore();
}

static void handleMemoryWrite(uint8_t values[], uint8_t len) {
  if (len < 3) return;
  const uint16_t addr = toUint16(values);
  memoryWrite(addr, values + 2, len -= 2);
  dumpMemory(addr, len);
}

void Commands::loop() {
  const char c = Serial.read();
  if (c == -1) return;
  if (c == 'p') Pins.print();
  if (c == 'R') Pins.reset();
  if (c == 'i') Input.readUint8(c, handleInstruction);
  if (c == 'd') Input.readUint8(c, handleDumpMemory);
  if (c == 'm') Input.readUint8(c, handleMemoryWrite);
  if (c == 's') {
    Pins.step();
    Regs.get();
    Regs.print();
    dumpMemory(Regs.pc, 6);
  }
  if (c == 'r') {
    Regs.get();
    Regs.print();
  }
  if (c == '?') Serial.println(VERSION);
}
