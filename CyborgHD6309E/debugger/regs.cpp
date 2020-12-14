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

static void capture2(uint8_t inst, uint8_t opr, uint8_t *buf, uint8_t max) {
    const uint8_t insn[] = {inst, opr};
    Pins.captureWrites(insn, sizeof(insn), buf, max);
}

void Regs::print() const {
    // text=29, hex=(sizeof(bytes)-1)*2, cc=8, eos=1
    char buffer[29 + (sizeof(bytes) - 1) * 2 + 8 + 1];
    char *p = buffer;
    p = outText(p, "PC=");
    p = outHex16(p, pc);
    p = outText(p, " S=");
    p = outHex16(p, s);
    p = outText(p, " U=");
    p = outHex16(p, u);
    p = outText(p, " Y=");
    p = outHex16(p, y);
    p = outText(p, " X=");
    p = outHex16(p, x);
    p = outText(p, " DP=");
    p = outHex8(p, dp);
    p = outText(p, " B=");
    p = outHex8(p, b);
    p = outText(p, " A=");
    p = outHex8(p, a);
    p = outText(p, " CC=");
    *p++ = bit1(cc & 0x80, 'E');
    *p++ = bit1(cc & 0x40, 'F');
    *p++ = bit1(cc & 0x20, 'H');
    *p++ = bit1(cc & 0x10, 'I');
    *p++ = bit1(cc & 0x08, 'N');
    *p++ = bit1(cc & 0x04, 'Z');
    *p++ = bit1(cc & 0x02, 'V');
    *p++ = bit1(cc & 0x01, 'C');
    *p = 0;
    Cli.println(buffer);
}

void Regs::get(bool show) {
    save();
    restore();
    if (show)
        print();
}

void Regs::save() {
    capture2(0x34, 0xFF, bytes, 12);      // PSHS PC,U,Y,X,DP,B,A,CC
    pc -= 2;                              // offset PSHS instruction.
    capture2(0x36, 0x40, bytes + 12, 2);  // PSHU S
    s += 12;                              // offset PSHS stack frame.
}

void Regs::restore() {
    const uint16_t sp = s - 12;
    const uint8_t lds[] = {// LDS #(s-12)
            0x10, 0xCE, (uint8_t)(sp >> 8), (uint8_t)sp};
    Pins.execInst(lds, sizeof(lds));
    const uint8_t puls[] = {0x35, 0xFF, 0, 0, cc, a, b, dp, (uint8_t)(x >> 8),
            (uint8_t)x, (uint8_t)(y >> 8), (uint8_t)y, (uint8_t)(u >> 8),
            (uint8_t)u, (uint8_t)(pc >> 8), (uint8_t)pc};
    Pins.execInst(puls, sizeof(puls));
}
