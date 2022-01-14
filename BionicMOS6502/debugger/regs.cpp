#include "regs.h"

#include <libcli.h>

#include <asm_mos6502.h>
#include <dis_mos6502.h>
#include "config.h"
#include "digital_fast.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;

libasm::mos6502::AsmMos6502 asm6502;
libasm::mos6502::DisMos6502 dis6502;
libasm::Assembler &assembler(asm6502);
libasm::Disassembler &disassembler(dis6502);

struct Regs Regs;
struct Memory Memory;

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

static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void Regs::print() const {
    // clang-format off
    static char buffer[] = {
            'P', 'C', '=', 0, 0, 0, 0, ' ',  // PC=3
            'S', '=', 0, 0, 0, 0, ' ',       // S=10
            'X', '=', 0, 0, ' ',             // X=17
            'Y', '=', 0, 0, ' ',             // Y=22
            'A', '=', 0, 0, ' ',             // A=27
            'P', '=',
            0, 0, '_', 0, 0, 0, 0, 0,        // P=32
            0,                               // EOS
    };
    // clang-format on
    outHex16(buffer + 3, pc);
    outHex16(buffer + 10, s + 0x0100);
    outHex8(buffer + 17, x);
    outHex8(buffer + 22, y);
    outHex8(buffer + 27, a);
    buffer[32] = bit1(p & 0x80, 'N');
    buffer[33] = bit1(p & 0x40, 'V');
    buffer[35] = bit1(p & 0x10, 'B');
    buffer[36] = bit1(p & 0x08, 'D');
    buffer[37] = bit1(p & 0x04, 'I');
    buffer[38] = bit1(p & 0x02, 'Z');
    buffer[39] = bit1(p & 0x01, 'C');
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

void Regs::save(bool show) {
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

    if (show)
        Signals::resetCycles();
    uint16_t sp;
    Pins.captureWrites(PUSH_ALL, sizeof(PUSH_ALL), &sp, buffer, sizeof(buffer));
    if (_cpuType == nullptr)
        setCpuType();
    if (show)
        Signals::printCycles();

    pc = be16(buffer) - 2;  // pc on stack points the last byte of JSR.
    s = sp;
    p = buffer[2];
    a = buffer[3];
    x = buffer[4];
    y = buffer[5];
}

void Regs::restore(bool show) {
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

    if (show)
        Signals::resetCycles();
    Pins.execInst(PULL_ALL, sizeof(PULL_ALL));
    if (show)
        Signals::printCycles();
}

void Regs::printRegList() const {
    cli.println(F("?Reg: PC S X Y A P"));
}

char Regs::validUint8Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("A")) == 0)
        return 'a';
    if (strcasecmp_P(word, PSTR("X")) == 0)
        return 'x';
    if (strcasecmp_P(word, PSTR("Y")) == 0)
        return 'y';
    if (strcasecmp_P(word, PSTR("P")) == 0)
        return 'P';
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("PC")) == 0)
        return 'p';
    if (strcasecmp_P(word, PSTR("S")) == 0)
        return 's';
    return 0;
}

void Regs::setRegValue(char reg, uint32_t value) {
    switch (reg) {
    case 'p':
        pc = value;
        break;
    case 's':
        s = (value & 0xff) | 0x0100;
        break;
    case 'x':
        x = value;
        break;
    case 'y':
        y = value;
        break;
    case 'a':
        a = value;
        break;
    case 'P':
        p = value;
        break;
    }
}

bool Memory::is_internal(uint16_t addr) {
    return false;
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
