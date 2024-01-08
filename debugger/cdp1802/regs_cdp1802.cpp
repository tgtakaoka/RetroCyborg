#include "regs_cdp1802.h"
#include "debugger.h"
#include "digital_bus.h"
#include "inst_cdp1802.h"
#include "mems_cdp1802.h"
#include "pins_cdp1802.h"
#include "signals_cdp1802.h"
#include "string_util.h"

namespace debugger {
namespace cdp1802 {

struct RegsCdp1802 Regs;

/**
 * - CDP1802: 0x68 opcode causes illegal write on M(R(P)).
 * - CDP1804/CDP1804A:0x68 opcode causes double instruction fetch.
 * TODO: distinguish CDP1804 and CDP1804A
 */

static const char CDP1802[] = "CDP1802";
static const char CDP1804A[] = "CDP1804A";

void RegsCdp1802::setCpuType() {
    Pins.cycle(InstCdp1802::PREFIX);
    auto signals = Pins.prepareCycle();
    if (signals->fetch()) {
        _cpuType = CDP1804A;
        Pins.cycle(InstCdp1802::DADI);
        Pins.cycle(0);
        Pins.cycle();  // execution cycle
    } else {
        _cpuType = CDP1802;
        signals->capture();  // capture illegal write
        Pins.cycle();
    }
}

const char *RegsCdp1802::cpu() const {
    return _cpuType ? _cpuType : CDP1802;
}

const char *RegsCdp1802::cpuName() const {
    return cpu();
}

void RegsCdp1802::reset() {
    _cpuType = nullptr;
}

bool RegsCdp1802::is1804() const {
    return _cpuType == CDP1804A;
}

void RegsCdp1802::print() const {
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
    outHex8(line0 + 2, _d);
    outHex4(line0 + 8, _df);
    outHex4(line0 + 12, _x);
    outHex4(line0 + 16, _p);
    outHex8(line0 + 20, _t);
    outHex4(line0 + 25, _q);
    outHex4(line0 + 30, _ie);
    cli.println(line0);
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
        outHex16(line1 + 4 + i * 9, _r[i]);
    cli.println(line1);
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
        outHex16(line2 + 4 + (i - 8) * 9, _r[i]);
    cli.println(line2);
}

void RegsCdp1802::save() {
    static const uint8_t SAV[] = {0x78};
    // STR R0, STR R1, MARK, STR R3, ...
    static const uint8_t STR[] = {
            0x50, 0x51, 0x79, 0x53, 0x54, 0x55, 0x56, 0x57,  //
            0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,  //
    };

    Pins.captureWrites(SAV, sizeof(SAV), nullptr, &_t, sizeof(_t));
    for (auto i = 0; i < 16; i++) {
        uint8_t tmp;
        Pins.captureWrites(&STR[i], 1, &_r[i], &tmp, sizeof(tmp));
        if (i == 0)
            _d = tmp;
        if (i == 2) {  // MARK
            _x = tmp >> 4;
            _p = tmp & 0xF;
        }
        _dirty[i] = false;
    }
    _dirty[2] = true;                // becase of MARK
    _dirty[_p] = true;               // becase this is a program counter
    _r[_p] -= sizeof(SAV) + _p + 1;  // adjust program counter
    _df = Pins.skip(InstCdp1802::LSDF);     // LSDF: skip if DF=1
    _q = Pins.skip(InstCdp1802::LSQ);       // LSQ: skip if Q=1
    _ie = Pins.skip(InstCdp1802::LSIE);     // LSIE: skip if IE=1
    if (_cpuType == nullptr)
        setCpuType();
}

void RegsCdp1802::restore() {
    uint8_t LDT[] = {
            0xD0,  // SEP Rn
            0xE0,  // SEX Rn
            0x79,  // MARK
    };
    uint8_t LDQ_DF[] = {
            0x7A,     // REQ:0x7A, SEQ:0x7B
            0xF8, 0,  // LDI df
            0x76,     // SHRC
    };
    static const uint8_t SEP15_SEX14[] = {
            0xDF,  // SEP R15
            0xEE   // SEX R14
    };
    uint8_t LDR[] = {
            // CDP1802
            0xF8, 0,  // LDI hi(Rn)
            0xB0,     // PHI Rn
            0xF8, 0,  // LDI lo(Rn)
            0xA0,     // PLO Rn
    };
    uint8_t LDD[] = {
            0xF8, 0  // LDI d
    };
    uint8_t RET[] = {
            0x70,  // RET: 0x70 or DIS:0x71
            0,     // x,p
    };

    uint8_t tmp = _t & 0xF;
    _dirty[tmp] = true;
    LDT[0] = InstCdp1802::SEP | tmp;
    LDT[1] = InstCdp1802::SEX | (_t >> 4);
    Pins.captureWrites(LDT, sizeof(LDT), nullptr, &tmp, sizeof(tmp));
    LDQ_DF[0] = _q ? InstCdp1802::SEQ : InstCdp1802::REQ;
    LDQ_DF[2] = _df ? 1 : 0;
    Pins.execInst(LDQ_DF, sizeof(LDQ_DF));
    Pins.execInst(SEP15_SEX14, sizeof(SEP15_SEX14));
    _dirty[14] = _dirty[15] = true;
    for (auto i = 0; i < 16; i++) {
        if (_dirty[i]) {
            auto rn = _r[i];
            if (i == 14)
                rn -= 1;  // offset R14
            if (i == 15)
                rn -= sizeof(LDD) + 1;  // offset R15
            LDR[1] = hi(rn);
            LDR[2] = 0xB0 | i;
            LDR[4] = lo(rn);
            LDR[5] = 0xA0 | i;
            Pins.execInst(LDR, sizeof(LDR));
        }
    }
    LDD[1] = _d;
    Pins.execInst(LDD, sizeof(LDD));  // R15+=2
    RET[0] = _ie ? InstCdp1802::RET : InstCdp1802::DIS;
    RET[1] = (_x << 4) | _p;
    Pins.execInst(RET, sizeof(RET));  // R15++, R14++
}

void RegsCdp1802::helpRegisters() const {
    cli.println(F("?Reg: D DF X P T IE Q R0~R15 RP RX"));
}

constexpr const char *REGS1[] = {
        "DF",  // 1
        "IE",  // 2
        "Q",   // 3
};
constexpr const char *REGS4[] = {
        "P",  // 4
        "X",  // 5
};
constexpr const char *REGS8[] = {
        "D",  // 6
        "T",  // 7
};
constexpr const char *REGS16[] = {
        "R0",   // 8
        "R1",   // 9
        "R2",   // 10
        "R3",   // 11
        "R4",   // 12
        "R5",   // 13
        "R6",   // 14
        "R7",   // 15
        "R8",   // 16
        "R9",   // 17
        "R10",  // 18
        "R11",  // 19
        "R12",  // 20
        "R13",  // 21
        "R14",  // 22
        "R15",  // 23
        "RP",   // 24
        "RX",   // 25
};

const Regs::RegList *RegsCdp1802::listRegisters(uint8_t n) const {
    static constexpr RegList REG_LIST[] = {
            {REGS1, 3, 1, 1},
            {REGS4, 2, 4, 0xF},
            {REGS8, 2, 6, UINT8_MAX},
            {REGS16, 18, 8, UINT16_MAX},
    };
    return n < 4 ? &REG_LIST[n] : nullptr;
}

void RegsCdp1802::setRegister(uint8_t reg, uint32_t value) {
    switch (reg) {
    case 1:
        _df = value;
        break;
    case 6:
        _d = value;
        break;
    case 4:
        _p = value;
        break;
    case 5:
        _x = value;
        break;
    case 7:
        _t = value;
        break;
    case 2:
        _ie = value;
        break;
    case 3:
        _q = value;
        break;
    case 24:
        _r[_p] = value;
        _dirty[_p] = true;
        break;
    case 25:
        _r[_x] = value;
        _dirty[_x] = true;
        break;
    default:
        _r[reg - 8U] = value;
        _dirty[reg - 8U] = true;
        break;
    }
}

}  // namespace cdp1802
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
