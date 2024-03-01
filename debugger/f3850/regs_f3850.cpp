#include "debugger.h"

#include <asm_f3850.h>
#include <dis_f3850.h>

#include "devs_f3850.h"
#include "digital_bus.h"
#include "i8251.h"
#include "mems_f3850.h"
#include "pins_f3850.h"
#include "regs_f3850.h"
#include "string_util.h"

#define LOG_ROMC(e)
//#define LOG_ROMC(e) e

namespace debugger {
namespace f3850 {

struct RegsF3850 Regs;

const char *RegsF3850::cpu() const {
    return "F3850";
}

bool RegsF3850::romc_read(Signals *signals) {
    LOG_ROMC(cli.print(F("@@ R ROMC=")));
    LOG_ROMC(cli.printHex(signals->romc, 2));
    LOG_ROMC(cli.print(F(" pc0=")));
    LOG_ROMC(cli.printHex(_pc0, 4));
    LOG_ROMC(cli.print(F(" dc0=")));
    LOG_ROMC(cli.printlnHex(_dc0, 4));
    switch (signals->romc()) {
    case 0x00:
        signals->markFetch();
        if (signals->readMemory())
            signals->data = Memory.read(_pc0);
        signals->markRead(_pc0++);
        break;
    case 0x01:
        if (signals->readMemory())
            signals->data = Memory.read(_pc0);
        signals->markRead(_pc0);
        _pc0 += static_cast<int8_t>(signals->data);
        break;
    case 0x02:
        if (signals->readMemory())
            signals->data = Memory.read(_dc0);
        signals->markRead(_dc0++);
        break;
    case 0x03:
        if (signals->readMemory())
            signals->data = Memory.read(_pc0);
        signals->markRead(_pc0++);
        _ioaddr = signals->data;
        break;
    case 0x06:
        signals->data = hi(_dc0);
        break;
    case 0x07:
        signals->data = hi(_pc1);
        break;
    case 0x09:
        signals->data = lo(_dc0);
        break;
    case 0x0B:
        signals->data = lo(_pc1);
        break;
    case 0x0C:
        if (signals->readMemory())
            signals->data = Memory.read(_pc0);
        signals->markRead(_pc0);
        _pc0 = uint16(hi(_pc0), signals->data);
        break;
    case 0x0E:
        if (signals->readMemory())
            signals->data = Memory.read(_pc0);
        signals->markRead(_pc0);
        _dc0 = uint16(hi(_dc0), signals->data);
        break;
    case 0x0F:  // Interruput acknowledge
        signals->data = lo(Devs.vector());
        _pc1 = _pc0;
        _pc0 = uint16(hi(_pc0), signals->data);
        break;
    case 0x11:
        if (signals->readMemory())
            signals->data = Memory.read(_pc0);
        signals->markRead(_pc0);
        _dc0 = uint16(signals->data, lo(_dc0));
        break;
    case 0x13:  // Interruput acknowledge
        signals->data = hi(Devs.vector());
        _pc0 = uint16(signals->data, lo(_pc0));
        break;
    case 0x1B:  // I/O read
        if (_ioaddr < 4)
            return false;
        if (Devs.isSelected(_ioaddr)) {
            signals->data = Devs.read(_ioaddr);
        } else {
            signals->data = 0;
        }
        signals->markIoRead(_ioaddr);
        break;
    case 0x1E:
        signals->data = lo(_pc0);
        break;
    case 0x1F:
        signals->data = hi(_pc0);
        break;
    default:
        _delay = hi(_delay);
        signals->data = 0;
        return false;
    }
    return true;
}

bool RegsF3850::romc_write(Signals *signals) {
    LOG_ROMC(cli.print(F("@@ W ROMC=")));
    LOG_ROMC(cli.printHex(signals->romc, 2));
    LOG_ROMC(cli.print(F(" pc0=")));
    LOG_ROMC(cli.printHex(_pc0, 4));
    LOG_ROMC(cli.print(F(" dc0=")));
    LOG_ROMC(cli.printlnHex(_dc0, 4));
    switch (signals->romc()) {
    case 0x04:
        _pc0 = _pc1;
        return false;
    case 0x05:
        if (signals->writeMemory())
            Memory.write(_dc0, signals->data);
        signals->markWrite(_dc0++);
        break;
    case 0x08:
        _pc1 = _pc0;
        _pc0 = uint16(signals->data, signals->data);
        break;
    case 0x0A:
        _dc0 += static_cast<int8_t>(signals->data);
        break;
    case 0x0D:
        _pc1 = _pc0 + 1;
        ++_delay;
        return false;
    case 0x12:
        _pc1 = _pc0;
        _pc0 = uint16(hi(_pc0), signals->data);
        break;
    case 0x14:
        _pc0 = uint16(signals->data, lo(_pc0));
        break;
    case 0x15:
        _pc1 = uint16(signals->data, lo(_pc1));
        break;
    case 0x16:
        _dc0 = uint16(signals->data, lo(_dc0));
        break;
    case 0x17:
        _pc0 = uint16(hi(_pc0), signals->data);
        break;
    case 0x18:
        _pc1 = uint16(hi(_pc1), signals->data);
        break;
    case 0x19:
        _dc0 = uint16(hi(_dc0), signals->data);
        break;
    case 0x1A:  // I/O write
        if (_ioaddr < 4)
            return false;
        if (Devs.isSelected(_ioaddr))
            Devs.write(_ioaddr, signals->data);
        signals->markIoWrite(_ioaddr);
        break;
    case 0x1C:
        _ioaddr = signals->data;
        _delay = hi(_delay);
        break;
    case 0x1D: {
        const auto tmp = _dc0;
        _dc0 = _dc1;
        _dc1 = tmp;
        return false;
    }
    default:

        return false;
    }
    return true;
}

void RegsF3850::print() const {
    // P0=XXXX P=XXXX DC=XXXX DC1=XXXX IS=XX W=_____ A=XX
    // R0=XX R1=XX R2=XX R3=XX R4=XX R5=XX R6=XX R7=XX
    // R8=XX  J=XX HU=XX HL=XX KU=XX KL=XX QU=XX QL=XX
    // clang-format off
    static char regs1[] = {
        'P', '0', '=', 0, 0, 0, 0, ' ',      // P0=3
        'P', '=', 0, 0, 0, 0, ' ',           // P=10
        'D', 'C', '=', 0, 0, 0, 0, ' ',      // DC=18
        'D', 'C', '1', '=', 0, 0, 0, 0, ' ', // DC1=27
        'I', 'S', '=', 0, 0, ' ',            // IS=35
        'W', '=', 0, 0, 0, 0, 0, ' ',        // W=40
        'A', '=', 0, 0,                      // A=48
        0,
    };
    static char regs2[] = {
        // line1=0
        'R', '0', '=', 0, 0, ' ', // R0=3
        'R', '1', '=', 0, 0, ' ', // R1=9
        'R', '2', '=', 0, 0, ' ', // R2=15
        'R', '3', '=', 0, 0, ' ', // R3=21
        'R', '4', '=', 0, 0, ' ', // R4=27
        'R', '5', '=', 0, 0, ' ', // R5=33
        'R', '6', '=', 0, 0, ' ', // R6=39
        'R', '7', '=', 0, 0, 0,   // R7=45
        // line2=48
        'R', '8', '=', 0, 0, ' ', // R8=51
        ' ', 'J', '=', 0, 0, ' ', // J=57
        'H', 'U', '=', 0, 0, ' ', // HU=63
        'H', 'L', '=', 0, 0, ' ', // HL=69
        'K', 'U', '=', 0, 0, ' ', // KU=75
        'K', 'L', '=', 0, 0, ' ', // KL=81
        'Q', 'U', '=', 0, 0, ' ', // QU=87
        'Q', 'L', '=', 0, 0, 0,   // QL=93
    };
    // clang-format on
    outHex16(regs1 + 3, _pc0);
    outHex16(regs1 + 10, _pc1);
    outHex16(regs1 + 18, _dc0);
    outHex16(regs1 + 27, _dc1);
    outHex4(regs1 + 35, (_isar >> 3) & 7);
    outHex4(regs1 + 36, _isar & 7);
    regs1[40] = bit1(_w & 0x10, 'I');  // Interrupt master enable #ICB
    regs1[41] = bit1(_w & 0x08, 'V');  // Overflow
    regs1[42] = bit1(_w & 0x04, 'Z');  // Zero
    regs1[43] = bit1(_w & 0x02, 'C');  // Carry
    regs1[44] = bit1(_w & 0x01, 'S');  // Sign
    outHex8(regs1 + 48, _a);
    cli.println(regs1);
    for (auto n = 0; n < 16; ++n)
        outHex8(regs2 + n * 6 + 3, _r[n]);
    cli.println(regs2);
    cli.println(regs2 + 48);
}

void RegsF3850::save() {
    static constexpr uint8_t SAVE_A[] = {
            0x17,  // ST ; (DC)<-A
    };
    static constexpr uint8_t SAVE_ISAR[] = {
            0x0A,  // LR A,IS
            0x17,  // ST
    };
    static constexpr uint8_t SAVE_REGS[] = {
            0x40, 0x17,  // LR A,0; ST
            0x41, 0x17,  // LR A,1; ST
            0x42, 0x17,  // LR A,2; ST
            0x43, 0x17,  // LR A,3; ST
            0x44, 0x17,  // LR A,4; ST
            0x45, 0x17,  // LR A,5; ST
            0x46, 0x17,  // LR A,6; ST
            0x47, 0x17,  // LR A,7; ST
            0x48, 0x17,  // LR A,8; ST
            0x49, 0x17,  // LR A,J; ST
            0x4A, 0x17,  // LR A,HU; ST
            0x4B, 0x17,  // LR A,HL; ST
            0x00, 0x17,  // LR A,KU; ST
            0x01, 0x17,  // LR A,KL; ST
            0x02, 0x17,  // LR A,QU; ST
            0x03, 0x17,  // LR A,QL; ST
    };
    static constexpr uint8_t SAVE_W[] = {
            0x1E,  // LR J,W
            0x49,  // LR A,J
            0x17,  // ST
    };
    const auto pc = _pc0;  // save PC0
    const auto dc = _dc0;  // save DC0
    Pins.captureWrites(SAVE_A, sizeof(SAVE_A), &_a, sizeof(_a));
    Pins.captureWrites(SAVE_ISAR, sizeof(SAVE_ISAR), &_isar, sizeof(_isar));
    Pins.captureWrites(SAVE_REGS, sizeof(SAVE_REGS), _r, sizeof(_r));
    Pins.captureWrites(SAVE_W, sizeof(SAVE_W), &_w, sizeof(_w));
    _pc0 = pc;  // restore PC0
    _dc0 = dc;  // restore DC
}

void RegsF3850::restore() {
    uint8_t LOAD_ALL[] = {
            0x20, 0, 0x59, 0x1D,  // LI w;  LR J,A; LR W,J
            0x20, 0, 0x59,        // LI j;  LR J,A
            0x60,                 // LISU isu
            0x68,                 // LISL isl
            0x20, 0,              // LI a
    };
    const auto pc = _pc0;  // save PC0
    LOAD_ALL[1] = _w;
    LOAD_ALL[5] = _r[9];  // J
    LOAD_ALL[7] = 0x60 | isu();
    LOAD_ALL[8] = 0x68 | isl();
    LOAD_ALL[10] = _a;
    Pins.execInst(LOAD_ALL, sizeof(LOAD_ALL));
    _pc0 = pc;  // restire PC0
}

void RegsF3850::set_isl(uint8_t val) {
    _isar &= ~07;
    _isar |= val;
}

void RegsF3850::set_isu(uint8_t val) {
    _isar &= ~070;
    _isar |= (val << 3);
}

void RegsF3850::update_r(uint8_t num, uint8_t val) {
    _r[num] = val;
    write_reg(num, val);
}

uint8_t RegsF3850::read_reg(uint8_t addr) {
    uint8_t READ_REG[] = {
            0x60,  // LISU isu
            0x68,  // LISL isl
            0x4C,  // LD A,S
            0x17,  // ST
    };
    uint8_t val;
    READ_REG[0] = 0x60 | ((addr >> 3) & 7);
    READ_REG[1] = 0x68 | (addr & 7);
    const auto pc = _pc0;  // save PC0
    const auto dc = _dc0;  // save DC0
    Pins.captureWrites(READ_REG, sizeof(READ_REG), &val, sizeof(val));
    _pc0 = pc;
    _dc0 = dc;
    return val;
}

void RegsF3850::write_reg(uint8_t addr, uint8_t val) {
    uint8_t WRITE_REG[] = {
            0x60,     // LISU isu
            0x68,     // LISL isl
            0x20, 0,  // LI val
            0x5C,     // LD S,A
    };
    WRITE_REG[0] = 0x60 | ((addr >> 3) & 7);
    WRITE_REG[1] = 0x68 | (addr & 7);
    WRITE_REG[3] = val;
    const auto pc = _pc0;  // save PC0
    Pins.execInst(WRITE_REG, sizeof(WRITE_REG));
    _pc0 = pc;
}

uint8_t RegsF3850::read_io(uint8_t addr) {
    if (addr < 2) {
        uint8_t READ_IO[] = {
                0xA0, 0x17,  // INS p; ST
        };
        const auto pc = _pc0;  // save PC0
        const auto dc = _dc0;  // save DC0
        uint8_t val;
        READ_IO[0] = 0xA0 | addr;
        Pins.captureWrites(READ_IO, sizeof(READ_IO), &val, sizeof(val));
        _pc0 = pc;
        _dc0 = dc;
        return val;
    }
    if (Devs.isSelected(addr))
        return Devs.read(addr);
    return 0;
}

void RegsF3850::write_io(uint8_t addr, uint8_t val) {
    if (addr < 2) {
        uint8_t WRITE_IO[] = {
                0x20, 0, 0xB0,  // LI d; OUTS p
        };
        const auto pc = _pc0;  // save PC0
        WRITE_IO[1] = val;
        WRITE_IO[2] = 0xB0 | addr;
        Pins.execInst(WRITE_IO, sizeof(WRITE_IO));
        _pc0 = pc;
        return;
    }
    if (Devs.isSelected(addr))
        Devs.write(addr, val);
}

void RegsF3850::helpRegisters() const {
    cli.println(
            F("?Reg: P0/PC P1/P DC DC1 H K Q IS W A R0-R8 J {H,K,Q,IS}{U,L}"));
}

constexpr const char *REGS3[] = {
        "ISU",  // 1
        "ISL",  // 2
};
constexpr const char *REGS6[] = {
        "IS",  // 3
        "W",   // 4
};
constexpr const char *REGS8[] = {
        "A",   // 5
        "R0",  // 6
        "R1",  // 7
        "R2",  // 8
        "R3",  // 9
        "R4",  // 10
        "R5",  // 11
        "R6",  // 12
        "R7",  // 13
        "R8",  // 14
        "J",   // 15, R9
        "HU",  // 16, R10
        "HL",  // 17, R11
        "KU",  // 18, R12
        "KL",  // 19, R13
        "QU",  // 10, R14
        "QL",  // 21, R15
};
constexpr const char *REGS16[] = {
        "P0",   // 22
        "PC",   // 23
        "P1",   // 24
        "P",    // 25
        "DC",   // 26
        "DC1",  // 27
        "H",    // 28
        "K",    // 29
        "Q",    // 30
};

const Regs::RegList *RegsF3850::listRegisters(uint8_t n) const {
    static constexpr RegList REG_LIST[] = {
            {REGS3, 2, 1, 0x7},
            {REGS6, 2, 3, 0x1F},
            {REGS8, 17, 5, UINT8_MAX},
            {REGS16, 9, 22, UINT16_MAX},
    };
    return n < 4 ? &REG_LIST[n] : nullptr;
}

void RegsF3850::setRegister(uint8_t reg, uint32_t value) {
    switch (reg) {
    case 22:
    case 23:
        _pc0 = value;
        break;
    case 24:
    case 25:
        _pc1 = value;
        break;
    case 26:
        _dc0 = value;
        break;
    case 27:
        _dc1 = value;
        break;
    case 28:
        update_r(HU, hi(value));
        update_r(HL, lo(value));
        break;
    case 29:
        update_r(KU, hi(value));
        update_r(KL, lo(value));
        break;
    case 30:
        update_r(QU, hi(value));
        update_r(QL, lo(value));
        break;
    case 5:
        _a = value;
        break;
    case 4:
        _w = value;
        break;
    case 3:
        _isar = value;
        break;
    case 1:
        set_isu(value);
        break;
    case 2:
        set_isl(value);
        break;
    default:
        update_r(reg - 6U, value);
        break;
    }
}

}  // namespace f3850
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
