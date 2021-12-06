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

static const char MC6809[] = "MC6809";
static const char HD6309[] = "HD6309";

void Regs::setCpuType() {
    static const uint8_t CHECKS[] = {
            0x10, 0x43, 0xCC,  // 6309: COMD + LDA #0
            0x86, 0x00,        // 6809: NOP + LDD #$8600
            0x97, 0x00, 0xFF,  // STA $00
    };
    uint8_t reg_a;
    Pins.captureWrites(CHECKS, sizeof(CHECKS), nullptr, &reg_a, sizeof(reg_a));
    _cpuType = (reg_a == 0) ? HD6309 : MC6809;
}

const char *Regs::cpu() const {
    return _cpuType;
}

bool Regs::is6309() const {
    return _cpuType == HD6309;
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
        'B', '=', 0, 0,                  // B=49
        0,                               // MC6809/HD6309=51
        'W', '=', 0, 0, 0, 0, ' ',       // W=54
        'V', '=', 0, 0, 0, 0,            // V=61
        0,                               // EOS
    };
    static char cc_bits[] = {
        ' ', 'C', 'C', '=',
        0, 0, 0, 0, 0, 0, 0, 0,          // CC=4
        0,
    };
    outHex8(buffer + 3, dp);
    outHex16(buffer + 9, pc);
    outHex16(buffer + 16, s);
    outHex16(buffer + 23, u);
    outHex16(buffer + 30, y);
    outHex16(buffer + 37, x);
    outHex8(buffer + 44, a);
    outHex8(buffer + 49, b);
    if (is6309()) {
        buffer[51] = ' ';
        outHex8(buffer + 54, e);
        outHex8(buffer + 56, f);
        outHex16(buffer + 61, v);
    } else {
        buffer[51] = 0;
    }
    cli.print(buffer);
    cc_bits[4] = bit1(cc & 0x80, 'E');
    cc_bits[5] = bit1(cc & 0x40, 'F');
    cc_bits[6] = bit1(cc & 0x20, 'H');
    cc_bits[7] = bit1(cc & 0x10, 'I');
    cc_bits[8] = bit1(cc & 0x08, 'N');
    cc_bits[9] = bit1(cc & 0x04, 'Z');
    cc_bits[10] = bit1(cc & 0x02, 'V');
    cc_bits[11] = bit1(cc & 0x01, 'C');
    cli.println(cc_bits);
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
    static uint8_t buffer[16];

    if (show)
        Signals::resetCycles();
    Pins.captureWrites(SWI, sizeof(SWI), &s, buffer, 12);
    // Capturing writes to stack in little endian order.
    pc = le16(buffer) - 1;  //  offset SWI instruction.
    // Injecting PC as SWI vector (with irrelevant read ahead).
    buffer[13] = hi(pc);
    buffer[14] = lo(pc);
    Pins.execInst(buffer + 12, 4);
    if (_cpuType == nullptr)
        setCpuType();
    if (is6309())
        save6309();
    if (show)
        Signals::printCycles();

    s++;
    y = le16(buffer + 4);
    x = le16(buffer + 6);
    dp = buffer[8];
    b = buffer[9];
    a = buffer[10];
    cc = buffer[11];
}

void Regs::restore(bool show) {
    static uint8_t LDS[4] = {0x10, 0xCE,0, 0}; // LDS #sp
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
    if (is6309())
        restore6309();
    Pins.execInst(LDS, sizeof(LDS));
    Pins.execInst(RTI, sizeof(RTI));
    if (show)
        Signals::printCycles();
}

void Regs::save6309() {
    static const uint8_t STW[] = { 0x10, 0x97, 0x00, 0xFF }; // STW $00
    static const uint8_t STV[] = {
        0x1F, 0x70, 0xFF, 0xFF, 0xFF, 0xFF, // TFR V,D
        0xDD, 0x00, 0xFF,                   // STD $00
    };
    static uint8_t buffer[4];
    Pins.captureWrites(STW, sizeof(STW), nullptr, buffer, 2);
    Pins.captureWrites(STV, sizeof(STV), nullptr, buffer + 2, 2);
    e = buffer[0];
    f = buffer[1];
    v = be16(buffer + 2);
}

void Regs::restore6309() {
    static uint8_t LDW_LDV[] = {
        0x10, 0x07, 0, 0,                   // LDW #w
        0xCC, 0, 0,                         // LDD #v
        0x1F, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, // TFR D,V
    };
    LDW_LDV[2] = e;
    LDW_LDV[3] = f;
    LDW_LDV[5] = hi(v);
    LDW_LDV[6] = lo(v);
    Pins.execInst(LDW_LDV, sizeof(LDW_LDV));
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
    if (is6309())
        save6309();
}

void Regs::printRegList() const {
    if (is6309()) {
        cli.println(F("?Reg: pc s u y x a b d e f w v cc DP"));
    } else {
        cli.println(F("?Reg: pc s u y x a b d cc DP"));
    }
}

bool Regs::validUint8Reg(char reg) const {
    if (reg == 'a' || reg == 'b' || reg == 'c' || reg == 'D'
        || (is6309() && (reg == 'e' || reg == 'f'))) {
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
    if (reg == 'p' || reg == 's' || reg == 'u' || reg == 'y' || reg == 'x' || reg == 'd'
        || (is6309() && (reg == 'w' || reg == 'v'))) {
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
    case 'e':
        e = value;
        break;
    case 'f':
        f = value;
        break;
    case 'w':
        setW(value);
        break;
    case 'v':
        v = value;
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
