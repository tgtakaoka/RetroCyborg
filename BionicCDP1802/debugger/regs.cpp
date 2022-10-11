#include "regs.h"

#include <libcli.h>

#include <asm_cdp1802.h>
#include <dis_cdp1802.h>
#include "mc6850.h"
#include "pins.h"
#include "string_util.h"

#include <ctype.h>

extern libcli::Cli &cli;
extern Mc6850 Acia;

libasm::cdp1802::AsmCdp1802 assembler;
libasm::cdp1802::DisCdp1802 disassembler;

struct Regs Regs;
struct Memory Memory;

static constexpr bool debug_cycles = false;

/**
 * - CDP1802: 0x68 opecode causes illegal write on M(R(P)).
 * - CDP1804/CDP1804A:0x68 opecode causes double instruction fetch.
 * TODO: distinguish CDP1804 and CDP1804A
 */

static const char CDP1802[] = "CDP1802";
static const char CDP1804A[] = "CDP1804A";

void Regs::setCpuType() {
    Signals::inject(0x68);
    Pins.cycle();
    Signals &signals = Pins.rawPrepareCycle();
    if (signals.fetchInsn()) {
        _cpuType = CDP1804A;
        Signals::inject(0xFC);  // DADI 0
        Pins.cycle();
        Signals::inject(0);
        Pins.cycle();
        Pins.cycle();  // execution cycle
    } else {
        _cpuType = CDP1802;
        Signals::capture();
        Pins.completeCycle(Pins.directCycle(signals));
    }
}

const char *Regs::cpu() const {
    return _cpuType ? _cpuType : CDP1802;
}

const char *Regs::cpuName() const {
    return cpu();
}

void Regs::reset() {
    _cpuType = nullptr;
}

bool Regs::is1804() const {
    return _cpuType == CDP1804A;
}

static char bit1(uint8_t v, char name) __attribute__((unused));
static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void Regs::print() const {
    // clang-format off
    static char line0[] = {
        'D', '=', 0, 0, ' ',    // D=2
        'D', 'F', '=', 0, ' ',  // DF=8
        'X', '=', 0, ' ',       // X=12
        'P', '=', 0, ' ',       // P=16
        'T', '=', 0, 0, ' ',    // T=20
        'Q', '=', 0, ' ',       // Q=25
        'I', 'E', '=', 0,       // IE=30
        0,                      // EOS
    };
    // clang-format on
    outHex8(line0 + 2, d);
    outHex4(line0 + 8, df);
    outHex4(line0 + 12, x);
    outHex4(line0 + 16, p);
    outHex8(line0 + 20, t);
    outHex4(line0 + 25, q);
    outHex4(line0 + 30, ie);
    cli.println(line0);
    Pins.idle();
    // clang-format off
    static char line1[] = {
        ' ', 'R', '0', '=', 0, 0, 0, 0, ' ', // Rn=4+9n
        ' ', 'R', '1', '=', 0, 0, 0, 0, ' ',
        ' ', 'R', '2', '=', 0, 0, 0, 0, ' ',
        ' ', 'R', '3', '=', 0, 0, 0, 0, ' ',
        ' ', 'R', '4', '=', 0, 0, 0, 0, ' ',
        ' ', 'R', '5', '=', 0, 0, 0, 0, ' ',
        ' ', 'R', '6', '=', 0, 0, 0, 0, ' ',
        ' ', 'R', '7', '=', 0, 0, 0, 0,
        0,
    };
    // clang-format on
    for (auto i = 0; i < 8; i++)
        outHex16(line1 + 4 + i * 9, r[i]);
    cli.println(line1);
    Pins.idle();
    // clang-format off
    static char line2[] = {
        ' ', 'R', '8', '=', 0, 0, 0, 0, ' ', // Rn=4+9(n-8)
        ' ', 'R', '9', '=', 0, 0, 0, 0, ' ',
        'R', '1', '0', '=', 0, 0, 0, 0, ' ',
        'R', '1', '1', '=', 0, 0, 0, 0, ' ',
        'R', '1', '2', '=', 0, 0, 0, 0, ' ',
        'R', '1', '3', '=', 0, 0, 0, 0, ' ',
        'R', '1', '4', '=', 0, 0, 0, 0, ' ',
        'R', '1', '5', '=', 0, 0, 0, 0,
        0,
    };
    // clang-format on
    for (auto i = 8; i < 16; i++)
        outHex16(line2 + 4 + (i - 8) * 9, r[i]);
    cli.println(line2);
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
    static const uint8_t SAV[] = { 0x78 };
    static const uint8_t STR[] = { // STR R0, STR R1, MARK, STR R3, ...
        0x50, 0x51, 0x79, 0x53, 0x54, 0x55, 0x56, 0x57,
        0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F
    };
    // clang-format on

    if (debug_cycles)
        cli.println(F("@@ save"));
    if (show)
        Signals::resetCycles();
    Pins.captureWrites(SAV, sizeof(SAV), nullptr, &t, sizeof(t));
    for (auto i = 0; i < 16; i++) {
        uint8_t tmp;
        Pins.captureWrites(&STR[i], 1, &r[i], &tmp, sizeof(tmp));
        if (i == 0)
            d = tmp;
        if (i == 2) {  // MARK
            x = tmp >> 4;
            p = tmp & 0xF;
        }
        dirty[i] = false;
    }
    dirty[2] = true;              // becase of MARK
    dirty[p] = true;              // becase this is a program counter
    r[p] -= sizeof(SAV) + p + 1;  // adjust program counter
    df = Pins.skip(0xCF);         // LSDF: skip if DF=1
    q = Pins.skip(0xCD);          // LSQ: skip if Q=1
    ie = Pins.skip(0xCC);         // LSIE: skip if IE=1
    if (_cpuType == nullptr)
        setCpuType();
    if (show)
        Signals::printCycles();
}

