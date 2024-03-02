#include "regs_i8085.h"
#include "debugger.h"
#include "digital_bus.h"
#include "inst_i8085.h"
#include "mems_i8085.h"
#include "pins_i8085.h"
#include "string_util.h"

namespace debugger {
namespace i8085 {

struct RegsI8085 Regs;

const char *RegsI8085::cpu() const {
    return "i8085";
}

const char *RegsI8085::cpuName() const {
    return "P8085";
}

void RegsI8085::print() const {
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
    outHex16(buffer + 3, _pc);
    outHex16(buffer + 11, _sp);
    outHex8(buffer + 19, _b);
    outHex8(buffer + 21, _c);
    outHex8(buffer + 27, _d);
    outHex8(buffer + 29, _e);
    outHex8(buffer + 35, _h);
    outHex8(buffer + 37, _l);
    outHex8(buffer + 42, _a);
    buffer[49] = bit1(_psw & 0x80, 'S');
    buffer[50] = bit1(_psw & 0x40, 'Z');
    buffer[51] = bit1(_psw & 0x20, '1');
    buffer[52] = bit1(_psw & 0x10, 'H');
    buffer[53] = bit1(_psw & 0x08, '1');
    buffer[54] = bit1(_psw & 0x04, 'P');
    buffer[55] = bit1(_psw & 0x02, '1');
    buffer[56] = bit1(_psw & 0x01, 'C');
    buffer[57] = _ie ? ' ' : 0;
    cli.println(buffer);
    Pins.idle();
}

void RegsI8085::save() {
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
    Pins.captureWrites(
            PUSH_ALL, sizeof(PUSH_ALL), &_sp, buffer, sizeof(buffer));
    _sp++;
    _pc = be16(buffer) - 1;  // offser RST instruction
    _a = buffer[2];
    _psw = buffer[3];
    _b = buffer[4];
    _c = buffer[5];
    _d = buffer[6];
    _e = buffer[7];
    _h = buffer[8];
    _l = buffer[9];
    _ie = (buffer[10] & 0x08) != 0;
}

void RegsI8085::restore() {
    static uint8_t POP_ALL[] = {
            0x31, 0, 0,  // LXI SP,sp-10
            0xE1, 0, 0,  // POP H
            0xD1, 0, 0,  // POP D
            0xC1, 0, 0,  // POP B
            0xF1, 0, 0,  // POP PSW
            0,           // EI/DI
            0xC9, 0, 0,  // RET
    };
    POP_ALL[1] = lo(_sp - 10);
    POP_ALL[2] = hi(_sp - 10);
    POP_ALL[4] = _l;
    POP_ALL[5] = _h;
    POP_ALL[7] = _e;
    POP_ALL[8] = _d;
    POP_ALL[10] = _c;
    POP_ALL[11] = _b;
    POP_ALL[13] = _psw;
    POP_ALL[14] = _a;
    POP_ALL[15] = _ie ? InstI8085::EI : InstI8085::DI;
    POP_ALL[17] = lo(_pc);
    POP_ALL[18] = hi(_pc);
    Pins.execInst(POP_ALL, sizeof(POP_ALL));
}

uint8_t RegsI8085::read_io(uint8_t addr) const {
    static uint8_t IN[] = {
        0xDB, 0,                // IN addr
        0x32, 0, 0,             // STA 0
    };
    IN[1] = addr;
    uint8_t data;
    Pins.captureWrites(IN, sizeof(IN), nullptr, &data, sizeof(data));
    return data;
}

void RegsI8085::write_io(uint8_t addr, uint8_t data) const {
    static uint8_t OUT[] = {
        0x3E, 0,                // MVI data
        0xD3, 0,                // OUT addr
    };
    OUT[1] = data;
    OUT[3] = addr;
    Pins.execInst(OUT, sizeof(OUT));
}

void RegsI8085::helpRegisters() const {
    cli.println(F("?Reg: PC SP BC DE HL A B C D E H L PSW"));
}

constexpr const char *REGS8[] = {
        "PSW",  // 1
        "A",    // 2
        "B",    // 3
        "C",    // 4
        "D",    // 5
        "E",    // 6
        "H",    // 7
        "L",    // 8
};
constexpr const char *REGS16[] = {
        "PC",  // 9
        "SP",  // 10
        "BC",  // 11
        "DE",  // 12
        "HL",  // 13
};

const Regs::RegList *RegsI8085::listRegisters(uint8_t n) const {
    static constexpr RegList REG_LIST[] = {
            {REGS8, 8, 1, UINT8_MAX},
            {REGS16, 5, 9, UINT16_MAX},
    };
    return n < 2 ? &REG_LIST[n] : nullptr;
}

void RegsI8085::setRegister(uint8_t reg, uint32_t value) {
    switch (reg) {
    case 9:
        _pc = value;
        break;
    case 10:
        _sp = value;
        break;
    case 11:
        _bc(value);
        break;
    case 12:
        _de(value);
        break;
    case 13:
        _hl(value);
        break;
    case 2:
        _a = value;
        break;
    case 3:
        _b = value;
        break;
    case 4:
        _c = value;
        break;
    case 5:
        _d = value;
        break;
    case 6:
        _e = value;
        break;
    case 7:
        _h = value;
        break;
    case 8:
        _l = value;
        break;
    case 1:
        _psw = value;
        break;
    }
}

}  // namespace i8085
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
