#include "regs.h"

#include <libcli.h>

#include <asm_mos6502.h>
#include <dis_mos6502.h>
#include "config.h"
#include "digital_fast.h"
#include "mc6850.h"
#include "pins.h"
#include "string_util.h"

static constexpr bool debug_cycles = false;

extern libcli::Cli &cli;
extern Mc6850 Acia;

libasm::mos6502::AsmMos6502 assembler;
libasm::mos6502::DisMos6502 disassembler;

struct Regs Regs;
struct Memory Memory;
EXTMEM uint8_t memory[Memory::memory_size];

static const char MOS6502[] = "MOS6502";
static const char G65SC02[] = "G65SC02";
static const char R65C02[] = "R65C02";
static const char W65C02S[] = "W65C02S";
static const char W65C816S[] = "W65C816S";

static const char *hardwareType() {
    switch (Signals::mpuType()) {
    case Signals::MpuType::W65C02S:
        return W65C02S;
    case Signals::MpuType::W65C816S:
        return W65C816S;
    default:
        return MOS6502;
    }
}

static const char *softwareType(uint8_t a) {
    switch (a) {
    case 'N':
        return MOS6502;
    case 'S':
        return G65SC02;
    case '8':
        return W65C816S;
    default:
        return Signals::mpuType() == Signals::MpuType::W65C02S ? W65C02S
                                                               : R65C02;
    }
}

void Regs::reset() {
    _cpuType = nullptr;
}

static uint8_t write(uint16_t addr, uint8_t val) {
    const auto old = Memory.raw_read(addr);
    Memory.raw_write(addr, val);
    return old;
}

void Regs::setCpuType() {
    // A new way to distinguish 6502 variants in software
    // http://forum.6502.org/viewtopic.php?f=2&t=5931
    static const uint8_t RUN_DETECT[] = {
            0xA9, 0x4E,        // LDA #$4E
            0x4C, 0x00, 0x10,  // JMP $1000
    };
    static const uint8_t STA[] = {
            0x85, 0x1D  // STA $1D
    };

    // See samples/detect.asm
    const auto m1d = write(0x1D, 'S' ^ '8');
    const auto m83 = write(0x83, 'N' ^ 'S');  // 0x1D
    const auto m84 = write(0x84, 0x00);
    const auto m85 = write(0x85, 0x00);
    // 6502:   LSR $83; EOR $83
    // 65SC02: NOP; NOP
    // 65C02:  RMB4 $83
    // 65C816: EOR [$83]
    const auto m1000 = write(0x1000, 0x47);
    const auto m1001 = write(0x1001, 0x83);

    Pins.execInst(RUN_DETECT, sizeof(RUN_DETECT));
    Pins.rawStep();  // execute $47, $83
    uint8_t a;
    Pins.captureWrites(STA, sizeof(STA), nullptr, &a, sizeof(a));
    a ^= Memory.raw_read(0x83);
    _cpuType = softwareType(a);

    Memory.raw_write(0x1D, m1d);
    Memory.raw_write(0x83, m83);
    Memory.raw_write(0x84, m84);
    Memory.raw_write(0x85, m85);
    Memory.raw_write(0x1000, m1000);
    Memory.raw_write(0x1001, m1001);
}

const char *Regs::cpu() const {
    return _cpuType ? _cpuType : hardwareType();
}

const char *Regs::cpuName() const {
    return cpu();
}

bool Regs::is65816() const {
    return _cpuType == W65C816S;
}