void Regs::restore(bool show) {
    // clang-format off
    static uint8_t LDT[] = {
        0xD0,                   // SEP Rn
        0xE0,                   // SEX Rn
        0x79,                   // MARK
    };
    static uint8_t LDQ_DF[] = {
        0x7A,                   // REQ:0x7A, SEQ:0x7B
        0xF8, 0,                // LDI df
        0x76,                   // SHRC
    };
    static const uint8_t SEP15_SEX14[] = {
        0xDF,                   // SEP R15
        0xEE                    // SEX R14
    };
    static uint8_t LDR[] = {    // CDP1802
        0xF8, 0,                // LDI hi(Rn)
        0xB0,                   // PHI Rn
        0xF8, 0,                // LDI lo(Rn)
        0xA0,                   // PLO Rn
    };
    static uint8_t RLDI[] = {   // CDP1804
        0x68, 0xC0, 0, 0        // RLDI
    };
    static uint8_t LDD[] = {
        0xF8, 0                 // LDI d
    };
    static uint8_t RET[] = {
        0x70,                   // RET: 0x70 or DIS:0x71
        0,                      // x,p
    };
    // clang-format on

    if (debug_cycles)
        cli.println(F("@@ restore"));
    if (show)
        Signals::resetCycles();
    uint8_t tmp = t & 0xF;
    dirty[tmp] = true;
    LDT[0] = 0xD0 | tmp;       // SEP
    LDT[1] = 0xE0 | (t >> 4);  // SEX
    Pins.captureWrites(LDT, sizeof(LDT), nullptr, &tmp, sizeof(tmp));
    LDQ_DF[0] = q ? 0x7B : 0x7A;
    LDQ_DF[2] = df ? 1 : 0;
    Pins.execInst(LDQ_DF, sizeof(LDQ_DF));
    Pins.execInst(SEP15_SEX14, sizeof(SEP15_SEX14));
    dirty[14] = dirty[15] = true;
    for (auto i = 0; i < 16; i++) {
        if (dirty[i]) {
            uint16_t rn = r[i];
            if (i == 14)
                rn -= 1;  // offset R14
            if (i == 15)
                rn -= sizeof(LDD) + 1;  // offset R15
            if (is1804()) {
                RLDI[1] = 0xC0 | i;
                RLDI[2] = hi(rn);
                RLDI[3] = lo(rn);
                Pins.execInst(RLDI, sizeof(RLDI));
            } else {
                LDR[1] = hi(rn);
                LDR[2] = 0xB0 | i;
                LDR[4] = lo(rn);
                LDR[5] = 0xA0 | i;
                Pins.execInst(LDR, sizeof(LDR));
            }
        }
    }
    LDD[1] = d;
    Pins.execInst(LDD, sizeof(LDD));  // R15+=2
    RET[0] = ie ? 0x70 : 0x71;
    RET[1] = (x << 4) | p;
    Pins.execInst(RET, sizeof(RET));  // R15++, R14++
    if (show)
        Signals::printCycles();
}

