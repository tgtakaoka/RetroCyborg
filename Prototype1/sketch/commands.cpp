/*
  The Controller can accept commands represented by one letter.

   R - reset 6309.
   p - print 6309 hardware signal status.
   v - set reset/interrupt vector.
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

#define VERSION F("* Cyborg09 Prototype1 0.7")

class Commands Commands;

static void capture1(uint8_t inst, uint8_t *buf, uint8_t max) {
  const uint8_t insn[] = { inst };
  Pins.captureWrites(insn, sizeof(insn), buf, max);
}

static void capture2(uint8_t inst, uint8_t opr, uint8_t *buf, uint8_t max) {
  const uint8_t insn[] = { inst, opr };
  Pins.captureWrites(insn, sizeof(insn), buf, max);
}

static void execInst2(uint8_t inst, uint8_t opr) {
  const uint8_t insn[] = { inst, opr };
  Pins.execInst(insn, sizeof(insn));
}

static void execInst3(uint8_t inst, uint16_t opr) {
  const uint8_t insn[] = { inst, opr>>8, opr };
  Pins.execInst(insn, sizeof(insn));
}

static void execInst4(uint8_t pre, uint8_t inst, uint16_t opr) {
  const uint8_t insn[] = { pre, inst, opr>>8, opr };
  Pins.execInst(insn, sizeof(insn));
}

static union Regs {
  struct {
    uint16_t pc;
    uint16_t u;
    uint16_t y;
    uint16_t x;
    uint8_t dp;
    uint8_t b;
    uint8_t a;
    uint8_t cc;
    uint16_t s;
  };
  uint8_t bytes[14];

  void print() const {
    hex4(pc, F(" PC="));
    hex4(s,  F(" S="));
    hex4(u,  F(" U="));
    hex4(y,  F(" Y="));
    hex4(x,  F(" X="));
    hex2(dp, F(" DP="));
    hex2(b,  F(" B="));
    hex2(a,  F(" A="));
    Serial.print(F(" EFHINZVC="));
    for (uint8_t m = 0x80; m != 0; m >>= 1) {
      Serial.print((m & cc) ? 1 : 0);
    }
    Serial.println();
  }

  void get() {
    save();
    restore();
  }

  void save() {
    capture1(0x3F, bytes, 12); // SWI
    pc -= 1; // offset SWI instruction.
    capture2(0x36, 0x40, bytes + 12, 2); // PSHU S
    s += 12; // offset SWI stack frame.
  }

  void restore() {
    execInst4(0x10, 0xCE, s - 12); // LDS #(s-12)
    const uint8_t rti[] = {
      0x3B, 0, cc, a, b, dp, x>>8, x, y>>8, y, u>>8, u, pc>>8, pc
    };
    Pins.execInst(rti, sizeof(rti));
  }

private:
  static void hex4(uint16_t v, const __FlashStringHelper *name) {
    Serial.print(name);
    printHex4(v);
  }

  static void hex2(uint8_t v, const __FlashStringHelper *name) {
    Serial.print(name);
    printHex2(v);
  }
} regs;

static void handleInstruction(uint8_t values[], uint8_t len) {
  Pins.execInst(values, len, true /* show */);
}

static void dumpMemory(uint16_t addr, uint16_t len) {
  regs.save();
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
  regs.restore();
}

static void handleDumpMemory(uint8_t values[], uint8_t len) {
  if (len < 3 || len > 4) return;
  const uint16_t num = (len == 3) ? values[2] : toUint16(values + 2);
  dumpMemory(toUint16(values), num);
}

static void memoryWrite(uint16_t addr, const uint8_t *values, uint8_t len) {
  regs.save();
  uint8_t lda_imm[] = { 0x86, 0x00 };
  const uint8_t sta_xpp[] = { 0xA7, 0x80 };
  execInst3(0x8E, addr); // LDX #addr
  for (uint8_t i = 0; i < len; i++) {
    execInst2(0x86, values[i]); // LDA #values[i]
    execInst2(0xA7, 0x80);      // STA ,X+
  }
  regs.restore();
}

static void handleMemoryWrite(uint8_t values[], uint8_t len) {
  if (len < 3) return;
  const uint16_t addr = toUint16(values);
  memoryWrite(addr, values + 2, len -= 2);
  dumpMemory(addr, len);
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
  if (c == 's') {
    Pins.step();
    regs.get();
    regs.print();
    dumpMemory(regs.pc, 6);
  }
  if (c == 'r') {
    regs.get();
    regs.print();
  }
  if (c == 'v') Input.readUint8(c, handleVector);
  if (c == '?') Serial.println(VERSION);
}
