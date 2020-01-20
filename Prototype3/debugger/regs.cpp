/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */

#include "console.h"
#include "pins.h"
#include "regs.h"

#include <Arduino.h>

union Regs Regs;

static void hex4(uint16_t v, const __FlashStringHelper *name) {
  Console.print(name);
  printHex16(v, ' ');
}

static void hex2(uint8_t v, const __FlashStringHelper *name) {
  Console.print(name);
  printHex8(v, ' ');
}

static void bit1(uint8_t v, char name) {
  Console.print(v ? name : '_');
}

static void capture2(uint8_t inst, uint8_t opr, uint8_t *buf, uint8_t max) {
  const uint8_t insn[] = { inst, opr };
  Pins.captureWrites(insn, sizeof(insn), buf, max);
}

void Regs::print() const {
  hex4(pc, F("PC="));
  hex4(s,  F("S="));
  hex4(u,  F("U="));
  hex4(y,  F("Y="));
  hex4(x,  F("X="));
  hex2(dp, F("DP="));
  hex2(b,  F("B="));
  hex2(a,  F("A="));
  Console.print(F("CC="));
  bit1(cc & 0x80, 'E');
  bit1(cc & 0x40, 'F');
  bit1(cc & 0x20, 'H');
  bit1(cc & 0x10, 'I');
  bit1(cc & 0x08, 'N');
  bit1(cc & 0x04, 'Z');
  bit1(cc & 0x02, 'V');
  bit1(cc & 0x01, 'C');
  Console.println();
}

void Regs::get(bool show) {
  save();
  restore();
  if (show) print();
}

void Regs::save() {
  capture2(0x34, 0xFF, bytes, 12); // PSHS PC,U,Y,X,DP,B,A,CC
  pc -= 2; // offset PSHS instruction.
  capture2(0x36, 0x40, bytes + 12, 2); // PSHU S
  s += 12; // offset PSHS stack frame.
}

void Regs::restore() {
  const uint16_t sp = s - 12;
  const uint8_t lds[] = { // LDS #(s-12)
    0x10, 0xCE, (uint8_t)(sp >> 8), (uint8_t)sp
  };
  Pins.execInst(lds, sizeof(lds));
  const uint8_t puls[] = {
    0x35, 0xFF, 0, 0,
    cc, a, b, dp,
    (uint8_t)(x >> 8), (uint8_t)x,
    (uint8_t)(y >> 8), (uint8_t)y,
    (uint8_t)(u >> 8), (uint8_t)u,
    (uint8_t)(pc >> 8), (uint8_t)pc
  };
  Pins.execInst(puls, sizeof(puls));
}
