#include "regs.h"

#include "config.h"
#include "digital_fast.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;

struct Regs Regs;
struct Memory Memory;

/**
 * How to determine MC6800 variants.
 *
 * MC6800/MC6801/HD6301
 *   LDX  #$55AA
 *   LDAB #100
 *   LDAA #5
 *   ADDA #5
 *   FCB  $18
 *        ; DAA  ($19) on MC6800
 *        ; ABA  ($1B) on MC6801/MC6803
 *        ; XGDX ($18) on HD6301/MC6803
 *   A=$10: MC6800
 *   A=110: MC6801/MC6803
 *   A=$55: HD6301/MC6803
 *
 * MC6800/MB8861(MB8870)
 *   LDX  #$FFFF
 *   FCB  $EC, $01
 *        ; CPX 1,X ($AC $01) on MC6800
 *        ; ADX 1   ($EC $01) on MB8861
 * X=$FFFF: MC6800
 * X=$0000: MB8861
 */

const char *Regs::cpu() const {
    return "6801";
}

static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void Regs::print() const {
    // text=35, reg=8*2, cc=8, eos=1
    char buffer[20 + 8 * 2 + 8 + 1];
    char *p = buffer;
    p = outHex16(outText(p, "PC="), pc);
    p = outHex16(outText(p, " SP="), sp);
    p = outHex16(outText(p, " X="), x);
    p = outHex8(outText(p, " A="), a);
    p = outHex8(outText(p, " B="), b);
    p = outText(p, " CC=");
    *p++ = bit1(cc & 0x20, 'H');
    *p++ = bit1(cc & 0x10, 'I');
    *p++ = bit1(cc & 0x08, 'N');
    *p++ = bit1(cc & 0x04, 'Z');
    *p++ = bit1(cc & 0x02, 'V');
    *p++ = bit1(cc & 0x01, 'C');
    *p = 0;
    cli.println(buffer);
    Pins.idle();
}

static constexpr uint16_t uint16(const uint8_t hi, const uint8_t lo) {
    return (static_cast<uint16_t>(hi) << 8) | lo;
}
static constexpr uint16_t le16(const uint8_t *p) {
    return uint16(p[1], p[0]);
}
static constexpr uint16_t be16(const uint8_t *p) {
    return uint16(p[0], p[1]);
}
static constexpr uint8_t hi(const uint16_t v) {
    return static_cast<uint8_t>(v >> 8);
}
static constexpr uint8_t lo(const uint16_t v) {
    return static_cast<uint8_t>(v);
}

void Regs::save(bool show) {
    static const uint8_t SWI[2] = {0x3F, 0xFF};  // SWI
    static uint8_t bytes[10];

    if (show)
        Signals::resetCycles();
    Pins.captureWrites(SWI, sizeof(SWI), &sp, bytes, 7);
    // Capturing writes to stack in little endian order.
    pc = le16(bytes) - 1;  //  offset SWI instruction.
    // Injecting PC as SWI vector (with irrelevant read ahead).
    bytes[7] = 0;
    bytes[8] = hi(pc);
    bytes[9] = lo(pc);
    Pins.execInst(bytes + 7, 3);
    if (show)
        Signals::printCycles();

    x = le16(bytes + 2);
    a = bytes[4];
    b = bytes[5];
    cc = bytes[6];
}

void Regs::restore(bool show) {
    static uint8_t LDS[3] = {0x8E};               // LDS #sp
    static uint8_t RTI[10] = {0x3B, 0xFF, 0xFF};  // RTI
    const uint16_t s = sp - 7;
    LDS[1] = hi(s);
    LDS[2] = lo(s);
    RTI[3] = cc;
    RTI[4] = b;
    RTI[5] = a;
    RTI[6] = hi(x);
    RTI[7] = lo(x);
    RTI[8] = hi(pc);
    RTI[9] = lo(pc);

    if (show)
        Signals::resetCycles();
    Pins.execInst(LDS, sizeof(LDS));
    Pins.execInst(RTI, sizeof(RTI));
    if (show)
        Signals::printCycles();
}

void Regs::capture(const Signals *stack) {
    sp = stack[0].addr;
    pc = uint16(stack[1].data, stack[0].data);
    x = uint16(stack[3].data, stack[2].data);
    a = stack[4].data;
    b = stack[5].data;
    cc = stack[6].data;
}

void Regs::printRegList() const {
    cli.println(F("?Reg: pc sp x a b cc"));
}

bool Regs::validUint8Reg(char reg) const {
    if (reg == 'a' || reg == 'b' || reg == 'c') {
        cli.print(reg);
        if (reg == 'c')
            cli.print('c');
        return true;
    }
    return false;
}

bool Regs::validUint16Reg(char reg) const {
    if (reg == 'p' || reg == 's' || reg == 'x') {
        cli.print(reg);
        if (reg == 'p')
            cli.print('c');
        if (reg == 's')
            cli.print('p');
        return true;
    }
    return false;
}

bool Regs::setRegValue(char reg, uint32_t value, State state) {
    if (state == State::CLI_CANCEL)
        return true;
    if (state == State::CLI_DELETE) {
        cli.backspace(reg == 'p' || reg == 's' || reg == 'c' ? 3 : 2);
        return false;
    }
    cli.println();
    switch (reg) {
    case 'p':
        pc = value;
        break;
    case 's':
        sp = value;
        break;
    case 'x':
        x = value;
        break;
    case 'a':
        a = value;
        break;
    case 'b':
        b = value;
        break;
    case 'c':
        cc = value;
        break;
    }
    print();
    return true;
}

bool Memory::is_internal(uint16_t addr) {
    return false;  // External Memory Space
}

uint8_t Memory::read(uint16_t addr) const {
    return raw_read(addr);
}

void Memory::write(uint16_t addr, uint8_t data) {
    raw_write(addr, data);
}

uint8_t Memory::raw_read(uint16_t addr) const {
    return memory[addr];
}

void Memory::raw_write(uint16_t addr, uint8_t data) {
    memory[addr] = data;
}

uint16_t Memory::raw_read_uint16(uint16_t addr) const {
    return (static_cast<uint16_t>(raw_read(addr)) << 8) | raw_read(addr + 1);
}

void Memory::raw_write_uint16(uint16_t addr, uint16_t data) {
    raw_write(addr, data >> 8);
    raw_write(addr + 1, data);
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
