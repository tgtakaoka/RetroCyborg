/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */

#include "regs.h"

#include <libcli.h>
#include "pins.h"
#include "string_util.h"

extern libcli::Cli Cli;

union Regs Regs;
static const char *cpu_type = nullptr;
static const char MC6809[] = "6809";
static const char HD6309[] = "6309";
static const uint8_t clrb[] = {0x5F};
static const uint8_t comd[] = {0x10, 0x43};
static const uint8_t stb[] = {0xD7, 0x00};

static const char *checkCpu() {
    Pins.execInst(clrb, sizeof(clrb));
    // 6309:COMD, 6809:NOP+COMA
    Pins.execInst(comd, sizeof(comd));
    // 6309:B=0xFF, 6809:B=0
    uint8_t b;
    Pins.captureWrites(stb, sizeof(stb), &b, 1);
    return b ? HD6309 : MC6809;
}

bool Regs::is6309() const {
    return cpu_type == HD6309;
}

const char *Regs::cpu() const {
    return cpu_type ? cpu_type : MC6809;
}

static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void Regs::print() const {
    // text=35, hex=(sizeof(bytes)-1)*2, cc=8, eos=1
    char buffer[35 + (sizeof(bytes) - 1) * 2 + 8 + 1];
    char *p = buffer;
    p = outHex16(outText(p, F("PC=")), pc);
    p = outHex16(outText(p, F(" S=")), s);
    p = outHex16(outText(p, F(" U=")), u);
    p = outHex16(outText(p, F(" Y=")), y);
    p = outHex16(outText(p, F(" X=")), x);
    p = outHex8(outText(p, F(" B=")), b);
    p = outHex8(outText(p, F(" A=")), a);
    if (is6309()) {
        p = outHex16(outText(p, F(" W=")), w);
        p = outHex16(outText(p, F(" V=")), v);
    }
    p = outHex8(outText(p, F(" DP=")), dp);
    p = outText(p, F(" CC="));
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
static const uint8_t tfr_wy[] = {0x1F, 0x62};    // TFR W,Y
static const uint8_t tfr_vx[] = {0x1F, 0x71};    // TFR V,X
static const uint8_t pshu_yx[] = {0x36, 0x30};   // PSHU Y,X
void Regs::save() {
    Pins.captureWrites(pshs_all, sizeof(pshs_all), bytes, 12);
    pc -= 2;  // offset PSHS instruction.
    Pins.captureWrites(pshu_s, sizeof(pshu_s), bytes + 12, 2);
    s += 12;  // offset PSHS stack frame.
    if (cpu_type == nullptr)
        cpu_type = checkCpu();
    if (is6309()) {
        Pins.execInst(tfr_wy, sizeof(tfr_wy));
        Pins.execInst(tfr_vx, sizeof(tfr_vx));
        Pins.captureWrites(pshu_yx, sizeof(pshu_yx), bytes + 14, 4);
    }
}

static const uint8_t tfr_dv[] = {0x1F, 0x07};  // TFR D,V
void Regs::restore() {
    if (is6309()) {
        const uint8_t ldd[] = {0xCC, (uint8_t)(v >> 8), (uint8_t)v};  // LDD #v
        Pins.execInst(ldd, sizeof(ldd));
        Pins.execInst(tfr_dv, sizeof(tfr_dv));
        const uint8_t ldw[] = {
                0x10, 0x86, (uint8_t)(w >> 8), (uint8_t)w};  // LDW #w
        Pins.execInst(ldw, sizeof(ldw));
    }
    const uint16_t sp = s - 12;
    const uint8_t lds[] = {
            0x10, 0xCE, (uint8_t)(sp >> 8), (uint8_t)sp};  // LDS #(s-12)
    Pins.execInst(lds, sizeof(lds));
    const uint8_t puls_all[] = {0x35, 0xFF, cc, a, b, dp, (uint8_t)(x >> 8),
            (uint8_t)x, (uint8_t)(y >> 8), (uint8_t)y, (uint8_t)(u >> 8),
            (uint8_t)u, (uint8_t)(pc >> 8), (uint8_t)pc};
    Pins.execInst(puls_all, sizeof(puls_all));
}
