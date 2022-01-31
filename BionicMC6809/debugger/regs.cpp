#include "regs.h"

#include <libcli.h>

#include <asm_mc6809.h>
#include <dis_mc6809.h>
#include "config.h"
#include "mc6850.h"
#include "digital_fast.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;
extern Mc6850 Acia;

libasm::mc6809::AsmMc6809 asm6809;
libasm::mc6809::DisMc6809 dis6809;
libasm::Assembler &assembler(asm6809);
libasm::Disassembler &disassembler(dis6809);

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
            0x5F, 0xFF,        // CLRB
            0x10, 0x43, 0xFF,  // 6309: COMD, 6809: NOP
            0xD7, 0x00, 0xFF,  // STB $00
    };
    uint8_t reg_b;
    Pins.captureWrites(CHECKS, sizeof(CHECKS), nullptr, &reg_b, sizeof(reg_b));
    _cpuType = (reg_b == 0) ? MC6809 : HD6309;
}

const char *Regs::cpu() const {
    return _cpuType ? _cpuType : MC6809;
}

const char *Regs::cpuName() const {
    return cpu();
}

void Regs::reset() {
    _cpuType = nullptr;
    _native6309 = false;
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
        0,                               // Native mode=65
        'N', 0,
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
        buffer[65] = native6309() ? ' ' : 0;
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
    static uint8_t buffer[18];

    if (show)
        Signals::resetCycles();
    const uint8_t regs = native6309() ? 14 : 12;
    Pins.captureWrites(SWI, sizeof(SWI), &s, buffer, regs);
    // Capturing writes to stack in little endian order.
    pc = le16(buffer) - 1;  //  offset SWI instruction.
    // Injecting PC as SWI vector (with irrelevant read ahead).
    buffer[15] = hi(pc);
    buffer[16] = lo(pc);
    Pins.execInst(buffer + 14, 4);
    if (_cpuType == nullptr)
        setCpuType();
    if (is6309())
        save6309();
    if (show)
        Signals::printCycles();

    s++;
    u = le16(buffer + 2);
    y = le16(buffer + 4);
    x = le16(buffer + 6);
    dp = buffer[8];
    const uint8_t *p = &buffer[9];
    if (native6309()) {
        f = *p++;
        e = *p++;
    }
    b = *p++;
    a = *p++;
    cc = *p;
}

void Regs::restore(bool show) {
    static uint8_t LDS[4] = {0x10, 0xCE,0, 0}; // LDS #sp
    const uint16_t sp = s - (native6309() ? 14 : 12);
    LDS[2] = hi(sp);
    LDS[3] = lo(sp);

    static uint8_t RTI[17] = {0x3B, 0xFF};  // RTI
    uint8_t *p = &RTI[2];
    *p++ = cc;
    *p++ = a;
    *p++ = b;
    if (native6309()) {
        *p++ = e;
        *p++ = f;
    }
    *p++ = dp;
    *p++ = hi(x);
    *p++ = lo(x);
    *p++ = hi(y);
    *p++ = lo(y);
    *p++ = hi(u);
    *p++ = lo(u);
    *p++ = hi(pc);
    *p = lo(pc);

    if (show)
        Signals::resetCycles();
    if (is6309())
        restore6309();
    Pins.execInst(LDS, sizeof(LDS));
    Pins.execInst(RTI, native6309() ? 17 : 15);
    if (show)
        Signals::printCycles();
}

void Regs::save6309() {
    static uint8_t buffer[2];

    if (native6309()) {
        static const uint8_t STV[] = {
            0x1F, 0x70, 0xFF, 0xFF, // TFR V,D
            0xDD, 0x00,             // STD $00
        };
        Pins.captureWrites(STV, sizeof(STV), nullptr, buffer, 2);
        v = be16(buffer);
        return;
    }

    static const uint8_t STV[] = {
        0x1F, 0x70, 0xFF, 0xFF, 0xFF, 0xFF, // TFR V,D
        0xDD, 0x00, 0xFF,                   // STD $00
    };
    Pins.captureWrites(STV, sizeof(STV), nullptr, buffer, 2);
    v = be16(buffer);
    static const uint8_t STW[] = { 0x10, 0x97, 0x00, 0xFF }; // STW $00
    Pins.captureWrites(STW, sizeof(STW), nullptr, buffer, 2);
    e = buffer[0];
    f = buffer[1];
}

