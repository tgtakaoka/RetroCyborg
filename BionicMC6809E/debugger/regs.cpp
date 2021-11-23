#include "regs.h"

#include "config.h"
#include "digital_fast.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;

struct Regs Regs;
struct Memory Memory;

/**
 * How to determine MC6809 variants.
 *
 *   CLRB
 *   FCB  $10, $43
 *        ; NOP  ($10) on MC6809
 *        ; COMA ($43) on MC6809
 *        ; COMD ($10,$43) on HD6309
 *   B=$00: MC6809
 *   B=$FF: HD6309
 */

const char *Regs::cpu() const {
    return "MC6809";
}

static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void Regs::print() const {
    // clang-format off
    static char buffer[] = {
        'D', 'P', '=', 0, 0, ' ',        // DP=3
        'P', 'C', '=', 0, 0, 0, 0, ' ',  // PC=9
        'S', '=', 0, 0, 0, 0, ' ',       // S=16
        'U', '=', 0, 0, 0, 0, ' ',       // U=23
        'Y', '=', 0, 0, 0, 0, ' ',       // S=30
        'X', '=', 0, 0, 0, 0, ' ',       // X=37
        'A', '=', 0, 0, ' ',             // A=44
        'B', '=', 0, 0, ' ',             // B=49
        'C', 'C', '=',                   // CC=55
        0, 0, 0, 0, 0, 0, 0, 0, 
        0,                               // EOS
    };
    outHex8(buffer + 3, dp);
    outHex16(buffer + 9, pc);
    outHex16(buffer + 16, s);
    outHex16(buffer + 23, u);
    outHex16(buffer + 30, y);
    outHex16(buffer + 37, x);
    outHex8(buffer + 44, a);
    outHex8(buffer + 49, b);
    char *p = buffer + 55;
    *p++ = bit1(cc & 0x80, 'E');
    *p++ = bit1(cc & 0x40, 'F');
    *p++ = bit1(cc & 0x20, 'H');
    *p++ = bit1(cc & 0x10, 'I');
    *p++ = bit1(cc & 0x08, 'N');
    *p++ = bit1(cc & 0x04, 'Z');
    *p++ = bit1(cc & 0x02, 'V');
    *p++ = bit1(cc & 0x01, 'C');
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
    static const uint8_t SWI[3] = {0x3F, 0xFF, 0xFF};  // SWI
    static uint8_t bytes[16];

    if (show)
        Signals::resetCycles();
    Pins.captureWrites(SWI, sizeof(SWI), &s, bytes, 12);
    // Capturing writes to stack in little endian order.
    pc = le16(bytes) - 1;  //  offset SWI instruction.
    // Injecting PC as SWI vector (with irrelevant read ahead).
    bytes[13] = hi(pc);
    bytes[14] = lo(pc);
    Pins.execInst(bytes + 12, 4);
    if (show)
        Signals::printCycles();

    s++;
    y = le16(bytes + 4);
    x = le16(bytes + 6);
    dp = bytes[8];
    b = bytes[9];
    a = bytes[10];
    cc = bytes[11];
}

void Regs::restore(bool show) {
    static uint8_t LDS[4] = {0x10, 0xCE};   // LDS #sp
    static uint8_t RTI[15] = {0x3B, 0xFF};  // RTI
    const uint16_t sp = s - 12;
    LDS[2] = hi(sp);
    LDS[3] = lo(sp);
    RTI[2] = cc;
    RTI[3] = a;
    RTI[4] = b;
    RTI[5] = dp;
    RTI[6] = hi(x);
    RTI[7] = lo(x);
    RTI[8] = hi(y);
    RTI[9] = lo(y);
    RTI[10] = hi(u);
    RTI[11] = lo(u);
    RTI[12] = hi(pc);
    RTI[13] = lo(pc);

    if (show)
        Signals::resetCycles();
    Pins.execInst(LDS, sizeof(LDS));
    Pins.execInst(RTI, sizeof(RTI));
    if (show)
        Signals::printCycles();
}

void Regs::capture(const Signals *stack) {
    s = stack[0].addr + 1;
    pc = uint16(stack[1].data, stack[0].data);
    u = uint16(stack[3].data, stack[2].data);
    y = uint16(stack[5].data, stack[4].data);
    x = uint16(stack[7].data, stack[6].data);
    dp = stack[8].data;
    b = stack[9].data;
    a = stack[10].data;
    cc = stack[11].data;
}

void Regs::printRegList() const {
    cli.println(F("?Reg: pc s u y x a b d cc DP"));
}

bool Regs::validUint8Reg(char reg) const {
    if (reg == 'a' || reg == 'b' || reg == 'c' || reg == 'D') {
        cli.print(reg);
        if (reg == 'c')
            cli.print('c');
        if (reg == 'D')
            cli.print('P');
        return true;
    }
    return false;
}

bool Regs::validUint16Reg(char reg) const {
    if (reg == 'p' || reg == 's' || reg == 'u' || reg == 'y' || reg == 'x' || reg == 'd') {
        cli.print(reg);
        if (reg == 'p')
            cli.print('c');
        return true;
    }
    return false;
}

bool Regs::setRegValue(char reg, uint32_t value, State state) {
    if (state == State::CLI_CANCEL)
        return true;
    if (state == State::CLI_DELETE) {
        cli.backspace(reg == 'p' || reg == 'c' || reg == 'D' ? 3 : 2);
        return false;
    }
    cli.println();
    switch (reg) {
    case 'p':
        pc = value;
        break;
    case 's':
        s = value;
        break;
    case 'u':
        u = value;
        break;
    case 'y':
        y = value;
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
    case 'd':
        setD(value);
        break;
    case 'D':
        dp = value;
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
