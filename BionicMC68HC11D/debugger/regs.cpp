#include "regs.h"

#include "config.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;

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
    return "MC68HC11";
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
        'Y', '=', 0, 0, 0, 0, ' ',       // Y=25
        'A', '=', 0, 0, ' ',             // A=32
        'B', '=', 0, 0, ' ',             // B=37
        'C', 'C', '=',                   // CC=43
        0, 0, 0, 0, 0, 0, 0, 0,
        0,                               // EOS
    };
    // clang-format on
    outHex16(buffer + 3, pc);
    outHex16(buffer + 11, sp);
    outHex16(buffer + 18, x);
    outHex16(buffer + 25, y);
    outHex8(buffer + 32, a);
    outHex8(buffer + 37, b);
    char *p = buffer + 43;
    *p++ = bit1(cc & 0x80, 'S');
    *p++ = bit1(cc & 0x40, 'X');
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

    uint8_t bytes[12];
    if (show)
        Signals::resetCycles();
    Pins.captureWrites(SWI, sizeof(SWI), &sp, bytes, 9);
    // Capturing writes to stack in little endian order.
    pc = le16(bytes) - 1;  //  offset SWI instruction.
    // Injecting PC as SWI vector (with irrelevant read ahead).
    bytes[9] = 0;
    bytes[10] = hi(pc);
    bytes[11] = lo(pc);
    Pins.execInst(bytes + 9, 3);
    if (show)
        Signals::printCycles();

    y = le16(bytes + 2);
    x = le16(bytes + 4);
    a = bytes[6];
    b = bytes[7];
    cc = bytes[8];
}

void Regs::restore(bool show) {
    static uint8_t LDS_RTI[3 + 12] = {0x8E, 0, 0, 0x3B, 0xFF, 0xFF};  // LDS #sp
    const uint16_t s = sp - 9;
    LDS_RTI[1] = hi(s);
    LDS_RTI[2] = lo(s);
    LDS_RTI[6] = cc & ~0x40;  // Forcibly clear X bit to accept #XIRQ.
    LDS_RTI[7] = b;
    LDS_RTI[8] = a;
    LDS_RTI[9] = hi(x);
    LDS_RTI[10] = lo(x);
    LDS_RTI[11] = hi(y);
    LDS_RTI[12] = lo(y);
    LDS_RTI[13] = hi(pc);
    LDS_RTI[14] = lo(pc);

    if (show)
        Signals::resetCycles();
    Pins.execInst(LDS_RTI, sizeof(LDS_RTI));
    if (show)
        Signals::printCycles();
}

void Regs::capture(const Signals *stack) {
    sp = stack[0].addr;
    pc = uint16(stack[1].data, stack[0].data);
    y = uint16(stack[3].data, stack[2].data);
    x = uint16(stack[5].data, stack[4].data);
    a = stack[6].data;
    b = stack[7].data;
    cc = stack[8].data;
}

void Regs::printRegList() const {
    cli.println(F("?Reg: pc sp y x a b d cc"));
}

bool Regs::validUint8Reg(char reg) const {
    if (reg == 'a' || reg == 'b' || reg == 'c') {
        cli.print(reg);
        if (reg == 'c')
            cli.print('c');
        return true;
    }
    return false;
}

bool Regs::validUint16Reg(char reg) const {
    if (reg == 'p' || reg == 's' || reg == 'y' || reg == 'x' || reg == 'd') {
        cli.print(reg);
        if (reg == 'p')
            cli.print('c');
        if (reg == 's')
            cli.print('p');
        return true;
    }
    return false;
}

bool Regs::setRegValue(char reg, uint32_t value, State state) {
    if (state == State::CLI_CANCEL)
        return true;
    if (state == State::CLI_DELETE) {
        cli.backspace(reg == 'p' || reg == 's' || reg == 'c' ? 3 : 2);
        return false;
    }
    cli.println();
    switch (reg) {
    case 'p':
        pc = value;
        break;
    case 's':
        sp = value;
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
    print();
    return true;
}

void Memory::setRamDevBase(uint16_t ram, uint16_t dev) {
    ram_base = ram;
    dev_base = dev;
}

void Memory::disableCOP() {
    // Disable watchdog timer.
    static const uint8_t DISABLE_COP[] = {
            0x86, 0x04, 0x97, 0x3F, 0};  // LDAA #NOCOP_bm; STAA dir[CONFIG]
    Pins.execInst(DISABLE_COP, sizeof(DISABLE_COP));
}

void Memory::setINIT() {
    // Move internal devices ana RAM base address.
    static uint8_t SET_INIT[] = {0x86, 0, 0x97, 0x3D, 0}; // LDAA #INIT_gc; STAA dir[INIT]
    SET_INIT[1] = static_cast<uint8_t>(ram_base >> 8 & 0xF0) |
                  static_cast<uint8_t>(dev_base >> 12);
    Pins.execInst(SET_INIT, sizeof(SET_INIT));
}

uint8_t Memory::internal_read(uint16_t addr) const {
    static uint8_t LDAA_STAA[4 + 3] = {
            0xB6, 0, 0, 0, 0xB7, 0x01, 0x00};  // LDAA ext[addr], STAA $0100
    LDAA_STAA[1] = hi(addr);
    LDAA_STAA[2] = lo(addr);
    Pins.captureWrites(LDAA_STAA, sizeof(LDAA_STAA), nullptr, &LDAA_STAA[3], 1);
    return LDAA_STAA[3];
}

void Memory::internal_write(uint16_t addr, uint8_t data) const {
    static uint8_t LDAA_STAA[2 + 4] = {
            0x86, 0, 0xB7, 0, 0, 0};  // LDAA #val, STA ext[addr]
    LDAA_STAA[1] = data;
    LDAA_STAA[3] = hi(addr);
    LDAA_STAA[4] = lo(addr);
    Pins.execInst(LDAA_STAA, sizeof(LDAA_STAA));
}

bool Memory::is_internal(uint16_t addr) const {
    if (addr >= ram_base + ram_start && addr < ram_base + ram_end)
        return true;
    if (addr >= dev_base + dev_start && addr < dev_base + dev_end)
        return true;
    return false;
}

uint8_t Memory::read(uint16_t addr) const {
    return is_internal(addr) ? internal_read(addr) : raw_read(addr);
}

void Memory::write(uint16_t addr, uint8_t data) {
    if (is_internal(addr)) {
        internal_write(addr, data);
    } else {
        raw_write(addr, data);
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
