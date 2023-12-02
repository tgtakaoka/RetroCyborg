#include <ctype.h>
#include <string.h>

#include "regs.h"

#include <asm_f3850.h>
#include <dis_f3850.h>
#include <libcli.h>

#include "config.h"
#include "digital_fast.h"
#include "i8251.h"
#include "pins.h"
#include "string_util.h"

#define LOG_ROMC(e)
//#define LOG_ROMC(e) e

extern libcli::Cli cli;
extern I8251 Usart;

static constexpr bool debug_cycles = false;

struct Regs Regs;
struct Memory Memory;

const char *Regs::cpu() const {
    return "F3850";
}

const char *Regs::cpuName() const {
    return "F3850";
}

namespace {

libasm::f3850::AsmF3850 assembler;
libasm::f3850::DisF3850 disassembler;

constexpr uint8_t insn_len(uint8_t e) {
    return e & 3;
}

constexpr uint8_t bus_cycles(uint8_t e) {
    return (e >> 2) & 7;
}

constexpr uint8_t E(uint8_t len, uint8_t cycles) {
    // len: 0~3, cycles: 0~2
    return (cycles << 2) | len;
}

constexpr uint8_t insn_table[] = {
        E(1, 0),  // 00: LR   A,KU
        E(1, 0),  // 01: LR   A,KL
        E(1, 0),  // 02: LR   A,QU
        E(1, 0),  // 03: LR   A,QL
        E(1, 0),  // 04: LR   KU,A
        E(1, 0),  // 05: LR   KL,A
        E(1, 0),  // 06: LR   QU,A
        E(1, 0),  // 07: LR   QL,A
        E(1, 2),  // 08: LR   K,P
        E(1, 2),  // 09: LR   P,K
        E(1, 0),  // 0A: LR   A,IS
        E(1, 0),  // 0B: LR   IS,A
        E(1, 2),  // 0C: PK   -
        E(1, 2),  // 0D: LR   P0,Q
        E(1, 2),  // 0E: LR   Q,DC
        E(1, 2),  // 0F: LR   DC,Q
        E(1, 2),  // 10: LR   DC,H
        E(1, 2),  // 11: LR   H,DC
        E(1, 0),  // 12: SR   1
        E(1, 0),  // 13: SL   1
        E(1, 0),  // 14: SR   4
        E(1, 0),  // 15: SL   4
        E(1, 1),  // 16: LM   -
        E(1, 1),  // 17: ST   -
        E(1, 0),  // 18: COM  -
        E(1, 0),  // 19: LNK  -
        E(1, 1),  // 1A: DI   -
        E(1, 1),  // 1B: EI   -
        E(1, 1),  // 1C: POP  -
        E(1, 1),  // 1D: LR   W,J
        E(1, 0),  // 1E: LR   J,W
        E(1, 0),  // 1F: INC  -
        E(2, 0),  // 20: LI   aa
        E(2, 0),  // 21: NI   aa
        E(2, 0),  // 22: OI   aa
        E(2, 0),  // 23: XI   aa
        E(2, 0),  // 24: AI   aa
        E(2, 0),  // 25: CI   aa
        E(2, 1),  // 26: IN   pp
        E(2, 1),  // 27: OUT  pp
        E(3, 2),  // 28: PI   iijj
        E(3, 1),  // 29: JMP  iijj
        E(3, 2),  // 2A: DCI  iijj
        E(1, 0),  // 2B: NOP  -
        E(1, 1),  // 2C: XDC  -
        E(0, 0),  // 2D: -    -
        E(0, 0),  // 2E: -    -
        E(0, 0),  // 2F: -    -
        E(1, 0),  // 30: DS   0
        E(1, 0),  // 31: DS   1
        E(1, 0),  // 32: DS   2
        E(1, 0),  // 33: DS   3
        E(1, 0),  // 34: DS   4
        E(1, 0),  // 35: DS   5
        E(1, 0),  // 36: DS   6
        E(1, 0),  // 37: DS   7
        E(1, 0),  // 38: DS   8
        E(1, 0),  // 39: DS   J
        E(1, 0),  // 3A: DS   HU
        E(1, 0),  // 3B: DS   HL
        E(1, 0),  // 3C: DS   S
        E(1, 0),  // 3D: DS   I
        E(1, 0),  // 3E: DS   D
        E(0, 0),  // 3F: -    -
        E(1, 0),  // 40: LR   A,0
        E(1, 0),  // 41: LR   A,1
        E(1, 0),  // 42: LR   A,2
        E(1, 0),  // 43: LR   A,3
        E(1, 0),  // 44: LR   A,4
        E(1, 0),  // 45: LR   A,5
        E(1, 0),  // 46: LR   A,6
        E(1, 0),  // 47: LR   A,7
        E(1, 0),  // 48: LR   A,8
        E(1, 0),  // 49: LR   A,J
        E(1, 0),  // 4A: LR   A,HU
        E(1, 0),  // 4B: LR   A,HL
        E(1, 0),  // 4C: LR   A,S
        E(1, 0),  // 4D: LR   A,I
        E(1, 0),  // 4E: LR   A,D
        E(0, 0),  // 4F: -    -
        E(1, 0),  // 50: LR   0,A
        E(1, 0),  // 51: LR   1,A
        E(1, 0),  // 52: LR   2,A
        E(1, 0),  // 53: LR   3,A
        E(1, 0),  // 54: LR   4,A
        E(1, 0),  // 55: LR   5,A
        E(1, 0),  // 56: LR   6,A
        E(1, 0),  // 57: LR   7,A
        E(1, 0),  // 58: LR   8,A
        E(1, 0),  // 59: LR   J,A
        E(1, 0),  // 5A: LR   HU,A
        E(1, 0),  // 5B: LR   HL,A
        E(1, 0),  // 5C: LR   S,A
        E(1, 0),  // 5D: LR   I,A
        E(1, 0),  // 5E: LR   D,A
        E(0, 0),  // 5F: -    -
        E(1, 0),  // 60: LISU 0
        E(1, 0),  // 61: LISU 1
        E(1, 0),  // 62: LISU 2
        E(1, 0),  // 63: LISU 3
        E(1, 0),  // 64: LISU 4
        E(1, 0),  // 65: LISU 5
        E(1, 0),  // 66: LISU 6
        E(1, 0),  // 67: LISU 7
        E(1, 0),  // 68: LISL 0
        E(1, 0),  // 69: LISL 1
        E(1, 0),  // 6A: LISL 2
        E(1, 0),  // 6B: LISL 3
        E(1, 0),  // 6C: LISL 4
        E(1, 0),  // 6D: LISL 5
        E(1, 0),  // 6E: LISL 6
        E(1, 0),  // 6F: LISL 7
        E(1, 0),  // 70: LIS  0
        E(1, 0),  // 71: LIS  1
        E(1, 0),  // 72: LIS  2
        E(1, 0),  // 73: LIS  3
        E(1, 0),  // 74: LIS  4
        E(1, 0),  // 75: LIS  5
        E(1, 0),  // 76: LIS  6
        E(1, 0),  // 77: LIS  7
        E(1, 0),  // 78: LIS  8
        E(1, 0),  // 79: LIS  9
        E(1, 0),  // 7A: LIS  10
        E(1, 0),  // 7B: LIS  11
        E(1, 0),  // 7C: LIS  12
        E(1, 0),  // 7D: LIS  13
        E(1, 0),  // 7E: LIS  14
        E(1, 0),  // 7F: LIS  15
        E(2, 1),  // 80: BT   0,ii
        E(2, 1),  // 81: BP   ii
        E(2, 1),  // 82: BC   ii
        E(2, 1),  // 83: BT   3,ii
        E(2, 1),  // 84: BZ   ii
        E(2, 1),  // 85: BT   5,ii
        E(2, 1),  // 86: BT   6,ii
        E(2, 1),  // 87: BT   7,ii
        E(1, 1),  // 88: AM   -
        E(1, 1),  // 89: AMD  -
        E(1, 1),  // 8A: NM   -
        E(1, 1),  // 8B: OM   -
        E(1, 1),  // 8C: XM   -
        E(1, 1),  // 8D: CM   -
        E(1, 1),  // 8E: ADC  -
        E(2, 0),  // 8F: BR7  ij
        E(2, 1),  // 90: BR   ij
        E(2, 1),  // 91: BM   ij
        E(2, 1),  // 92: BNC  ij
        E(2, 1),  // 93: BF   3,ij
        E(2, 1),  // 94: BNZ  ij
        E(2, 1),  // 95: BF   5,ij
        E(2, 1),  // 96: BF   6,ij
        E(2, 1),  // 97: BF   7,ij
        E(2, 1),  // 98: BNO  ij
        E(2, 1),  // 99: BF   9,ij
        E(2, 1),  // 9A: BF   10,ij
        E(2, 1),  // 9B: BF   11,ij
        E(2, 1),  // 9C: BF   12,ij
        E(2, 1),  // 9D: BF   13,ij
        E(2, 1),  // 9E: BF   14,ij
        E(2, 1),  // 9F: BF   15,ij
        E(1, 1),  // A0: INS  0
        E(1, 1),  // A1: INS  1
        E(1, 1),  // A2: -    -
        E(1, 1),  // A3: -    -
        E(1, 2),  // A4: INS  4
        E(1, 2),  // A5: INS  5
        E(1, 2),  // A6: INS  6
        E(1, 2),  // A7: INS  7
        E(1, 2),  // A8: INS  8
        E(1, 2),  // A9: INS  9
        E(1, 2),  // AA: INS  10
        E(1, 2),  // AB: INS  11
        E(1, 2),  // AC: INS  12
        E(1, 2),  // AD: INS  13
        E(1, 2),  // AE: INS  14
        E(1, 2),  // AF: INS  15
        E(1, 1),  // B0: OUTS 0
        E(1, 1),  // B1: OUTS 1
        E(1, 1),  // B2: -    -
        E(1, 1),  // B3: -    -
        E(1, 2),  // B4: OUTS 4
        E(1, 2),  // B5: OUTS 5
        E(1, 2),  // B6: OUTS 6
        E(1, 2),  // B7: OUTS 7
        E(1, 2),  // B8: OUTS 8
        E(1, 2),  // B9: OUTS 9
        E(1, 2),  // BA: OUTS 10
        E(1, 2),  // BB: OUTS 11
        E(1, 2),  // BC: OUTS 12
        E(1, 2),  // BD: OUTS 13
        E(1, 2),  // BE: OUTS 14
        E(1, 2),  // BF: OUTS 15
        E(1, 0),  // C0: AS   0
        E(1, 0),  // C1: AS   1
        E(1, 0),  // C2: AS   2
        E(1, 0),  // C3: AS   3
        E(1, 0),  // C4: AS   4
        E(1, 0),  // C5: AS   5
        E(1, 0),  // C6: AS   6
        E(1, 0),  // C7: AS   7
        E(1, 0),  // C8: AS   8
        E(1, 0),  // C9: AS   J
        E(1, 0),  // CA: AS   HU
        E(1, 0),  // CB: AS   HL
        E(1, 0),  // CC: AS   S
        E(1, 0),  // CD: AS   I
        E(1, 0),  // CE: AS   D
        E(0, 0),  // CF: -    -
        E(1, 1),  // D0: ASD  0
        E(1, 1),  // D1: ASD  1
        E(1, 1),  // D2: ASD  2
        E(1, 1),  // D3: ASD  3
        E(1, 1),  // D4: ASD  4
        E(1, 1),  // D5: ASD  5
        E(1, 1),  // D6: ASD  6
        E(1, 1),  // D7: ASD  7
        E(1, 1),  // D8: ASD  8
        E(1, 1),  // D9: ASD  J
        E(1, 1),  // DA: ASD  HU
        E(1, 1),  // DB: ASD  HL
        E(1, 1),  // DC: ASD  S
        E(1, 1),  // DD: ASD  I
        E(1, 1),  // DE: ASD  D
        E(0, 0),  // DF: -    -
        E(1, 0),  // E0: XS   0
        E(1, 0),  // E1: XS   1
        E(1, 0),  // E2: XS   2
        E(1, 0),  // E3: XS   3
        E(1, 0),  // E4: XS   4
        E(1, 0),  // E5: XS   5
        E(1, 0),  // E6: XS   6
        E(1, 0),  // E7: XS   7
        E(1, 0),  // E8: XS   8
        E(1, 0),  // E9: XS   J
        E(1, 0),  // EA: XS   HU
        E(1, 0),  // EB: XS   HL
        E(1, 0),  // EC: XS   S
        E(1, 0),  // ED: XS   I
        E(1, 0),  // EE: XS   D
        E(0, 0),  // EF: -    -
        E(1, 0),  // F0: NS   0
        E(1, 0),  // F1: NS   1
        E(1, 0),  // F2: NS   2
        E(1, 0),  // F3: NS   3
        E(1, 0),  // F4: NS   4
        E(1, 0),  // F5: NS   5
        E(1, 0),  // F6: NS   6
        E(1, 0),  // F7: NS   7
        E(1, 0),  // F8: NS   8
        E(1, 0),  // F9: NS   J
        E(1, 0),  // FA: NS   HU
        E(1, 0),  // FB: NS   HL
        E(1, 0),  // FC: NS   S
        E(1, 0),  // FD: NS   I
        E(1, 0),  // FE: NS   D
        E(0, 0),  // FF: -    -
};

char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

}  // namespace

