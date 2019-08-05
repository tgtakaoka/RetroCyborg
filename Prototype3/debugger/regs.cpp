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
    Serial.print((m & cc) ? '1' : '0');
  }
  Serial.println();
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
    0x10, 0xCE, sp >> 8, sp
  };
  Pins.execInst(lds, sizeof(lds));
  const uint8_t puls[] = {
    0x35, 0xFF, 0, 0, cc, a, b, dp, x >> 8, x, y >> 8, y, u >> 8, u, pc >> 8, pc
  };
  Pins.execInst(puls, sizeof(puls));
}