void Regs::restore6309() {
    if (native6309()) {
        static uint8_t LDV[] = {
            0xCC, 0, 0,             // LDD #v
            0x1F, 0x07, 0xFF, 0xFF, // TFR D,V
        };
        LDV[1] = hi(v);
        LDV[2] = lo(v);
        Pins.execInst(LDV, sizeof(LDV));
        return;
    }

    static uint8_t LDW_LDV[] = {
        0x10, 0x86, 0, 0,                   // LDW #w
        0xCC, 0, 0,                         // LDD #v
        0x1F, 0x07, 0xFF, 0xFF, 0xFF, 0xFF, // TFR D,V
    };
    LDW_LDV[2] = e;
    LDW_LDV[3] = f;
    LDW_LDV[5] = hi(v);
    LDW_LDV[6] = lo(v);
    Pins.execInst(LDW_LDV, sizeof(LDW_LDV));
}

void Regs::capture(const Signals *stack, bool native6309) {
    _native6309 = native6309;
    s = stack[0].addr + 1;
    pc = uint16(stack[1].data, stack[0].data);
    u = uint16(stack[3].data, stack[2].data);
    y = uint16(stack[5].data, stack[4].data);
    x = uint16(stack[7].data, stack[6].data);
    dp = stack[8].data;
    stack += 9;
    if (native6309) {
        f = (stack++)->data;
        e = (stack++)->data;
    }
    b = (stack++)->data;
    a = (stack++)->data;
    cc = stack->data;
    if (is6309())
        save6309();
}

void Regs::printRegList() const {
    if (is6309()) {
        cli.println(F("?Reg: PC S U Y X A B E F D W Q V CC DP"));
    } else {
        cli.println(F("?Reg: PC S U Y X A B D CC DP"));
    }
}

char Regs::validUint8Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("A")) == 0)
        return 'a';
    if (strcasecmp_P(word, PSTR("B")) == 0)
        return 'b';
    if (strcasecmp_P(word, PSTR("CC")) == 0)
        return 'c';
    if (strcasecmp_P(word, PSTR("DP")) == 0)
        return 'D';
    if (is6309()) {
        if (strcasecmp_P(word, PSTR("E")) == 0)
            return 'e';
        if (strcasecmp_P(word, PSTR("F")) == 0)
            return 'f';
    }
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("PC")) == 0)
        return 'p';
    if (strcasecmp_P(word, PSTR("S")) == 0)
        return 's';
    if (strcasecmp_P(word, PSTR("U")) == 0)
        return 'u';
    if (strcasecmp_P(word, PSTR("Y")) == 0)
        return 'y';
    if (strcasecmp_P(word, PSTR("X")) == 0)
        return 'x';
    if (strcasecmp_P(word, PSTR("D")) == 0)
        return 'd';
    if (is6309()) {
        if (strcasecmp_P(word, PSTR("W")) == 0)
            return 'w';
        if (strcasecmp_P(word, PSTR("V")) == 0)
            return 'v';
    }
    return 0;
}

char Regs::validUint32Reg(const char *word) const {
    if (is6309()) {
        if (strcasecmp_P(word, PSTR("Q")) == 0)
            return 'q';
    }
    return 0;
}

void Regs::setRegValue(char reg, uint32_t value) {
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
    case 'q':
        setQ(value);
        break;
    }
}

bool Memory::is_internal(uint16_t addr) {
    return false;  // External Memory Space
}

uint8_t Memory::read(uint16_t addr) const {
    if (Acia.isSelected(addr))
        return Acia.read(addr);
    return raw_read(addr);
}

void Memory::write(uint16_t addr, uint8_t data) {
    if (Acia.isSelected(addr)) {
        Acia.write(addr, data);
        return;
    }
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
