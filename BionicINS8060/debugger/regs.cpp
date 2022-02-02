#include "regs.h"

#include <libcli.h>

#include <asm_ins8060.h>
#include <dis_ins8060.h>
#include "config.h"
#include "digital_fast.h"
#include "pins.h"
#include "string_util.h"
#include "mc6850.h"

#include <ctype.h>

extern libcli::Cli &cli;
extern Mc6850 Acia;

libasm::ins8060::AsmIns8060 asm8060;
libasm::ins8060::DisIns8060 dis8060;
libasm::Assembler &assembler(asm8060);
libasm::Disassembler &disassembler(dis8060);

struct Regs Regs;
struct Memory Memory;

static constexpr uint8_t cycles[] = {
        2,  // RR  : 00: HALT 1:8
        1,  // R   : 01: XAE  1:7
        1,  // R   : 02: CCL  1:5
        1,  // R   : 03: SCL  1:5
        1,  // R   : 04: DINT 1:6
        1,  // R   : 05: IEN  1:6
        1,  // R   : 06: CSA  1:5
        1,  // R   : 07: CAS  1:6
        1,  // R   : 08: NOP  1:5
        0,  // -   : 09: -    -:-
        0,  // -   : 0A: -    -:-
        0,  // -   : 0B: -    -:-
        0,  // -   : 0C: -    -:-
        0,  // -   : 0D: -    -:-
        0,  // -   : 0E: -    -:-
        0,  // -   : 0F: -    -:-
        0,  // -   : 10: -    -:-
        0,  // -   : 11: -    -:-
        0,  // -   : 12: -    -:-
        0,  // -   : 13: -    -:-
        0,  // -   : 14: -    -:-
        0,  // -   : 15: -    -:-
        0,  // -   : 16: -    -:-
        0,  // -   : 17: -    -:-
        0,  // -   : 18: -    -:-
        1,  // R   : 19: SIO  1:5
        0,  // -   : 1A: -    -:-
        0,  // -   : 1B: -    -:-
        1,  // R   : 1C: SR   1:5
        1,  // R   : 1D: SRL  1:5
        1,  // R   : 1E: RR   1:5
        1,  // R   : 1F: RRL  1:5
        0,  // -   : 20: -    -:-
        0,  // -   : 21: -    -:-
        0,  // -   : 22: -    -:-
        0,  // -   : 23: -    -:-
        0,  // -   : 24: -    -:-
        0,  // -   : 25: -    -:-
        0,  // -   : 26: -    -:-
        0,  // -   : 27: -    -:-
        0,  // -   : 28: -    -:-
        0,  // -   : 29: -    -:-
        0,  // -   : 2A: -    -:-
        0,  // -   : 2B: -    -:-
        0,  // -   : 2C: -    -:-
        0,  // -   : 2D: -    -:-
        0,  // -   : 2E: -    -:-
        0,  // -   : 2F: -    -:-
        1,  // R   : 30: XPAL 1:8
        1,  // R   : 31: XPAL 1:8
        1,  // R   : 32: XPAL 1:8
        1,  // R   : 33: XPAL 1:8
        1,  // R   : 34: XPAH 1:8
        1,  // R   : 35: XPAH 1:8
        1,  // R   : 36: XPAH 1:8
        1,  // R   : 37: XPAH 1:8
        0,  // -   : 38: -    -:-
        0,  // -   : 39: -    -:-
        0,  // -   : 3A: -    -:-
        0,  // -   : 3B: -    -:-
        1,  // R   : 3C: XPPC 1:7
        1,  // R   : 3D: XPPC 1:7
        1,  // R   : 3E: XPPC 1:7
        1,  // R   : 3F: XPPC 1:7
        1,  // R   : 40: LDE  1:6
        0,  // -   : 41: -    -:-
        0,  // -   : 42: -    -:-
        0,  // -   : 43: -    -:-
        0,  // -   : 44: -    -:-
        0,  // -   : 45: -    -:-
        0,  // -   : 46: -    -:-
        0,  // -   : 47: -    -:-
        0,  // -   : 48: -    -:-
        0,  // -   : 49: -    -:-
        0,  // -   : 4A: -    -:-
        0,  // -   : 4B: -    -:-
        0,  // -   : 4C: -    -:-
        0,  // -   : 4D: -    -:-
        0,  // -   : 4E: -    -:-
        0,  // -   : 4F: -    -:-
        1,  // R   : 50: ANE  1:7
        0,  // -   : 51: -    -:-
        0,  // -   : 52: -    -:-
        0,  // -   : 53: -    -:-
        0,  // -   : 54: -    -:-
        0,  // -   : 55: -    -:-
        0,  // -   : 56: -    -:-
        0,  // -   : 57: -    -:-
        1,  // R   : 58: ORE  1:6
        0,  // -   : 59: -    -:-
        0,  // -   : 5A: -    -:-
        0,  // -   : 5B: -    -:-
        0,  // -   : 5C: -    -:-
        0,  // -   : 5D: -    -:-
        0,  // -   : 5E: -    -:-
        0,  // -   : 5F: -    -:-
        1,  // R   : 60: XRE  1:6
        0,  // -   : 61: -    -:-
        0,  // -   : 62: -    -:-
        0,  // -   : 63: -    -:-
        0,  // -   : 64: -    -:-
        0,  // -   : 65: -    -:-
        0,  // -   : 66: -    -:-
        0,  // -   : 67: -    -:-
        1,  // R   : 68: DAE  1:11
        0,  // -   : 69: -    -:-
        0,  // -   : 6A: -    -:-
        0,  // -   : 6B: -    -:-
        0,  // -   : 6C: -    -:-
        0,  // -   : 6D: -    -:-
        0,  // -   : 6E: -    -:-
        0,  // -   : 6F: -    -:-
        1,  // R   : 70: ADE  1:7
        0,  // -   : 71: -    -:-
        0,  // -   : 72: -    -:-
        0,  // -   : 73: -    -:-
        0,  // -   : 74: -    -:-
        0,  // -   : 75: -    -:-
        0,  // -   : 76: -    -:-
        0,  // -   : 77: -    -:-
        1,  // R   : 78: CAE  1:8
        0,  // -   : 79: -    -:-
        0,  // -   : 7A: -    -:-
        0,  // -   : 7B: -    -:-
        0,  // -   : 7C: -    -:-
        0,  // -   : 7D: -    -:-
        0,  // -   : 7E: -    -:-
        0,  // -   : 7F: -    -:-
        0,  // -   : 80: -    -:-
        0,  // -   : 81: -    -:-
        0,  // -   : 82: -    -:-
        0,  // -   : 83: -    -:-
        0,  // -   : 84: -    -:-
        0,  // -   : 85: -    -:-
        0,  // -   : 86: -    -:-
        0,  // -   : 87: -    -:-
        0,  // -   : 88: -    -:-
        0,  // -   : 89: -    -:-
        0,  // -   : 8A: -    -:-
        0,  // -   : 8B: -    -:-
        0,  // -   : 8C: -    -:-
        0,  // -   : 8D: -    -:-
        0,  // -   : 8E: -    -:-
        2,  // RR  : 8F: DLY  2:13-131593
        2,  // RR  : 90: JMP  2:11
        2,  // RR  : 91: JMP  2:11
        2,  // RR  : 92: JMP  2:11
        2,  // RR  : 93: JMP  2:11
        2,  // RR  : 94: JP   2:9/11
        2,  // RR  : 95: JP   2:9/11
        2,  // RR  : 96: JP   2:9/11
        2,  // RR  : 97: JP   2:9/11
        2,  // RR  : 98: JZ   2:9/11
        2,  // RR  : 99: JZ   2:9/11
        2,  // RR  : 9A: JZ   2:9/11
        2,  // RR  : 9B: JZ   2:9/11
        2,  // RR  : 9C: JNZ  2:9/11
        2,  // RR  : 9D: JNZ  2:9/11
        2,  // RR  : 9E: JNZ  2:9/11
        2,  // RR  : 9F: JNZ  2:9/11
        0,  // -   : A0: -    -:-
        0,  // -   : A1: -    -:-
        0,  // -   : A2: -    -:-
        0,  // -   : A3: -    -:-
        0,  // -   : A4: -    -:-
        0,  // -   : A5: -    -:-
        0,  // -   : A6: -    -:-
        0,  // -   : A7: -    -:-
        4,  // RRRW: A8: ILD  2:22
        4,  // RRRW: A9: ILD  2:22
        4,  // RRRW: AA: ILD  2:22
        4,  // RRRW: AB: ILD  2:22
        0,  // -   : AC: -    -:-
        0,  // -   : AD: -    -:-
        0,  // -   : AE: -    -:-
        0,  // -   : AF: -    -:-
        0,  // -   : B0: -    -:-
        0,  // -   : B1: -    -:-
        0,  // -   : B2: -    -:-
        0,  // -   : B3: -    -:-
        0,  // -   : B4: -    -:-
        0,  // -   : B5: -    -:-
        0,  // -   : B6: -    -:-
        0,  // -   : B7: -    -:-
        4,  // RRRW: B8: DLD  2:22
        4,  // RRRW: B9: DLD  2:22
        4,  // RRRW: BA: DLD  2:22
        4,  // RRRW: BB: DLD  2:22
        0,  // -   : BC: -    -:-
        0,  // -   : BD: -    -:-
        0,  // -   : BE: -    -:-
        0,  // -   : BF: -    -:-
        3,  // RRR : C0: LD   2:18
        3,  // RRR : C1: LD   2:18
        3,  // RRR : C2: LD   2:18
        3,  // RRR : C3: LD   2:18
        2,  // RR  : C4: LDI  2:10
        3,  // RRR : C5: LD   2:18
        3,  // RRR : C6: LD   2:18
        3,  // RRR : C7: LD   2:18
        3,  // RRW : C8: ST   2:18
        3,  // RRW : C9: ST   2:18
        3,  // RRW : CA: ST   2:18
        3,  // RRW : CB: ST   2:18
        0,  // -   : CC: -    -:-
        3,  // RRW : CD: ST   2:18
        3,  // RRW : CE: ST   2:18
        3,  // RRW : CF: ST   2:18
        3,  // RRR : D0: AND  2:18
        3,  // RRR : D1: AND  2:18
        3,  // RRR : D2: AND  2:18
        3,  // RRR : D3: AND  2:18
        2,  // RR  : D4: ANI  2:11
        3,  // RRR : D5: AND  2:18
        3,  // RRR : D6: AND  2:18
        3,  // RRR : D7: AND  2:18
        3,  // RRR : D8: OR   2:18
        3,  // RRR : D9: OR   2:18
        3,  // RRR : DA: OR   2:18
        3,  // RRR : DB: OR   2:18
        2,  // RR  : DC: ORI  2:10
        3,  // RRR : DD: OR   2:18
        3,  // RRR : DE: OR   2:18
        3,  // RRR : DF: OR   2:18
        3,  // RRR : E0: XOR  2:18
        3,  // RRR : E1: XOR  2:18
        3,  // RRR : E2: XOR  2:18
        3,  // RRR : E3: XOR  2:18
        2,  // RR  : E4: XRI  2:10
        3,  // RRR : E5: XOR  2:18
        3,  // RRR : E6: XOR  2:18
        3,  // RRR : E7: XOR  2:18
        3,  // RRR : E8: DAD  2:23
        3,  // RRR : E9: DAD  2:23
        3,  // RRR : EA: DAD  2:23
        3,  // RRR : EB: DAD  2:23
        2,  // RR  : EC: DAI  2:15
        3,  // RRR : ED: DAD  2:23
        3,  // RRR : EE: DAD  2:23
        3,  // RRR : EF: DAD  2:23
        3,  // RRR : F0: ADD  2:19
        3,  // RRR : F1: ADD  2:19
        3,  // RRR : F2: ADD  2:19
        3,  // RRR : F3: ADD  2:19
        2,  // RR  : F4: ADI  2:11
        3,  // RRR : F5: ADD  2:19
        3,  // RRR : F6: ADD  2:19
        3,  // RRR : F7: ADD  2:19
        3,  // RRR : F8: CAD  2:20
        3,  // RRR : F9: CAD  2:20
        3,  // RRR : FA: CAD  2:20
        3,  // RRR : FB: CAD  2:20
        2,  // RR  : FC: CAI  2:12
        3,  // RRR : FD: CAD  2:20
        3,  // RRR : FE: CAD  2:20
        3,  // RRR : FF: CAD  2:20
};

