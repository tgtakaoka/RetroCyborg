#include "regs.h"

#include <libcli.h>

#include <asm_i8080.h>
#include <dis_i8080.h>
#include "config.h"
#include "digital_fast.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli cli;

libasm::i8080::AsmI8080 assembler;
libasm::i8080::DisI8080 disassembler;

struct Regs Regs;
struct Memory Memory;

const char *Regs::cpu() const {
    return "I8085";
}

const char *Regs::cpuName() const {
    return "P8085";
}

static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void Regs::print() const {
    // clang-format off
    static char buffer[] = {
        'P', 'C', '=', 0, 0, 0, 0, ' ',  // PC=3
        'S', 'P', '=', 0, 0, 0, 0, ' ',  // SP=11
        'B', 'C', '=', 0, 0, 0, 0, ' ',  // BC=19
        'D', 'E', '=', 0, 0, 0, 0, ' ',  // DE=27
        'H', 'L', '=', 0, 0, 0, 0, ' ',  // HL=35
        'A', '=', 0, 0, ' ',             // A=42
        'P', 'S', 'W', '=',
        0, 0, 0, 0, 0, 0, 0, 0,          // PSW=49
        ' ', 'I', 'E',                   // IE=57
        0,                               // EOS
    };
    // clang-format on
    outHex16(buffer + 3, pc);
    outHex16(buffer + 11, sp);
    outHex8(buffer + 19, b);
    outHex8(buffer + 21, c);
    outHex8(buffer + 27, d);
    outHex8(buffer + 29, e);
    outHex8(buffer + 35, h);
    outHex8(buffer + 37, l);
    outHex8(buffer + 42, a);
    buffer[49] = bit1(psw & 0x80, 'S');
    buffer[50] = bit1(psw & 0x40, 'Z');
    buffer[51] = (psw & 0x20) ? '1' : '0';
    buffer[52] = bit1(psw & 0x10, 'H');
    buffer[53] = (psw & 0x08) ? '1' : '0';
    buffer[54] = bit1(psw & 0x04, 'P');
    buffer[55] = (psw & 0x02) ? '1' : '0';
    buffer[56] = bit1(psw & 0x01, 'C');
    buffer[57] = ie ? ' ' : 0;
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
    static const uint8_t PUSH_ALL[] = {
            0xC7,        // RST 0
            0xF5,        // PUSH PSW
            0x20,        // RIM
            0xF3,        // DI
            0xC5,        // PUSH B
            0xD5,        // PUSH D
            0xE5,        // PUSH H
            0x32, 0, 0,  // STA 0
    };
    uint8_t buffer[11];
    if (show)
        Signals::resetCycles();
    Pins.captureWrites(PUSH_ALL, sizeof(PUSH_ALL), &sp, buffer, sizeof(buffer));
    sp++;
    pc = be16(buffer) - 1;  // offser RST instruction
    a = buffer[2];
    psw = buffer[3];
    b = buffer[4];
    c = buffer[5];
    d = buffer[6];
    e = buffer[7];
    h = buffer[8];
    l = buffer[9];
    ie = (buffer[10] & 0x08) != 0;
    if (show)
        Signals::printCycles();
}

void Regs::restore(bool show) {
    static uint8_t POP_ALL[] = {
            0x31, 0, 0,  // LXI SP,sp-10
            0xE1, 0, 0,  // POP H
            0xD1, 0, 0,  // POP D
            0xC1, 0, 0,  // POP B
            0xF1, 0, 0,  // POP PSW
            0,           // EI/DI
            0xC9, 0, 0,  // RET
    };
    if (show)
        Signals::resetCycles();
    POP_ALL[1] = lo(sp - 10);
    POP_ALL[2] = hi(sp - 10);
    POP_ALL[4] = l;
    POP_ALL[5] = h;
    POP_ALL[7] = e;
    POP_ALL[8] = d;
    POP_ALL[10] = c;
    POP_ALL[11] = b;
    POP_ALL[13] = psw;
    POP_ALL[14] = a;
    POP_ALL[15] = ie ? 0xFB : 0xF3;  // EI/DI
    POP_ALL[17] = lo(pc);
    POP_ALL[18] = hi(pc);
    Pins.execInst(POP_ALL, sizeof(POP_ALL));
    if (show)
        Signals::printCycles();
}

void Regs::printRegList() const {
    cli.println(F("?Reg: PC SP BC DE HL A B C D E H L PSW"));
}

char Regs::validUint8Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("PSW")) == 0)
        return 'f';
    if (word[1])
        return 0;
    const char c = tolower(*word);
    if (c >= 'a' && c <= 'e')
        return c;
    if (c == 'h' || c == 'l')
        return c;
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("PC")) == 0)
        return 'P';
    if (strcasecmp_P(word, PSTR("SP")) == 0)
        return 'S';
    if (strcasecmp_P(word, PSTR("BC")) == 0)
        return 'B';
    if (strcasecmp_P(word, PSTR("DE")) == 0)
        return 'D';
    if (strcasecmp_P(word, PSTR("HL")) == 0)
        return 'H';
    return 0;
}

void Regs::setRegValue(char reg, uint32_t value) {
    switch (reg) {
    case 'P':
        pc = value;
        break;
    case 'S':
        sp = value;
        break;
    case 'B':
        setBc(value);
        break;
    case 'D':
        setDe(value);
        break;
    case 'H':
        setHl(value);
        break;
    case 'a':
        a = value;
        break;
    case 'b':
        b = value;
        break;
    case 'c':
        c = value;
        break;
    case 'd':
        d = value;
        break;
    case 'e':
        e = value;
        break;
    case 'h':
        h = value;
        break;
    case 'l':
        l = value;
        break;
    case 'f':
        psw = value;
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
    for (int i = insn.length(); i < disassembler.codeMax() + 1; i++) {
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
    return raw_read(addr);
}

void Memory::write(uint16_t addr, uint8_t data) {
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