uint8_t Regs::insnLen(uint8_t insn) {
    return insn_len(insn_table[insn]);
}

uint8_t Regs::busCycles(uint8_t insn) {
    return bus_cycles(insn_table[insn]);
}

bool Regs::romc_read(Signals &signals) {
    LOG_ROMC(cli.print(F("@@ R ROMC=")));
    LOG_ROMC(cli.printHex(signals.romc, 2));
    LOG_ROMC(cli.print(F(" pc0=")));
    LOG_ROMC(cli.printHex(_pc0, 4));
    LOG_ROMC(cli.print(F(" dc0=")));
    LOG_ROMC(cli.printlnHex(_dc0, 4));
    switch (signals.romc) {
    case 0x00:
        signals.markFetch();
        if (signals.readRam())
            signals.debug('f').data = Memory.read(_pc0);
        signals.markRead(_pc0++);
        break;
    case 0x01:
        if (signals.readRam())
            signals.debug('m').data = Memory.read(_pc0);
        signals.markRead(_pc0);
        _pc0 += static_cast<int8_t>(signals.data);
        break;
    case 0x02:
        if (signals.readRam())
            signals.debug('m').data = Memory.read(_dc0);
        signals.markRead(_dc0++);
        break;
    case 0x03:
        if (signals.readRam())
            signals.debug('o').data = Memory.read(_pc0);
        signals.markRead(_pc0++);
        _ioaddr = signals.data;
        break;
    case 0x06:
        signals.data = hi(_dc0);
        break;
    case 0x07:
        signals.data = hi(_pc1);
        break;
    case 0x09:
        signals.data = lo(_dc0);
        break;
    case 0x0B:
        signals.data = lo(_pc1);
        break;
    case 0x0C:
        if (signals.readRam())
            signals.debug('m').data = Memory.read(_pc0);
        signals.markRead(_pc0);
        _pc0 = uint16(hi(_pc0), signals.data);
        break;
    case 0x0E:
        if (signals.readRam())
            signals.debug('m').data = Memory.read(_pc0);
        signals.markRead(_pc0);
        _dc0 = uint16(hi(_dc0), signals.data);
        break;
    case 0x0F:  // Interruput acknowledge
        signals.debug('v').data = lo(Usart.intrVec());
        _pc1 = _pc0;
        _pc0 = uint16(hi(_pc0), signals.data);
        break;
    case 0x11:
        if (signals.readRam())
            signals.debug('m').data = Memory.read(_pc0);
        signals.markRead(_pc0);
        _dc0 = uint16(signals.data, lo(_dc0));
        break;
    case 0x13:  // Interruput acknowledge
        signals.debug('V').data = hi(Usart.intrVec());
        _pc0 = uint16(signals.data, lo(_pc0));
        break;
    case 0x1B:  // I/O read
        if (_ioaddr < 4)
            return false;
        if (Usart.isSelected(_ioaddr)) {
            signals.debug('u').data = Usart.read(_ioaddr);
        } else {
            signals.data = 0;
        }
        signals.markIoRead(_ioaddr);
        break;
    case 0x1E:
        signals.data = lo(_pc0);
        break;
    case 0x1F:
        signals.data = hi(_pc0);
        break;
    default:
        _delay = hi(_delay);
        signals.data = 0;
        return false;
    }
    return true;
}