uint8_t Regs::bus_cycles(uint8_t insn) {
    return cycles[insn];
}

const char *Regs::cpu() const {
    return "INS8060";
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
        'P', '1', '=', 0, 0, 0, 0, ' ',  // P1=11
        'P', '2', '=', 0, 0, 0, 0, ' ',  // P2=19
        'P', '3', '=', 0, 0, 0, 0, ' ',  // P3=27
        'E', '=', 0, 0, ' ',             // E=34
        'A', '=', 0, 0, ' ',             // A=39
        'S', '=',
        0, 0, 0, 0, 0, 0, 0, 0,          // S=44
        0,
    };
    constexpr char *s_bits = buffer + 44;
    outHex16(buffer + 3, pc);
    outHex16(buffer + 11, p1);
    outHex16(buffer + 19, p2);
    outHex16(buffer + 27, p3);
    outHex8(buffer + 34, e);
    outHex8(buffer + 39, a);
    s_bits[0] = bit1(s & 0x80, 'C');
    s_bits[1] = bit1(s & 0x40, 'V');
    s_bits[2] = bit1(s & 0x20, 'B');
    s_bits[3] = bit1(s & 0x10, 'A');
    s_bits[4] = bit1(s & 0x08, 'I');
    s_bits[5] = bit1(s & 0x04, '2');
    s_bits[6] = bit1(s & 0x02, '1');
    s_bits[7] = bit1(s & 0x01, '0');
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
    // clang-format off
    static const uint8_t ST_ALL[] = {
        0xC8, 0xFE,                         // ST $-1
        0x40, 0xC8, 0xFF,                   // LDE, ST $
        0x06, 0xC8, 0xFF,                   // CSA, ST $
        0x31, 0xC8, 0xFF, 0x35, 0xC8, 0xFF, // XPAL P1, ST $, XPAH P1, ST $
        0x32, 0xC8, 0xFF, 0x36, 0xC8, 0xFF, // XPAL P1, ST $, XPAH P1, ST $
        0x33, 0xC8, 0xFF, 0x37, 0xC8, 0xFF, // XPAL P1, ST $, XPAH P1, ST $
    };
    // clang-format on
    static uint8_t buffer[9];
    if (show)
        Signals::resetCycles();
    Pins.captureWrites(ST_ALL, sizeof(ST_ALL), &pc, buffer, sizeof(buffer));
    if (show)
        Signals::printCycles();
    a = buffer[0];
    e = buffer[1];
    s = buffer[2];
    p1 = le16(buffer + 3);
    p2 = le16(buffer + 5);
    p3 = le16(buffer + 7);
}

