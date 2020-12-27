/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */

#include "regs.h"

#include <libcli.h>
#include "pins.h"
#include "string_util.h"

union Regs Regs;

#if 0
static char bit1(uint8_t v, char name) {
  return v ? name : '_';
}

static void capture2(uint8_t inst, uint8_t opr, uint8_t *buf, uint8_t max) {
  const uint8_t insn[] = { inst, opr };
  Pins.captureWrites(insn, sizeof(insn), buf, max);
}

static void capture1(uint8_t inst, uint8_t *buf, uint8_t max) {
  Pins.captureWrites(&inst, sizeof(inst), buf, max);
}

void Regs::print() const {
  char buffer[];
  char *p = buffer;
  p = outHex16(outText(p, "PC="),  pc);
  p = outHex16(outText(p, " BC="), bc);
  p = outHex16(outText(p, " DE="), de);
  p = outHex16(outText(p, " HL="), hl);
  p = outHex16(outText(p, " IX="), ix);
  p = outHex16(outText(p, " IY="), iy);
  p = outHex16(outText(p, " SP="), sp);
  p = outHex8(outText(p, " A="), a);
  p = outText(p, " F=");
  // F=SZ_H_VNC
  *p++ = bit1(f & 0x80, 'S');
  *p++ = bit1(f & 0x40, 'Z');
  *p++ = bit1(f & 0x10, 'H');
  *p++ = bit1(f & 0x04, 'V');
  *p++ = bit1(f & 0x02, 'N');
  *p++ = bit1(f & 0x01, 'C');
  Console.println(buffer);
}

void Regs::get(bool show) {
  save();
  restore();
  if (show) print();
}

void Regs::save() {
  capture4(0xED, 0x73, 0x00, 0x00, &sp, sizeof(sp)); // LD (0),SP
  capture1(0xC7, &pc, sizeof(pc));                   // RST 00H
  pc -= 5; // offset LD and RST
  capture1(0xE5, &hl, sizeof(hl));                   // PUSH HL
  capture1(0xC5, &bc, sizeof(bc));                   // PUSH BC
  capture1(0xD5, &de, sizeof(de));                   // PUSH DE
  capture1(0xF5, &af, sizeof(af));                   // PUSH AF
  capture2(0xDD, 0xE5, &ix, sizeof(ix));             // PUSH IX
  capture2(0xFD, 0xE5, &iy, sizeof(iy));             // PUSH IY
}

void Regs::restore() {
  execute3(0xF1, a, f);                            // POP AF
  execute3(0xC1, (uint8_t)bc, (uint8_t)(bc >> 8)); // POP BC
  execute3(0xD1, (uint8_t)de, (uint8_t)(de >> 8)); // POP DE
  execute4(0xDD, 0xE1, (uint8_t)ix, (uint8_t)(ix >> 8)); // POP IX
  execute4(0xFD, 0xE1, (uint8_t)iy, (uint8_t)(iy >> 8)); // POP IY
  sp -= 6; // offset POPs and RET
  execute3(0xE1, (uint8_t)sp, (uint8_t)(sp >> 8)); // POP HL
  execute1(0xF9);                                  // LD SP,HL
  execute3(0xE1, (uint8_t)hl, (uint8_t)(hl >> 8)); // POP HL
  execute3(0xC9, (uint8_t)pc, (uint8_t)(pc >> 8)); // RET
}

#endif