bool Regs::romc_write(Signals &signals) {
    LOG_ROMC(cli.print(F("@@ W ROMC=")));
    LOG_ROMC(cli.printHex(signals.romc, 2));
    LOG_ROMC(cli.print(F(" pc0=")));
    LOG_ROMC(cli.printHex(_pc0, 4));
    LOG_ROMC(cli.print(F(" dc0=")));
    LOG_ROMC(cli.printlnHex(_dc0, 4));
    switch (signals.romc) {
    case 0x04:
        _pc0 = _pc1;
        return false;
    case 0x05:
        if (signals.writeRam())
            Memory.write(_dc0, signals.debug('m').data);
        signals.markWrite(_dc0++);
        break;
    case 0x08:
        _pc1 = _pc0;
        _pc0 = uint16(signals.data, signals.data);
        break;
    case 0x0A:
        _dc0 += static_cast<int8_t>(signals.data);
        break;
    case 0x0D:
        _pc1 = _pc0 + 1;
        ++_delay;
        return false;
    case 0x12:
        _pc1 = _pc0;
        _pc0 = uint16(hi(_pc0), signals.data);
        break;
    case 0x14:
        _pc0 = uint16(signals.data, lo(_pc0));
        break;
    case 0x15:
        _pc1 = uint16(signals.data, lo(_pc1));
        break;
    case 0x16:
        _dc0 = uint16(signals.data, lo(_dc0));
        break;
    case 0x17:
        _pc0 = uint16(hi(_pc0), signals.data);
        break;
    case 0x18:
        _pc1 = uint16(hi(_pc1), signals.data);
        break;
    case 0x19:
        _dc0 = uint16(hi(_dc0), signals.data);
        break;
    case 0x1A:  // I/O write
        if (_ioaddr < 4)
            return false;
        if (Usart.isSelected(_ioaddr))
            Usart.write(signals.debug('U').data, _ioaddr);
        signals.markIoWrite(_ioaddr);
        break;
    case 0x1C:
        _ioaddr = signals.data;
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

void Regs::print() const {
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

namespace {

constexpr uint16_t uint16(uint8_t hi, uint8_t lo) {
    return (static_cast<uint16_t>(hi) << 8) | lo;
}

constexpr uint16_t le16(uint8_t *p) {
    return uint16(p[1], p[0]);
}

constexpr uint16_t be16(uint8_t *p) {
    return uint16(p[0], p[1]);
}

constexpr uint8_t hi(uint16_t v) {
    return static_cast<uint8_t>(v >> 8);
}

constexpr uint8_t lo(uint16_t v) {
    return static_cast<uint8_t>(v);
}

}  // namespace

void Regs::save(bool show) {
    if (debug_cycles)
        cli.println(F("@@ save"));
    if (show)
        Signals::resetCycles();
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
    if (show)
        Signals::printCycles();
}

void Regs::restore(bool show) {
    if (debug_cycles)
        cli.println(F("@@ restore"));
    if (show)
        Signals::resetCycles();
    static uint8_t LOAD_ALL[] = {
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
    if (show)
        Signals::printCycles();
}

void Regs::update_r(uint8_t num, uint8_t val) {
    _r[num] = val;
    write_reg(num, val);
}

uint8_t Regs::read_reg(uint8_t addr) {
    static uint8_t READ_REG[] = {
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

void Regs::write_reg(uint8_t addr, uint8_t val) {
    static uint8_t WRITE_REG[] = {
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

uint8_t Regs::read_io(uint8_t addr) {
    if (addr < 2) {
        static uint8_t READ_IO[] = {
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
    if (Usart.isSelected(addr))
        return Usart.read(addr);
    return 0;
}

void Regs::write_io(uint8_t addr, uint8_t val) {
    if (addr < 2) {
        static uint8_t WRITE_IO[] = {
                0x20, 0, 0xB0,  // LI d; OUTS p
        };
        const auto pc = _pc0;  // save PC0
        WRITE_IO[1] = val;
        WRITE_IO[2] = 0xB0 | addr;
        Pins.execInst(WRITE_IO, sizeof(WRITE_IO));
        _pc0 = pc;
        return;
    }
    if (Usart.isSelected(addr))
        Usart.write(val, addr);
}

void Regs::printRegList() const {
    cli.println(F("?Reg: P0 P DC DC1 H K Q IS W A R0-R15 J {H,K,Q,IS}{U,L}"));
}

char Regs::validUint8Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("A")) == 0)
        return 'A';
    if (strcasecmp_P(word, PSTR("W")) == 0)
        return 'W';
    if (strcasecmp_P(word, PSTR("J")) == 0)
        return '9';
    if (strcasecmp_P(word, PSTR("HU")) == 0)
        return 'a';
    if (strcasecmp_P(word, PSTR("HL")) == 0)
        return 'b';
    if (strcasecmp_P(word, PSTR("KU")) == 0)
        return 'c';
    if (strcasecmp_P(word, PSTR("KL")) == 0)
        return 'd';
    if (strcasecmp_P(word, PSTR("QU")) == 0)
        return 'e';
    if (strcasecmp_P(word, PSTR("QL")) == 0)
        return 'f';
    if (strcasecmp_P(word, PSTR("IS")) == 0)
        return 'I';
    if (strcasecmp_P(word, PSTR("ISU")) == 0)
        return 'U';
    if (strcasecmp_P(word, PSTR("ISL")) == 0)
        return 'L';
    if (toupper(*word++) == 'R' && isdigit(*word)) {
        auto num = *word++ - '0';
        if (isdigit(*word)) {
            num *= 10;
            num += *word++ - '0';
        }
        if (*word == 0 && num < 16)
            return num < 10 ? num + '0' : num - 10 + 'a';
    }
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("P0")) == 0 ||
            strcasecmp_P(word, PSTR("PC")) == 0)
        return 'P';
    if (strcasecmp_P(word, PSTR("P")) == 0 ||
            strcasecmp_P(word, PSTR("SP")) == 0)
        return 'S';
    if (strcasecmp_P(word, PSTR("DC")) == 0)
        return 'D';
    if (strcasecmp_P(word, PSTR("DC1")) == 0)
        return 'E';
    if (strcasecmp_P(word, PSTR("H")) == 0)
        return 'H';
    if (strcasecmp_P(word, PSTR("K")) == 0)
        return 'K';
    if (strcasecmp_P(word, PSTR("Q")) == 0)
        return 'Q';
    return 0;
}

void Regs::setRegValue(char reg, uint32_t value) {
    switch (reg) {
    case 'P':
        _pc0 = value;
        break;
    case 'S':
        _pc1 = value;
        break;
    case 'D':
        _dc0 = value;
        break;
    case 'E':
        _dc1 = value;
        break;
    case 'H':
        update_r(HU, hi(value));
        update_r(HL, lo(value));
        break;
    case 'K':
        update_r(KU, hi(value));
        update_r(KL, lo(value));
        break;
    case 'Q':
        update_r(QU, hi(value));
        update_r(QL, lo(value));
        break;
    case 'A':
        _a = value;
        break;
    case 'W':
        _w = value;
        break;
    case 'I':
        _isar = value;
        break;
    case 'U':
        set_isu(value);
        break;
    case 'L':
        set_isl(value);
        break;
    default:
        if (isxdigit(reg)) {
            const auto num = isdigit(reg) ? reg - '0' : reg - 'a' + 10;
            update_r(num, value);
        }
    }
}

namespace {

void printInsn(const libasm::Insn &insn) {
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

}  // namespace

uint16_t Memory::disassemble(uint16_t addr, uint16_t numInsn) {
    disassembler.setCpu(Regs.cpu());
    disassembler.setOption("uppercase", "true");
    uint16_t num = 0;
    while (num < numInsn) {
        char operands[20];
        libasm::Insn insn(addr);
        setAddress(addr);
        disassembler.decode(*this, insn, operands, sizeof(operands));
        addr += insn.length();
        num++;
        printInsn(insn);
        cli.printStr(insn.name(), -6);
        cli.printlnStr(operands, -12);
        if (insn.getError()) {
            cli.print(F("Error: "));
            cli.printStr_P(insn.errorText_P());
            if (*insn.errorAt()) {
                cli.print(F(" at '"));
                cli.printStr(insn.errorAt());
                cli.print('\'');
            }
            cli.println();
        }
    }
    return addr;
}

uint16_t Memory::assemble(uint16_t addr, const char *line) {
    assembler.setCpu(Regs.cpu());
    libasm::Insn insn(addr);
    assembler.encode(line, insn);
    if (insn.getError()) {
        cli.print(F("Error: "));
        cli.print(insn.errorText_P());
        if (*insn.errorAt()) {
            cli.print(F(" at '"));
            cli.printStr(insn.errorAt());
            cli.print('\'');
        }
        cli.println();
    } else {
        write(insn.address(), insn.bytes(), insn.length());
        disassemble(insn.address(), 1);
        addr += insn.length();
    }
    return addr;
}

void Memory::write(uint16_t addr, const uint8_t *data, uint8_t len) {
    for (auto i = 0; i < len; i++) {
        write(addr++, *data++);
    }
}

uint8_t Memory::read(uint16_t addr, const char *space) const {
    if (space == nullptr)
        return memory[addr];
    if (toupper(*space) == 'I')
        return Regs.read_io(addr);
    if (toupper(*space) == 'R' && addr < 0x40)
        return Regs.read_reg(addr);
    return 0;
}

void Memory::write(uint16_t addr, uint8_t data, const char *space) {
    if (space == nullptr) {
        memory[addr] = data;
    } else if (toupper(*space) == 'I') {
        Regs.write_io(addr, data);
    } else if (toupper(*space) == 'R' && addr < 0x40) {
        Regs.write_reg(addr, data);
    }
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