void Regs::restore(bool show) {
    // clang-format off
    static uint8_t LD_ALL[] = {
        0xC4, 0, 0x07,                // LDI s, CAS; s=1
        0xC4, 0, 0x01,                // LDI e, XAE; e=4
        0xC4, 0, 0x33, 0xC4, 0, 0x37, // LDI lo(p3), XPAL P1, LDI hi(p3), XPAH P1
        0xC4, 0, 0x32, 0xC4, 0, 0x36, // LDI lo(p2), XPAL P1, LDI hi(p2), XPAH P1
        0xC4, 0, 0x31, 0xC4, 0, 0x35, // LDI lo(pc), XPAL P1, LDI hi(pc), XPAH P1
        0x3D,                         // XPPC P1
        0xC4, 0, 0x31, 0xC4, 0, 0x35, // LDI lo(p1), XPAL P1, LDI hi(p1), XPAH P1
        0xC4, 0,                      // LDI a
    };
    // clang-format on
    LD_ALL[1] = s;
    LD_ALL[4] = e;
    LD_ALL[7] = lo(p3);
    LD_ALL[10] = hi(p3);
    LD_ALL[13] = lo(p2);
    LD_ALL[16] = hi(p2);
    const auto p = _pc(pc, pc - 8); // offset restore P1 and A
    LD_ALL[19] = lo(p);
    LD_ALL[22] = hi(p);
    LD_ALL[26] = lo(p1);
    LD_ALL[29] = hi(p1);
    LD_ALL[32] = a;
    if (show)
        Signals::resetCycles();
    Pins.execInst(LD_ALL, sizeof(LD_ALL));
    if (show)
        Signals::printCycles();
}

void Regs::printRegList() const {
    cli.println(F("?Reg: PC P1 P2 P3 A E S"));
}

char Regs::validUint8Reg(const char *word) const {
    if (word[1])
        return 0;
    const char reg = tolower(*word);
    if (reg == 'a' || reg == 'e' || reg == 's')
        return reg;
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("PC")) == 0)
        return '0';
    if (strcasecmp_P(word, PSTR("P1")) == 0)
        return '1';
    if (strcasecmp_P(word, PSTR("P2")) == 0)
        return '2';
    if (strcasecmp_P(word, PSTR("P3")) == 0)
        return '3';
    return 0;
}

void Regs::setRegValue(char reg, uint32_t value) {
    switch (reg) {
    case '0':
        pc = value;
        break;
    case '1':
        p1 = value;
        break;
    case '2':
        p2 = value;
        break;
    case '3':
        p3 = value;
        break;
    case 'a':
        a = value;
        break;
    case 'e':
        e = value;
        break;
    case 's':
        s = value;
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
    disassembler.setUppercase(true);
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
            cli.println(disassembler.errorText(disassembler.getError()));
            continue;
        }
        cli.printStr(insn.name(), -6);
        cli.printlnStr(operands, -12);
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
        Acia.write(addr,  data);
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
