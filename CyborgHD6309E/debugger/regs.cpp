#include "regs.h"

#include <libcli.h>

#include <asm_mc6809.h>
#include <dis_mc6809.h>
#include "mc6850.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;

libasm::mc6809::AsmMc6809 assembler;
libasm::mc6809::DisMc6809 disassembler;

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
static const char MC6809E[] = "MC6809E";
static const char HD6309E[] = "HD6309E";

void Regs::setCpuType() {
    static const uint8_t CLRB[] = {0x5F};
    static const uint8_t COMD[] = {0x10, 0x43};
    static const uint8_t STB[] = {0xD7, 0x00};

    Pins.execInst(CLRB, sizeof(CLRB));
    Pins.execInst(COMD, sizeof(COMD));
    uint8_t reg_b;
    Pins.captureWrites(STB, sizeof(STB), &reg_b, sizeof(reg_b));
    _cpuType = reg_b ? HD6309 : MC6809;
}

const char *Regs::cpu() const {
    return _cpuType ? _cpuType : MC6809;
}

const char *Regs::cpuName() const {
    return is6309() ? HD6309E : MC6809E;
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
        'D', 'P', '=', 0, 0, ' ',  // dp=3
        'P', 'C', '=', 0, 0, 0, 0, // pc=9
        ' ', 'S', '=', 0, 0, 0, 0, // s=16
        ' ', 'U', '=', 0, 0, 0, 0, // u=23
        ' ', 'Y', '=', 0, 0, 0, 0, // y=30
        ' ', 'X', '=', 0, 0, 0, 0, // x=37
        ' ', 'A', '=', 0, 0,       // A=44
        ' ', 'B', '=', 0, 0,       // B=49
        ' ',                       // 6309=51
        'W', '=', 0, 0, 0, 0, ' ', // w=54
        'V', '=', 0, 0, 0, 0,      // v=61
        0,                         // Native mode=65
        'N', 0,
    };
    static char cc_bits[] = {
        ' ', 'C', 'C', '=',
        0, 0, 0, 0, 0, 0, 0, 0, // CC=4
        0,
    };
    // clang-format on
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
    char *p = cc_bits + 4;
    *p++ = bit1(cc & 0x80, 'E');
    *p++ = bit1(cc & 0x40, 'F');
    *p++ = bit1(cc & 0x20, 'H');
    *p++ = bit1(cc & 0x10, 'I');
    *p++ = bit1(cc & 0x08, 'N');
    *p++ = bit1(cc & 0x04, 'Z');
    *p++ = bit1(cc & 0x02, 'V');
    *p = bit1(cc & 0x01, 'C');
    cli.println(cc_bits);
}

