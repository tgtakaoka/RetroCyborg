#include "regs.h"

#include <libcli.h>

#include <asm_mc6800.h>
#include <dis_mc6800.h>
#include "config.h"
#include "digital_fast.h"
#include "mc6850.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli cli;
extern Mc6850 Acia;

libasm::mc6800::AsmMc6800 assembler;
libasm::mc6800::DisMc6800 disassembler;

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
 *        ; ADX #1  ($EC $01) on MB8861
 * X=$FFFF: MC6800
 * X=$0000: MB8861
 */

static const char MC6800[] = "MC6800";
static const char MB8861[] = "MB8861";

static const char *softwareType(uint16_t x) {
    return x == 0 ? MB8861 : MC6800;
}

void Regs::reset() {
    _cpuType = nullptr;
}

void Regs::setCpuType() {
    static const uint8_t LDX[] = {0xCE, 0xFF, 0xFF};  // LDX #$FFFF
    static const uint8_t ADX[] = {
            0xEC, 0x01, 0x01, 0x01, 0x01, 0x01};  // ADX #1/CPX 1,X, NOP*4
    static const uint8_t STX[] = {0xEF, 0x01, 0x00, 0x00, 0x00};  // STX $0100
    Pins.execInst(LDX, sizeof(LDX));
    Pins.execInst(ADX, sizeof(ADX));
    uint16_t reg_x;
    Pins.captureWrites(STX, sizeof(STX), nullptr, &reg_x, sizeof(reg_x));
    _cpuType = softwareType(reg_x);
}

const char *Regs::cpu() const {
    return _cpuType ? _cpuType : MC6800;
}

const char *Regs::cpuName() const {
    return cpu();
}

static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void Regs::print() const {
    // clang-format off
    static char buffer[] = {
        'P', 'C', '=', 0, 0, 0, 0, ' ',  // PC=3
        'S', 'P', '=', 0, 0, 0, 0, ' ',  // SP=11
        'X', '=', 0, 0, 0, 0, ' ',       // X=18
        'A', '=', 0, 0, ' ',             // A=25
        'B', '=', 0, 0, ' ',             // B=30
        'C', 'C', '=', 0, 0, 0, 0, 0, 0, // CC=36
        0,                               // EOS
    };
    // clang-format on
    outHex16(buffer + 3, pc);
    outHex16(buffer + 11, sp);
    outHex16(buffer + 18, x);
    outHex8(buffer + 25, a);
    outHex8(buffer + 30, b);
    char *p = buffer + 36;
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
    if (_cpuType == nullptr)
        setCpuType();
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
    cli.println(F("?Reg: PC SP X A B CC"));
}

char Regs::validUint8Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("A")) == 0)
        return 'a';
    if (strcasecmp_P(word, PSTR("B")) == 0)
        return 'b';
    if (strcasecmp_P(word, PSTR("CC")) == 0)
        return 'c';
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("PC")) == 0)
        return 'p';
    if (strcasecmp_P(word, PSTR("SP")) == 0)
        return 's';
    if (strcasecmp_P(word, PSTR("X")) == 0)
        return 'x';
    return 0;
}

void Regs::setRegValue(char reg, uint32_t value) {
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
            cli.println(disassembler.errorText_P(disassembler.getError()));
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
        cli.println(assembler.errorText_P(assembler.getError()));
    } else {
        Memory.write(insn.address(), insn.bytes(), insn.length());
        disassemble(insn.address(), 1);
        addr += insn.length();
    }
    return addr;
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

void Memory::write(uint16_t addr, const uint8_t *data, uint8_t len) {
    for (auto i = 0; i < len; i++) {
        write(addr++, *data++);
    }
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
