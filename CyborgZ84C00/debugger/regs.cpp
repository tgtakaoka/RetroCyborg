/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */

#include "regs.h"

#include <libcli.h>
#include "pins.h"
#include "string_util.h"

extern libcli::Cli Cli;

union Regs Regs;

static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

static void capture4(uint8_t prefix, uint8_t inst, uint16_t opr, uint16_t *buf,
        uint8_t max) {
    const uint8_t insn[] = {prefix, inst, (uint8_t)opr, (uint8_t)(opr >> 8)};
    Pins.captureWrites(
            insn, sizeof(insn), reinterpret_cast<uint8_t *>(buf), max);
}

static void capture2(uint8_t prefix, uint8_t inst, uint16_t *buf, uint8_t max) {
    const uint8_t insn[] = {prefix, inst};
    Pins.captureWrites(
            insn, sizeof(insn), reinterpret_cast<uint8_t *>(buf), max);
}

static void capture1(uint8_t inst, uint16_t *buf, uint8_t max) {
    Pins.captureWrites(
            &inst, sizeof(inst), reinterpret_cast<uint8_t *>(buf), max);
}

static void execute4(uint8_t prefix, uint8_t inst, uint16_t opr) {
    const uint8_t insn[] = {prefix, inst, (uint8_t)opr, (uint8_t)(opr >> 8)};
    Pins.execInst(insn, sizeof(insn));
}

static void execute3(uint8_t inst, uint16_t opr) {
    const uint8_t insn[] = {inst, (uint8_t)opr, (uint8_t)(opr >> 8)};
    Pins.execInst(insn, sizeof(insn));
}

static void execute1(uint8_t inst) {
    const uint8_t insn[] = {inst};
    Pins.execInst(insn, sizeof(insn));
}

void Regs::print() const {
    // text=33, hex=(sizeof(bytes)-1)*2, f=6, eos=1
    char buffer[33 + (sizeof(Regs) - 1) * 2 + 6 + 1];
    char *p = buffer;
    p = outHex16(outText(p, "PC="), pc);
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
    Cli.println(buffer);
}

void Regs::get(bool show) {
    save();
    restore();
    if (show)
        print();
}

void Regs::save() {
    capture4(0xED, 0x73, 0x0000, &sp, sizeof(sp));  // LD (0),SP
    capture1(0xC7, &pc, sizeof(pc));                // RST 00H
    pc -= 5;                                        // offset LD and RST
    capture1(0xE5, &hl, sizeof(hl));                // PUSH HL
    capture1(0xC5, &bc, sizeof(bc));                // PUSH BC
    capture1(0xD5, &de, sizeof(de));                // PUSH DE
    capture1(0xF5, &af, sizeof(af));                // PUSH AF
    capture2(0xDD, 0xE5, &ix, sizeof(ix));          // PUSH IX
    capture2(0xFD, 0xE5, &iy, sizeof(iy));          // PUSH IY
}

void Regs::restore() {
    execute3(0xF1, af);        // POP AF
    execute3(0xC1, bc);        // POP BC
    execute3(0xD1, de);        // POP DE
    execute4(0xDD, 0xE1, ix);  // POP IX
    execute4(0xFD, 0xE1, iy);  // POP IY
    sp -= 6;                   // offset POPs and RET
    execute3(0xE1, sp);        // POP HL
    execute1(0xF9);            // LD SP,HL
    execute3(0xE1, hl);        // POP HL
    execute3(0xC9, pc);        // RET
}
