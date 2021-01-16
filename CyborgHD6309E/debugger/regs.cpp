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

void Regs::print() const {
    // text=29, hex=(sizeof(bytes)-1)*2, cc=8, eos=1
    char buffer[29 + (sizeof(bytes) - 1) * 2 + 8 + 1];
    char *p = buffer;
    p = outHex16(outText(p, "PC="), pc);
    p = outHex16(outText(p, " S="), s);
    p = outHex16(outText(p, " U="), u);
    p = outHex16(outText(p, " Y="), y);
    p = outHex16(outText(p, " X="), x);
    p = outHex8(outText(p, " DP="), dp);
    p = outHex8(outText(p, " B="), b);
    p = outHex8(outText(p, " A="), a);
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

static const uint8_t pshs_all[] = {0x34, 0xFF};  // PSHS PC,U,Y,X,DP,B,A,CC
static const uint8_t pshu_s[] = {0x36, 0x40};    // PSHU S
void Regs::save() {
    Pins.captureWrites(pshs_all, sizeof(pshs_all), bytes, 12);
    pc -= 2;  // offset PSHS instruction.
    Pins.captureWrites(pshu_s, sizeof(pshu_s), bytes + 12, 2);
    s += 12;  // offset PSHS stack frame.
}

void Regs::restore() {
    const uint16_t sp = s - 12;
    const uint8_t lds[] = {
            0x10, 0xCE, (uint8_t)(sp >> 8), (uint8_t)sp};  // LDS #(s-12)
    Pins.execInst(lds, sizeof(lds));
    const uint8_t puls_all[] = {0x35, 0xFF, cc, a, b, dp, (uint8_t)(x >> 8),
            (uint8_t)x, (uint8_t)(y >> 8), (uint8_t)y, (uint8_t)(u >> 8),
            (uint8_t)u, (uint8_t)(pc >> 8), (uint8_t)pc};
    Pins.execInst(puls_all, sizeof(puls_all));
}