void Regs::printRegList() const {
    cli.println(F("?Reg: D DF X P T IE Q R0~R15 RP RX"));
}

char Regs::validUint8Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("D")) == 0)
        return 'd';
    if (strcasecmp_P(word, PSTR("DF")) == 0)
        return 'f';
    if (strcasecmp_P(word, PSTR("P")) == 0)
        return 'p';
    if (strcasecmp_P(word, PSTR("X")) == 0)
        return 'x';
    if (strcasecmp_P(word, PSTR("T")) == 0)
        return 't';
    if (strcasecmp_P(word, PSTR("IE")) == 0)
        return 'i';
    if (strcasecmp_P(word, PSTR("Q")) == 0)
        return 'q';
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("RP")) == 0)
        return 'P';
    if (strcasecmp_P(word, PSTR("RX")) == 0)
        return 'X';
    if (toupper(*word++) == 'R' && isdigit(*word)) {
        uint16_t num = *word++ - '0';
        if (isdigit(*word)) {
            num *= 10;
            num += *word++ - '0';
        }
        if (*word == 0 && num < 16)
            return num < 10 ? num + '0' : num - 10 + 'A';
    }
    return 0;
}

void Regs::setRegValue(char reg, uint32_t value) {
    switch (reg) {
    case 'd':
        d = value;
        break;
    case 'f':
        df = value & 1;
        break;
    case 'p':
        p = value & 0xF;
        break;
    case 'x':
        x = value & 0xF;
        break;
    case 't':
        t = value;
        break;
    case 'i':
        ie = value & 1;
        break;
    case 'q':
        q = value & 1;
        break;
    case 'P':
        r[p] = value;
        dirty[p] = true;
        break;
    case 'X':
        r[x] = value;
        dirty[x] = true;
        break;
    default:
        if (isxdigit(reg)) {
            const auto num = isdigit(reg) ? reg - '0' : reg - 'A' + 10;
            r[num] = value;
            dirty[num] = true;
        }
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

static const char TEXT_TRUE[] = "true";
static const char OPT_USE_REGISTER[] = "use-register";

uint16_t Regs::disassemble(uint16_t addr, uint16_t numInsn) const {
    disassembler.setCpu(cpu());
    disassembler.setOption("uppercase", TEXT_TRUE);
    disassembler.setOption(OPT_USE_REGISTER, TEXT_TRUE);
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
    assembler.setOption(OPT_USE_REGISTER, TEXT_TRUE);
    assembler.setOption("smart-branch", TEXT_TRUE);
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

void Memory::write(uint16_t addr, const uint8_t *data, uint8_t len) {
    for (auto i = 0; i < len; i++) {
        write(addr++, *data++);
    }
}

uint8_t Memory::internal_read(uint8_t addr) const {
    return 0;
}

void Memory::internal_write(uint8_t addr, uint8_t data) const {}

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
