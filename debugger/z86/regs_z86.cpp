#include "regs_z86.h"
#include "debugger.h"
#include "digital_bus.h"
#include "mems_z86.h"
#include "pins_z86.h"
#include "string_util.h"

namespace debugger {
namespace z86 {

struct RegsZ86 Regs;

const char *RegsZ86::cpu() const {
    return "Z86C";
}

const char *RegsZ86::cpuName() const {
    return "Z86C91";
}

void RegsZ86::print() const {
    // clang-format off
    static char buffer[] = {
        'P', 'C', '=', 0, 0, 0, 0, ' ',  // PC=3
        'S', 'P', '=', 0, 0, 0, 0, ' ',  // SP=11
        'R', 'P', '=', 0, 0, ' ',        // RP=19
        'F', '=',                        // S=24
        0, 0, 0, 0, 0, 0, 0, 0,
        0,
    };
    // clang-format on
    outHex16(buffer + 3, _pc);
    outHex8(buffer + 11, get_sfr(sfr_sph));
    outHex8(buffer + 13, get_sfr(sfr_spl));
    outHex8(buffer + 19, get_sfr(sfr_rp));
    const auto flags = get_sfr(sfr_flags);
    constexpr char *flags_bits = buffer + 24;
    flags_bits[0] = bit1(flags & 0x80, 'C');
    flags_bits[1] = bit1(flags & 0x40, 'Z');
    flags_bits[2] = bit1(flags & 0x20, 'S');
    flags_bits[3] = bit1(flags & 0x10, 'V');
    flags_bits[4] = bit1(flags & 0x08, 'D');
    flags_bits[5] = bit1(flags & 0x04, 'H');
    flags_bits[6] = bit1(flags & 0x02, '2');
    flags_bits[7] = bit1(flags & 0x01, '1');
    cli.println(buffer);
    // clang-format off
    static char line1[] = {
        ' ', 'R', '0', '=', 0, 0, ' ', // Rn=4+7n
        ' ', 'R', '1', '=', 0, 0, ' ',
        ' ', 'R', '2', '=', 0, 0, ' ',
        ' ', 'R', '3', '=', 0, 0, ' ',
        ' ', 'R', '4', '=', 0, 0, ' ',
        ' ', 'R', '5', '=', 0, 0, ' ',
        ' ', 'R', '6', '=', 0, 0, ' ',
        ' ', 'R', '7', '=', 0, 0,
        0,
    };
    // clang-format on
    for (auto i = 0; i < 8; i++)
        outHex8(line1 + 4 + i * 7, _r[i]);
    cli.println(line1);
    // clang-format off
    static char line2[] = {
        ' ', 'R', '8', '=', 0, 0, ' ', // Rn=4+7(n-8)
        ' ', 'R', '9', '=', 0, 0, ' ',
        'R', '1', '0', '=', 0, 0, ' ',
        'R', '1', '1', '=', 0, 0, ' ',
        'R', '1', '2', '=', 0, 0, ' ',
        'R', '1', '3', '=', 0, 0, ' ',
        'R', '1', '4', '=', 0, 0, ' ',
        'R', '1', '5', '=', 0, 0,
        0,
    };
    // clang-format on
    for (auto i = 8; i < 16; i++)
        outHex8(line2 + 4 + (i - 8) * 7, _r[i]);
    cli.println(line2);
}

uint8_t RegsZ86::read_reg(uint8_t addr) {
    static uint8_t READ_REG[] = {
            0xF8, 0,     // LD r15,Rn
            0x92, 0xFE,  // LDE @rr14,r15
    };
    READ_REG[1] = addr;
    uint8_t val;
    Pins.captureWrites(READ_REG, sizeof(READ_REG), nullptr, &val, sizeof(val));
    restore_r(15);
    if (in_rp(addr))
        _r[addr & 0xF] = val;
    return val;
}

void RegsZ86::write_reg(uint8_t addr, uint8_t val) {
    static uint8_t WRITE_REG[] = {
            0xE6, 0, 0,  // LD Rn,IM
    };
    WRITE_REG[1] = addr;
    WRITE_REG[2] = val;
    Pins.execInst(WRITE_REG, sizeof(WRITE_REG));
    if (in_rp(addr))
        _r[addr & 0xF] = val;
}

void RegsZ86::reset() {
    constexpr auto P3M = 247;
    constexpr auto P01M = 248;
    write_reg(P3M, Z8_DATA_MEMORY << 3);
    write_reg(P01M, 0x92 | (Z8_INTERNAL_STACK << 2));
    static constexpr uint8_t JP_RESET[] = {0x8D, 0x00, 0x0C}; // JP %000C
    Pins.execInst(JP_RESET, sizeof(JP_RESET));
}

void RegsZ86::save_r(uint8_t num) {
    static uint8_t SAVE_R[] = {
            0x92, 0x0E,  // LDE @rr14, rn
    };
    SAVE_R[1] = (num << 4) | 14;  // LDE @rr14,ri
    Pins.captureWrites(
            SAVE_R, sizeof(SAVE_R), nullptr, &_r[num], sizeof(_r[0]));
}

void RegsZ86::save() {
    const auto *signals = Pins.cycle(0xFF);  // NOP
    _pc = signals->addr;

    for (auto num = 0; num < 16; num++)
        save_r(num);
    for (auto num = 0; num < 4; num++)
        _sfr[num] = read_reg(sfr_base + num);
    restore_r(15);
}

void RegsZ86::restore_r(uint8_t num) {
    static uint8_t LOAD_R[] = {
            0x0C, 0,  // LD r,IM
    };
    LOAD_R[0] = 0x0C | (num << 4);
    LOAD_R[1] = _r[num];
    Pins.execInst(LOAD_R, sizeof(LOAD_R));
}

void RegsZ86::restore() {
    static uint8_t JP[] = {
            0x8D, 0, 0,  // JP DA
    };
    for (auto num = 0; num < 4; num++)
        write_reg(sfr_base + num, _sfr[num]);
    for (auto num = 0; num < 16; num++)
        restore_r(num);

    JP[1] = hi(_pc);
    JP[2] = lo(_pc);
    Pins.execInst(JP, sizeof(JP));
}

void RegsZ86::set_rp(uint8_t val) {
    set_sfr(sfr_rp, val);
    for (auto num = 0; num < 16; num++)
        save_r(num);
}

void RegsZ86::set_sfr(uint8_t name, uint8_t val) {
    const auto num = name - sfr_base;
    _sfr[num] = val;
    write_reg(name, val);
}

void RegsZ86::set_r(uint8_t num, uint8_t val) {
    _r[num] = val;
    restore_r(num);
}

void RegsZ86::helpRegisters() const {
    cli.println(F("?Reg: PC SP RP FLAGS R0~R15 RR0~14"));
}

constexpr const char *REGS8[] = {
        "R0",     // 1
        "R1",     // 2
        "R2",     // 3
        "R3",     // 4
        "R4",     // 5
        "R5",     // 6
        "R6",     // 7
        "R7",     // 8
        "R8",     // 9
        "R9",     // 10
        "R10",    // 11
        "R11",    // 12
        "R12",    // 13
        "R13",    // 14
        "R14",    // 15
        "R15",    // 16
        "FLAGS",  // 17
        "RP",     // 18
};
constexpr const char *REGS16[] = {
        "RR0",   // 19
        "RR2",   // 20
        "RR4",   // 21
        "RR6",   // 22
        "RR8",   // 23
        "RR10",  // 24
        "RR12",  // 25
        "RR14",  // 26
        "PC",    // 27
        "SP",    // 28
};

const Regs::RegList *RegsZ86::listRegisters(uint8_t n) const {
    static constexpr RegList REG_LIST[] = {
            {REGS8, 18, 1, UINT8_MAX},
            {REGS16, 10, 19, UINT16_MAX},
    };
    return n < 2 ? &REG_LIST[n] : nullptr;
}

void RegsZ86::setRegister(uint8_t reg, uint32_t value) {
    switch (reg) {
    case 27:
        _pc = value;
        break;
    case 28:
        set_sph(hi(value));
        set_spl(lo(value));
        break;
    case 18:
        set_rp(value);
        break;
    case 17:
        set_flags(value);
        break;
    default:
        if (reg >= 19) {
            set_rr((reg - 19U) * 2, value);
        } else {
            set_r(reg - 1U, value);
        }
        break;
    }
}

}  // namespace z86
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
