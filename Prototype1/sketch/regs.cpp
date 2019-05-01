#include <Arduino.h>

#include "hex.h"
#include "pins.h"
#include "regs.h"

union Regs Regs;

static void hex4(uint16_t v, const __FlashStringHelper *name) {
  Serial.print(name);
  printHex4(v, ' ');
}

static void hex2(uint8_t v, const __FlashStringHelper *name) {
  Serial.print(name);
  printHex2(v, ' ');
}

static void execInst4(uint8_t pre, uint8_t inst, uint16_t opr) {
  const uint8_t insn[] = { pre, inst, opr>>8, opr };
  Pins.execInst(insn, sizeof(insn));
}

static void capture1(uint8_t inst, uint8_t *buf, uint8_t max) {
  const uint8_t insn[] = { inst };
  Pins.captureWrites(insn, sizeof(insn), buf, max);
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
  Serial.print(F("EFHINZVC="));
  for (uint8_t m = 0x80; m != 0; m >>= 1) {
    Serial.print((m & cc) ? 1 : 0);
  }
  Serial.println();
}

void Regs::get() {
  save();
  restore();
}

void Regs::save() {
  capture1(0x3F, bytes, 12); // SWI
  pc -= 1; // offset SWI instruction.
  capture2(0x36, 0x40, bytes + 12, 2); // PSHU S
  s += 12; // offset SWI stack frame.
}

void Regs::restore() {
  execInst4(0x10, 0xCE, s - 12); // LDS #(s-12)
  const uint8_t rti[] = {
    0x3B, 0, cc, a, b, dp, x>>8, x, y>>8, y, u>>8, u, pc>>8, pc
  };
  Pins.execInst(rti, sizeof(rti));
}