static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void Regs::print() const {
    char *buffer;
    char *pbuf;
    if (Signals::native65816()) {
        // clang-format off
        static char b65816[] = {
                'K', '=', 0, 0, ' ',              // PBR=2
                'P', 'C', '=', 0, 0, 0, 0, ' ',   // PC=8
                'S', '=', 0, 0, 0, 0, ' ',        // S=15
                'D', '=', 0, 0, 0, 0, ' ',        // D=22
                'B', '=', 0, 0, ' ',              // DBR=29
                'X', '=', 0, 0, 0, 0, ' ',        // X=34
                'Y', '=', 0, 0, 0, 0, ' ',        // Y=41
                'C', '=', 0, 0, 0, 0, ' ',        // C=48
                'P', '=', 0, 0, 0, 0, 0, 0, 0, 0, // P=55
                'E',                              // E
                0,                                // EOS
        };
        // clang-format on
        buffer = b65816;
        outHex8(buffer + 2, pbr);
        outHex16(buffer + 8, pc);
        outHex16(buffer + 15, s);
        outHex16(buffer + 22, d);
        outHex8(buffer + 29, dbr);
        outHex16(buffer + 34, x);
        outHex16(buffer + 41, y);
        outHex16(buffer + 48, getC());
        pbuf = &buffer[55];
        if (e) {
            pbuf[2] = '_';
            pbuf[3] = bit1(p & 0x10, 'B');
            pbuf[8] = 'E';
        } else {
            pbuf[2] = bit1(p & 0x20, 'M');
            pbuf[3] = bit1(p & 0x10, 'X');
            pbuf[8] = '_';
        }
    } else {
        // clang-format off
        static char b6502[] = {
                'P', 'C', '=', 0, 0, 0, 0, ' ',      // PC=3
                'S', '=', 0, 0, 0, 0, ' ',           // S=10
                'X', '=', 0, 0, ' ',                 // X=17
                'Y', '=', 0, 0, ' ',                 // Y=22
                'A', '=', 0, 0, ' ',                 // A=27
                'P', '=', 0, 0, '_', 0, 0, 0, 0, 0,  // P=32
                0,                                   // EOS
        };
        // clang-format on
        buffer = b6502;
        outHex16(buffer + 3, pc);
        outHex16(buffer + 10, s);
        outHex8(buffer + 17, x);
        outHex8(buffer + 22, y);
        outHex8(buffer + 27, a);
        pbuf = &buffer[32];
        pbuf[3] = bit1(p & 0x10, 'B');
    }
    pbuf[0] = bit1(p & 0x80, 'N');
    pbuf[1] = bit1(p & 0x40, 'V');
    pbuf[4] = bit1(p & 0x08, 'D');
    pbuf[5] = bit1(p & 0x04, 'I');
    pbuf[6] = bit1(p & 0x02, 'Z');
    pbuf[7] = bit1(p & 0x01, 'C');
    cli.println(buffer);
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
static void setle16(uint8_t *p, const uint16_t v) {
    p[0] = lo(v);
    p[1] = hi(v);
}

static constexpr uint8_t noop = 0xFF;

static const char OPT_LONGA[] = "longa";
static const char OPT_LONGX[] = "longx";
static const char TEXT_TRUE[] = "true";
static const char TEXT_FALSE[] = "false";

void Regs::save(bool show) {
    e = Signals::native65816() ? 0 : 1;
    if (e == 0) {
        save65816(show);
        return;
    }

    static const uint8_t PUSH_ALL[] = {
            // 6502:  JSR PCL --- PCH
            // 65816: JSR PCL PCH ---
            0x20, 0x00, 0x02, 0x02,  // JSR $0200
            0x08, noop,              // PHP
            0x48, noop,              // PHA
            0x8A, noop,              // TXA
            0x48, noop,              // PHA
            0x98, noop,              // TYA
            0x48, noop,              // PHA
    };
    uint8_t buffer[6];

    if (debug_cycles)
        cli.println(F("save"));
    if (show)
        Signals::resetCycles();
    uint16_t sp;
    Pins.captureWrites(PUSH_ALL, sizeof(PUSH_ALL), &sp, buffer, sizeof(buffer));
    if (_cpuType == nullptr)
        setCpuType();
    if (show)
        Signals::printCycles();

    pc = be16(buffer) - 2;  // pc on stack points the last byte of JSR.
    s = lo(sp) | 0x0100;
    setP(buffer[2]);
    a = buffer[3];
    x = buffer[4];
    y = buffer[5];
}

void Regs::save65816(bool show) {
    static const uint8_t PUSH_ALL[] = {
            0x22, 0x00, 0x02, noop, 0x00,  // JSL $000200; PBR=0, PC=1
            0x08, noop,                    // PHP; P=3
            0xC2, 0x30, noop,              // REP #$30; M=0, X=0
            0x48, noop,                    // PHA; C=4
            0x8B, noop,                    // PHB; DBR=6
            0xDA, noop,                    // PHX; X=7
            0x5A, noop,                    // PHY; Y=9
            0x0B, noop,                    // PHD; D=11
    };
    uint8_t buffer[13];

    if (debug_cycles)
        cli.println(F("save65816"));
    if (show)
        Signals::resetCycles();
    Pins.captureWrites(PUSH_ALL, sizeof(PUSH_ALL), &s, buffer, sizeof(buffer));
    if (_cpuType == nullptr)
        setCpuType();
    if (show)
        Signals::printCycles();

    pbr = buffer[0];
    pc = be16(buffer + 1) - 3;  // pc on stack points the last byte of JSL.
    setP(buffer[3]);
    b = buffer[4];
    a = buffer[5];
    dbr = buffer[6];
    x = be16(buffer + 7);
    y = be16(buffer + 9);
    d = be16(buffer + 11);
}

void Regs::restore(bool show) {
    if (Signals::native65816()) {
        restore65816(show);
        return;
    }

    static uint8_t PULL_ALL[] = {
            0xA2, 0,     // s:1 LDX #s
            0x9A, noop,  // TXS
            0xA0, 0,     // y:5 LDY #y
            0xA2, 0,     // x:7 LDX #x
            0xA9, 0,     // a:9 LDA #a
            0x40,        // RTI
            noop,        // fetch next op code
            noop,        // discarded stack fetch
            0,           // p:13
            0,           // lo(pc):14
            0,           // hi(pc):15
    };

    PULL_ALL[1] = s - 3;  // offset for RTI
    PULL_ALL[5] = y;
    PULL_ALL[7] = x;
    PULL_ALL[9] = a;
    PULL_ALL[13] = p;
    setle16(PULL_ALL + 14, pc);

    if (debug_cycles)
        cli.println(F("restore"));
    if (show)
        Signals::resetCycles();
    Pins.execInst(PULL_ALL, sizeof(PULL_ALL));
    if (show)
        Signals::printCycles();
}

void Regs::restore65816(bool show) {
    static uint8_t PULL_ALL[] = {
            0xC2, 0x30, noop,     //        REP #$30; M=0, X=0
            0xA9, 0, 0,           // d:4    LDA #d
            0x5B, noop,           //        TCD
            0xA0, 0, 0,           // y:9    LDY #y
            0xA2, 0, 0,           // x:12   LDX #x
            0xA9, 0, 0,           // s:15   LDA #s-4
            0x1B, noop,           //        TCS
            0xA9, 0, 0,           // c:20   LDA #c
            0xAB, noop, noop, 0,  // dbr:25 PLB (dbr)
            0x40,                 //        RTI
            noop, noop,           //        fetch next op and increment stack
            0,                    // p:29
            0, 0,                 // pc:30
            0,                    // pbr:32
    };

    setle16(PULL_ALL + 4, d);
    setle16(PULL_ALL + 9, y);
    setle16(PULL_ALL + 12, x);
    setle16(PULL_ALL + 15, s - 5);  // offset PLB(dbr) and RTI(p,pc,pbr)
    PULL_ALL[20] = a;
    PULL_ALL[21] = b;
    PULL_ALL[25] = dbr;
    PULL_ALL[29] = p;
    setle16(PULL_ALL + 30, pc);
    PULL_ALL[32] = pbr;

    if (debug_cycles)
        cli.println(F("restore65816"));
    if (show)
        Signals::resetCycles();
    Pins.execInst(PULL_ALL, sizeof(PULL_ALL));
    if (show)
        Signals::printCycles();
}

void Regs::printRegList() const {
    if (is65816()) {
        cli.println(F("?Reg: PC S X Y A C D P K/PBR B/DBR"));
    } else {
        cli.println(F("?Reg: PC S X Y A P"));
    }
}

char Regs::validUint8Reg(const char *word) const {
    if (is65816()) {
        if (strcasecmp_P(word, PSTR("B")) == 0 ||
                strcasecmp_P(word, PSTR("DBR")) == 0)
            return 'B';
        if (strcasecmp_P(word, PSTR("K")) == 0 ||
                strcasecmp_P(word, PSTR("PBR")) == 0)
            return 'K';
    } else {
        if (strcasecmp_P(word, PSTR("X")) == 0)
            return 'x';
        if (strcasecmp_P(word, PSTR("Y")) == 0)
            return 'y';
    }
    if (strcasecmp_P(word, PSTR("A")) == 0)
        return 'a';
    if (strcasecmp_P(word, PSTR("P")) == 0)
        return 'P';
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (is65816()) {
        if (strcasecmp_P(word, PSTR("C")) == 0)
            return 'c';
        if (strcasecmp_P(word, PSTR("X")) == 0)
            return 'x';
        if (strcasecmp_P(word, PSTR("Y")) == 0)
            return 'y';
        if (strcasecmp_P(word, PSTR("D")) == 0)
            return 'd';
    }
    if (strcasecmp_P(word, PSTR("PC")) == 0)
        return 'p';
    if (strcasecmp_P(word, PSTR("S")) == 0)
        return 's';
    return 0;
}

void Regs::setP(uint8_t value) {
    const char *longa = TEXT_FALSE;
    const char *longx = TEXT_FALSE;
    if (e) {
        p = value | 0x20;
    } else {
        p = value;
        longa = (p & 0x20) == 0 ? TEXT_TRUE : TEXT_FALSE;
        longx = (p & 0x10) == 0 ? TEXT_TRUE : TEXT_FALSE;
    }
    assembler.setOption(OPT_LONGA, longa);
    assembler.setOption(OPT_LONGX, longx);
    disassembler.setOption(OPT_LONGA, longa);
    disassembler.setOption(OPT_LONGX, longx);
}

void Regs::setRegValue(char reg, uint32_t value) {
    switch (reg) {
    case 'p':
        pc = value;
        break;
    case 's':
        if (is65816()) {
            s = value;
        } else {
            s = (value & 0xff) | 0x0100;
        }
        break;
    case 'x':
        x = value;
        break;
    case 'y':
        y = value;
        break;
    case 'd':
        d = value;
        break;
    case 'a':
        a = value;
        break;
    case 'c':
        setC(value);
        break;
    case 'P':
        setP(value);
        break;
    case 'B':
        dbr = value;
        break;
    case 'K':
        pbr = value;
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

uint32_t Regs::disassemble(uint32_t addr, uint16_t numInsn) const {
    disassembler.setCpu(cpu());
    disassembler.setOption("uppercase", TEXT_TRUE);
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

uint32_t Regs::assemble(uint32_t addr, const char *line) const {
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

uint8_t Memory::read(uint32_t addr) const {
    if (Acia.isSelected(addr))
        return Acia.read(addr);
    return raw_read(addr);
}

void Memory::write(uint32_t addr, uint8_t data) {
    if (Acia.isSelected(addr)) {
        Acia.write(addr, data);
        return;
    }
    raw_write(addr, data);
}

void Memory::write(uint32_t addr, const uint8_t *data, uint8_t len) {
    for (auto i = 0; i < len; i++) {
        write(addr++, *data++);
    }
}

uint8_t Memory::raw_read(uint32_t addr) const {
    return addr < memory_size ? memory[addr] : 0xFF;
}

void Memory::raw_write(uint32_t addr, uint8_t data) {
    if (addr < memory_size)
        memory[addr] = data;
}

uint16_t Memory::raw_read_uint16(uint32_t addr) const {
    return (static_cast<uint16_t>(raw_read(addr)) << 8) | raw_read(addr + 1);
}

void Memory::raw_write_uint16(uint32_t addr, uint16_t data) {
    raw_write(addr, data >> 8);
    raw_write(addr + 1, data);
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
