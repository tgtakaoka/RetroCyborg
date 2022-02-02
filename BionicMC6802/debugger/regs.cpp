#include "regs.h"

#include <libcli.h>

#include <asm_mc6800.h>
#include <dis_mc6800.h>
#include "config.h"
#include "digital_fast.h"
#include "mc6850.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;
extern Mc6850 Acia;

libasm::mc6800::AsmMc6800 asm6800;
libasm::mc6800::DisMc6800 dis6800;
libasm::Assembler &assembler(asm6800);
libasm::Disassembler &disassembler(dis6800);

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
 *        ; ADX 1   ($EC $01) on MB8861
 * X=$FFFF: MC6800
 * X=$0000: MB8861
 */

const char *Regs::cpu() const {
    return Memory::is_internal_ram_enabled() ? "MC6802" : "MC6808";
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

bool Memory::is_internal_ram_enabled() {
    return digitalReadFast(PIN_RE) == HIGH;
}

uint8_t Memory::internal_read(uint8_t addr) const {
    static uint8_t LDAA_STAA[] = {
            0x96, 0, 0, 0xB7, 0x01, 0x00, 0};  // LDAA dir[addr], STAA $0100
    LDAA_STAA[1] = addr;
    Pins.captureWrites(LDAA_STAA, sizeof(LDAA_STAA), nullptr, &LDAA_STAA[2], 1);
    return LDAA_STAA[2];
}

void Memory::internal_write(uint8_t addr, uint8_t data) const {
    static uint8_t LDAA_STAA[] = {
            0x86, 0, 0x97, 0, 0, 0};  // LDAA #val, STA dir[addr]
    LDAA_STAA[1] = data;
    LDAA_STAA[3] = addr;
    Pins.execInst(LDAA_STAA, sizeof(LDAA_STAA));
}

bool Memory::is_internal(uint16_t addr) {
    return is_internal_ram_enabled() && addr < 0x0100;
}

uint8_t Memory::read(uint16_t addr) const {
    if (Acia.isSelected(addr))
        return Acia.read(addr);
    return is_internal(addr) ? internal_read(addr) : raw_read(addr);
}

void Memory::write(uint16_t addr, uint8_t data) {
    if (Acia.isSelected(addr)) {
        Acia.write(addr, data);
        return;
    }
    if (is_internal(addr)) {
        internal_write(addr, data);
    } else {
        raw_write(addr, data);
    }
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
