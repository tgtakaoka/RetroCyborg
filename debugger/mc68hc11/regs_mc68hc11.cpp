#include "regs_mc68hc11.h"
#include "debugger.h"
#include "devs_mc68hc11.h"
#include "digital_bus.h"
#include "mc68hc11_init.h"
#include "mems_mc68hc11.h"
#include "pins_mc68hc11.h"
#include "string_util.h"

namespace debugger {
namespace mc68hc11 {

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
 *        ; ABA  ($1B) on MC6801
 *        ; XGDX ($18) on HD6301
 *        ; prefix     on MC68HC11
 *   FCB  $08
 *        ; INX on MC6800/MC6801/HD6301
 *        ; INY on MC68HC11
 *   A=$10, X=$55AB: MC6800
 *   A=110, X=$55AB: MC6801
 *   A=$55, X=$0A65: HD6301
 *   A=10,  X=$55AA: MC68HC11
 *
 * MC6800/MB8861(MB8870)
 *   LDX  #$FFFF
 *   FCB  $EC, $01
 *        ; CPX 1,X ($AC $01, 6 clcoks) on MC6800
 *        ; ADX #1  ($EC $01, 2 clocks) on MB8861
 * X=$FFFF: MC6800
 * X=$0000: MB8861
 */

SoftwareType RegsMc68hc11::checkSoftwareType() {
    return _type = SW_MC68HC11;
}

const char *RegsMc68hc11::cpuName() const {
    return Debugger.target().identity();
}

void RegsMc68hc11::print() const {
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
    outHex16(buffer + 3, _pc);
    outHex16(buffer + 11, _sp);
    outHex16(buffer + 18, _x);
    outHex16(buffer + 25, _y);
    outHex8(buffer + 32, _a);
    outHex8(buffer + 37, _b);
    auto *p = buffer + 43;
    *p++ = bit1(_cc & 0x80, 'S');
    *p++ = bit1(_cc & 0x40, 'X');
    *p++ = bit1(_cc & 0x20, 'H');
    *p++ = bit1(_cc & 0x10, 'I');
    *p++ = bit1(_cc & 0x08, 'N');
    *p++ = bit1(_cc & 0x04, 'Z');
    *p++ = bit1(_cc & 0x02, 'V');
    *p++ = bit1(_cc & 0x01, 'C');
    cli.println(buffer);
}

void RegsMc68hc11::save() {
    static const uint8_t SWI = 0x3F;  // 1:N:w:W:W:W:W:W:W:W:W:n:V:v
    _pins->injectReads(&SWI, sizeof(SWI));
    uint8_t context[9];
    _pins->captureWrites(context, sizeof(context), &_sp);
    // Capturing writes to stack in little endian order.
    _pc = le16(context) - 1;  //  offset SWI instruction.
    _y = le16(context + 2);
    _x = le16(context + 4);
    _a = context[6];
    _b = context[7];
    _cc = context[8];
    // Read SWI vector
    context[0] = 0;  // irrelevant data
    context[1] = hi(_pc);
    context[2] = lo(_pc);
    _pins->injectReads(context, 3);
}

void RegsMc68hc11::restore() {
    uint8_t LDS_RTI[3 + 12];
    _cc &= ~0x40;  // clear X bit to enable #XIRQ for step/suspend
    const uint16_t s = _sp - 9;
    LDS_RTI[0] = 0x8E;  // LDS #sp ; 1:2:3
    LDS_RTI[1] = hi(s);
    LDS_RTI[2] = lo(s);
    LDS_RTI[3] = 0x3B;  // RTI     ; 1:N:n:R:r:r:r:r:r:r:r:r
    LDS_RTI[6] = _cc;
    LDS_RTI[7] = _b;
    LDS_RTI[8] = _a;
    LDS_RTI[9] = hi(_x);
    LDS_RTI[10] = lo(_x);
    LDS_RTI[11] = hi(_y);
    LDS_RTI[12] = lo(_y);
    LDS_RTI[13] = hi(_pc);
    LDS_RTI[14] = lo(_pc);
    _pins->injectReads(LDS_RTI, sizeof(LDS_RTI));
}

uint16_t RegsMc68hc11::capture(const mc6800::Signals *stack, bool step) {
    _sp = stack->addr;
    _pc = uint16(stack->next(1)->data, stack->data);
    if (!step)
        --_pc;
    _y = uint16(stack->next(3)->data, stack->next(2)->data);
    _x = uint16(stack->next(5)->data, stack->next(4)->data);
    _a = stack->next(6)->data;
    _b = stack->next(7)->data;
    _cc = stack->next(8)->data;
    return _pc;
}

void RegsMc68hc11::helpRegisters() const {
    cli.println(F("?Reg: PC SP X Y A B D CC"));
}

constexpr const char *REGS8[] = {
        "A",   // 1
        "B",   // 2
        "CC",  // 3
};
constexpr const char *REGS16[] = {
        "PC",  // 4
        "SP",  // 5
        "X",   // 6
        "Y",   // 7
        "D",   // 8
};

const Regs::RegList *RegsMc68hc11::listRegisters(uint8_t n) const {
    static constexpr RegList REG_LIST[] = {
            {REGS8, 3, 1, UINT8_MAX},
            {REGS16, 3, 5, UINT16_MAX},
    };
    return n < 2 ? &REG_LIST[n] : nullptr;
}

void RegsMc68hc11::setRegister(uint8_t reg, uint32_t value) {
    switch (reg) {
    case 4:
        _pc = value;
        break;
    case 5:
        _sp = value;
        break;
    case 6:
        _x = value;
        break;
    case 1:
        _a = value;
        break;
    case 2:
        _b = value;
        break;
    case 3:
        _cc = value;
        break;
    case 7:
        _y = value;
        break;
    case 8:
        _d(value);
        break;
    }
}

uint8_t RegsMc68hc11::internal_read(uint16_t addr) const {
    uint8_t LDAA[3];
    LDAA[0] = 0xB6;  // LDAA addr ; 1:2:3:A
    LDAA[1] = hi(addr);
    LDAA[2] = lo(addr);
    _pins->injectReads(LDAA, sizeof(LDAA), 4);
    static constexpr uint8_t STAA_FF00[] = {0xB7, 0xFF, 0x00};
    const auto staa = _mems->is_internal(0x8000) ? STAA_FF00 : STAA_8000;
    _pins->injectReads(staa, sizeof(STAA_8000));
    uint8_t data;
    _pins->captureWrites(&data, sizeof(data));
    return data;
}

void RegsMc68hc11::internal_write(uint16_t addr, uint8_t data) const {
    uint8_t LDAA_STAA[2 + 3];
    LDAA_STAA[0] = 0x86;  // LDAA #val ; 1:2
    LDAA_STAA[1] = data;
    LDAA_STAA[2] = 0xB7;  // STAA addr ; 1:2:3:B
    LDAA_STAA[3] = hi(addr);
    LDAA_STAA[4] = lo(addr);
    _pins->injectReads(LDAA_STAA, sizeof(LDAA_STAA), 6);
}

}  // namespace mc68hc11
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