void Regs::get(bool show) {
    save();
    restore();
    if (show)
        print();
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

void Regs::save() {
    static const uint8_t PSHS_ALL[] = {0x34, 0xFF};  // PSHS PC,U,Y,X,DP,B,A,CC
    static const uint8_t PSHU_S[] = {0x36, 0x40};    // PSHU S
    static uint8_t buffer[14];

    Pins.captureWrites(PSHS_ALL, sizeof(PSHS_ALL), buffer, 12);
    if (_cpuType == nullptr)
        setCpuType();
    Pins.captureWrites(PSHU_S, sizeof(PSHU_S), buffer + 12, 2);
    if (is6309())
        save6309();
    // capture writes to stack in reverse order.
    pc = le16(buffer + 0) - 2;  // offset PSHS instruction.
    u = le16(buffer + 2);
    y = le16(buffer + 4);
    x = le16(buffer + 6);
    dp = buffer[8];
    b = buffer[9];
    a = buffer[10];
    cc = buffer[11];
    s = le16(buffer + 12) + 12;  // offset PSHS stack frame.
}

void Regs::restore() {
    static uint8_t LDS[] = {0x10, 0xCE, 0, 0};       // LDS #s-12
    static uint8_t PULS_ALL[2 + 12] = {0x35, 0xFF};  // PULS CC,A,B,DP,X,Y,U,PC

    const uint16_t sp = s - 12;
    LDS[2] = hi(sp);
    LDS[3] = lo(sp);
    PULS_ALL[2] = cc;
    PULS_ALL[3] = a;
    PULS_ALL[4] = b;
    PULS_ALL[5] = dp;
    PULS_ALL[6] = hi(x);
    PULS_ALL[7] = lo(x);
    PULS_ALL[8] = hi(y);
    PULS_ALL[9] = lo(y);
    PULS_ALL[10] = hi(u);
    PULS_ALL[11] = lo(u);
    PULS_ALL[12] = hi(pc);
    PULS_ALL[13] = lo(pc);

    if (is6309())
        restore6309();
    Pins.execInst(LDS, sizeof(LDS));
    Pins.execInst(PULS_ALL, sizeof(PULS_ALL));
}

void Regs::save6309() {
    static const uint8_t TFR_WY[] = {0x1F, 0x62};   // TFR W,Y
    static const uint8_t TFR_VX[] = {0x1F, 0x71};   // TFR V,X
    static const uint8_t PSHU_YX[] = {0x36, 0x30};  // PSHU Y,X
    static uint8_t buffer[4];

    Pins.execInst(TFR_WY, sizeof(TFR_WY));
    Pins.execInst(TFR_VX, sizeof(TFR_VX));
    Pins.captureWrites(PSHU_YX, sizeof(PSHU_YX), buffer, 4);
    f = buffer[0];
    e = buffer[1];
    v = le16(buffer + 2);
}

void Regs::restore6309() {
    static uint8_t LDD[] = {0xCC, 0, 0};           // LDD #v
    static const uint8_t TFR_DV[] = {0x1F, 0x07};  // TFR D,V
    static uint8_t LDW[] = {0x10, 0x86, 0, 0};     // LDW #w

    LDD[1] = hi(v);
    LDD[2] = lo(v);
    LDW[2] = e;
    LDW[3] = f;
    Pins.execInst(LDD, sizeof(LDD));
    Pins.execInst(TFR_DV, sizeof(TFR_DV));
    Pins.execInst(LDW, sizeof(LDW));
}

void Regs::printRegList() const {
    if (is6309()) {
        cli.print(F("?Reg: PC S U X Y A B E F D W Q V CC DP"));
    } else {
        cli.print(F("?Reg: PC S U X Y A B D CC DP"));
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
    case 'd':
        setD(value);
        break;
    case 'w':
        setW(value);
        break;
    case 'v':
        v = value;
        break;
    case 'a':
        a = value;
        break;
    case 'b':
        b = value;
        break;
    case 'e':
        e = value;
        break;
    case 'f':
        f = value;
        break;
    case 'D':
        dp = value;
        break;
    case 'c':
        cc = value;
        break;
    case 'q':
        setQ(value);
        break;
    }
    restore();
    return true;
}

static void printInsn(const libasm::Insn &insn) {
    cli.printHex(insn.address(), 4);
    cli.print(':');
    for (int i = 0; i < insn.length(); i++) {
        cli.printHex(insn.bytes()[i], 2);
        cli.print(' ');
    }
    for (int i = insn.length(); i < 5; i++) {
        cli.print(F("   "));
    }
}

uint16_t Regs::disassemble(uint16_t addr, uint16_t numInsn) const {
    disassembler.setCpu(cpu());
    disassembler.setOption("uppercase", "true");
    uint16_t num = 0;
    while (num < numInsn) {
        char operands[20];
        libasm::Insn insn(addr);
        Memory.setAddress(addr);
        disassembler.decode(Memory, insn, operands, sizeof(operands));
        addr += insn.length();
        num++;
        printInsn(insn);
        if (disassembler.getError()) {
            cli.print(F("Error: "));
            cli.println(reinterpret_cast<const __FlashStringHelper *>(
                    disassembler.errorText_P(disassembler.getError())));
            continue;
        }
        cli.printStr(insn.name(), -6);
        cli.printlnStr(operands, -12);
    }
    return addr;
}

uint16_t Regs::assemble(uint16_t addr, const char *line) const {
    assembler.setCpu(cpu());
    libasm::Insn insn(addr);
    if (assembler.encode(line, insn)) {
        cli.print(F("Error: "));
        cli.println(reinterpret_cast<const __FlashStringHelper *>(
                assembler.errorText_P(assembler.getError())));
    } else {
        Memory.write(insn.address(), insn.bytes(), insn.length());
        disassemble(insn.address(), 1);
        addr += insn.length();
    }
    return addr;
}

uint8_t Memory::read(uint16_t addr) const {
    static uint8_t LDA[] = {0xB6, 0, 0};  // LDA $addr
    LDA[1] = hi(addr);
    LDA[2] = lo(addr);
    uint8_t data;
    Pins.captureReads(LDA, sizeof(LDA), &data, 1);
    return data;
}

void Memory::write(uint16_t addr, uint8_t data) {
    static uint8_t LDA[] = {0x86, 0};     // LDA #data
    static uint8_t STA[] = {0xB7, 0, 0};  // STA $addr
    LDA[1] = data;
    STA[1] = hi(addr);
    STA[2] = lo(addr);
    Pins.execInst(LDA, sizeof(LDA));
    Pins.execInst(STA, sizeof(STA));
}

void Memory::write(uint16_t addr, const uint8_t *data, uint8_t len) {
    for (auto i = 0; i < len; i++) {
        write(addr++, *data++);
    }
}

uint8_t Memory::nextByte() {
    Regs.save();
    const uint8_t data = read(address());
    Regs.restore();
    return data;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
