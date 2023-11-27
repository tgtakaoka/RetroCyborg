#include "regs.h"

#include <libcli.h>

#include <asm_scn2650.h>
#include <dis_scn2650.h>
#include "config.h"
#include "digital_fast.h"
#include "pins.h"
#include "string_util.h"

#include <ctype.h>
#include <string.h>

extern libcli::Cli cli;

static constexpr bool debug_cycles = false;

libasm::scn2650::AsmScn2650 assembler;
libasm::scn2650::DisScn2650 disassembler;

struct Regs Regs;
struct Memory Memory;

static constexpr uint8_t insn_len(uint8_t e) {
    return (e >> 2) & 3;
}

static constexpr uint8_t bus_cycles(uint8_t e) {
    return e & 3;
}

static constexpr bool has_indirect(uint8_t e) {
    return e & 0x80;
}

static constexpr uint8_t E(uint8_t len, uint8_t bus) {
    return (len << 2) | bus;
}

static constexpr uint8_t I(uint8_t len, uint8_t bus) {
    return (len << 2) | bus | 0x80;
}

static constexpr uint8_t cycles[] = {
    E(0, 0),  // 00:-    -
    E(1, 0),  // 01:LODZ R1
    E(1, 0),  // 02:LODZ R2
    E(1, 0),  // 03:LODZ R3
    E(2, 0),  // 04:LODI R0,nn
    E(2, 0),  // 05:LODI R1,nn
    E(2, 0),  // 06:LODI R2,nn
    E(2, 0),  // 07:LODI R3,nn
    I(2, 1),  // 08:LODR R0,rel
    I(2, 1),  // 09:LODR R1,rel
    I(2, 1),  // 0A:LODR R2,rel
    I(2, 1),  // 0B:LODR R3,rel
    I(3, 1),  // 0C:LODA R0,abs
    I(3, 1),  // 0D:LODA R1,abs
    I(3, 1),  // 0E:LODA R2,abs
    I(3, 1),  // 0F:LODA R3,abs
    E(0, 0),  // 10:-    -
    E(0, 0),  // 11:-    -
    E(1, 0),  // 12:SPSU -
    E(1, 0),  // 13:SPSL -
    E(1, 0),  // 14:RETC EQ
    E(1, 0),  // 15:RETC GT
    E(1, 0),  // 16:RETC LT
    E(1, 0),  // 17:RETC UN
    I(2, 0),  // 18:BCTR EQ,rel
    I(2, 0),  // 19:BCTR GT,rel
    I(2, 0),  // 1A:BCTR LT,rel
    I(2, 0),  // 1B:BCTR UN,rel
    I(3, 0),  // 1C:BCTA EQ,abs
    I(3, 0),  // 1D:BCTA GT,abs
    I(3, 0),  // 1E:BCTA LT,abs
    I(3, 0),  // 1F:BCTA UN,abs
    E(1, 0),  // 20:EORZ R0
    E(1, 0),  // 21:EORZ R1
    E(1, 0),  // 22:EORZ R2
    E(1, 0),  // 23:EORZ R3
    E(2, 0),  // 24:EORI R0,nn
    E(2, 0),  // 25:EORI R1,nn
    E(2, 0),  // 26:EORI R2,nn
    E(2, 0),  // 27:EORI R3,nn
    I(2, 1),  // 28:EORR R0,rel
    I(2, 1),  // 29:EORR R1,rel
    I(2, 1),  // 2A:EORR R2,rel
    I(2, 1),  // 2B:EORR R3,rel
    I(3, 1),  // 2C:EORA R0,abs
    I(3, 1),  // 2D:EORA R1,abs
    I(3, 1),  // 2E:EORA R2,abs
    I(3, 1),  // 2F:EORA R3,abs
    E(1, 1),  // 30:REDC R0
    E(1, 1),  // 31:REDC R1
    E(1, 1),  // 32:REDC R2
    E(1, 1),  // 33:REDC R3
    E(1, 0),  // 34:RETE EQ
    E(1, 0),  // 35:RETE GT
    E(1, 0),  // 36:RETE LT
    E(1, 0),  // 37:RETE UN
    I(2, 0),  // 38:BSTR EQ,rel
    I(2, 0),  // 39:BSTR GT,rel
    I(2, 0),  // 3A:BSTR LT,rel
    I(2, 0),  // 3B:BSTR UN,rel
    I(3, 0),  // 3C:BSTA EQ,abs
    I(3, 0),  // 3D:BSTA GT,abs
    I(3, 0),  // 3E:BSTA LT,abs
    I(3, 0),  // 3F:BSTA UN,abs
    E(1, 0),  // 40:HALT -
    E(1, 0),  // 41:ANDZ R1
    E(1, 0),  // 42:ANDZ R2
    E(1, 0),  // 43:ANDZ R3
    E(2, 0),  // 44:ANDI R0,nn
    E(2, 0),  // 45:ANDI R1,nn
    E(2, 0),  // 46:ANDI R2,nn
    E(2, 0),  // 47:ANDI R3,nn
    I(2, 1),  // 48:ANDR R0,rel
    I(2, 1),  // 49:ANDR R1,rel
    I(2, 1),  // 4A:ANDR R2,rel
    I(2, 1),  // 4B:ANDR R3,rel
    I(3, 1),  // 4C:ANDA R0,abs
    I(3, 1),  // 4D:ANDA R1,abs
    I(3, 1),  // 4E:ANDA R2,abs
    I(3, 1),  // 4F:ANDA R3,abs
    E(1, 0),  // 50:RRR  R0
    E(1, 0),  // 51:RRR  R1
    E(1, 0),  // 52:RRR  R2
    E(1, 0),  // 53:RRR  R3
    E(2, 1),  // 54:REDE R0,io
    E(2, 1),  // 55:REDE R1,io
    E(2, 1),  // 56:REDE R2,io
    E(2, 1),  // 57:REDE R3,io
    I(2, 0),  // 58:BRNR R0,rel
    I(2, 0),  // 59:BRNR R1,rel
    I(2, 0),  // 5A:BRNR R2,rel
    I(2, 0),  // 5B:BRNR R3,rel
    I(3, 0),  // 5C:BRNA R0,abs
    I(3, 0),  // 5D:BRNA R1,abs
    I(3, 0),  // 5E:BRNA R2,abs
    I(3, 0),  // 5F:BRNA R3,abs
    E(1, 0),  // 60:IORZ R0
    E(1, 0),  // 61:IORZ R1
    E(1, 0),  // 62:IORZ R2
    E(1, 0),  // 63:IORZ R3
    E(2, 0),  // 64:IORI R0,nn
    E(2, 0),  // 65:IORI R1,nn
    E(2, 0),  // 66:IORI R2,nn
    E(2, 0),  // 67:IORI R3,nn
    I(2, 1),  // 68:IORR R0,rel
    I(2, 1),  // 69:IORR R1,rel
    I(2, 1),  // 6A:IORR R2,rel
    I(2, 1),  // 6B:IORR R3,rel
    I(3, 1),  // 6C:IORA R0,abs
    I(3, 1),  // 6D:IORA R1,abs
    I(3, 1),  // 6E:IORA R2,abs
    I(3, 1),  // 6F:IORA R3,abs
    E(1, 1),  // 70:REDD R0
    E(1, 1),  // 71:REDD R1
    E(1, 1),  // 72:REDD R2
    E(1, 1),  // 73:REDD R3
    E(2, 0),  // 74:CPSU nn
    E(2, 0),  // 75:CPSL nn
    E(2, 0),  // 76:PPSU nn
    E(2, 0),  // 77:PPSL nn
    I(2, 0),  // 78:BSNR R0,rel
    I(2, 0),  // 79:BSNR R1,rel
    I(2, 0),  // 7A:BSNR R2,rel
    I(2, 0),  // 7B:BSNR R3,rel
    I(3, 0),  // 7C:BSNA R0,abs
    I(3, 0),  // 7D:BSNA R1,abs
    I(3, 0),  // 7E:BSNA R2,abs
    I(3, 0),  // 7F:BSNA R3,abs
    E(1, 0),  // 80:ADDZ R0
    E(1, 0),  // 81:ADDZ R1
    E(1, 0),  // 82:ADDZ R2
    E(1, 0),  // 83:ADDZ R3
    E(2, 0),  // 84:ADDI R0,nn
    E(2, 0),  // 85:ADDI R1,nn
    E(2, 0),  // 86:ADDI R2,nn
    E(2, 0),  // 87:ADDI R3,nn
    I(2, 1),  // 88:ADDR R0,rel
    I(2, 1),  // 89:ADDR R1,rel
    I(2, 1),  // 8A:ADDR R2,rel
    I(2, 1),  // 8B:ADDR R3,rel
    I(3, 1),  // 8C:ADDA R0,abs
    I(3, 1),  // 8D:ADDA R1,abs
    I(3, 1),  // 8E:ADDA R2,abs
    I(3, 1),  // 8F:ADDA R3,abs
    E(0, 0),  // 90:-    -
    E(0, 0),  // 91:-    -
    E(1, 0),  // 92:LPSU -
    E(1, 0),  // 93:LPSL -
    E(1, 0),  // 94:DAR  R0
    E(1, 0),  // 95:DAR  R1
    E(1, 0),  // 96:DAR  R2
    E(1, 0),  // 97:DAR  R3
    I(2, 0),  // 98:BCFR EQ,rel
    I(2, 0),  // 99:BCFR GT,rel
    I(2, 0),  // 9A:BCFR LT,rel
    I(2, 0),  // 9B:ZBRR rel
    I(3, 0),  // 9C:BCFA EQ,abs
    I(3, 0),  // 9D:BCFA GT,abs
    I(3, 0),  // 9E:BCFA LT,abs
    I(3, 0),  // 9F:BXA  abs
    E(1, 0),  // A0:SUBZ R0
    E(1, 0),  // A1:SUBZ R1
    E(1, 0),  // A2:SUBZ R2
    E(1, 0),  // A3:SUBZ R3
    E(2, 0),  // A4:SUBI R0,nn
    E(2, 0),  // A5:SUBI R1,nn
    E(2, 0),  // A6:SUBI R2,nn
    E(2, 0),  // A7:SUBI R3,nn
    I(2, 1),  // A8:SUBR R0,rel
    I(2, 1),  // A9:SUBR R1,rel
    I(2, 1),  // AA:SUBR R2,rel
    I(2, 1),  // AB:SUBR R3,rel
    I(3, 1),  // AC:SUBA R0,abs
    I(3, 1),  // AD:SUBA R1,abs
    I(3, 1),  // AE:SUBA R2,abs
    I(3, 1),  // AF:SUBA R3,abs
    E(1, 1),  // B0:WRTC R0
    E(1, 1),  // B1:WRTC R1
    E(1, 1),  // B2:WRTC R2
    E(1, 1),  // B3:WRTC R3
    E(2, 0),  // B4:TPSU nn
    E(2, 0),  // B5:TPSL nn
    E(0, 0),  // B6:-    -
    E(0, 0),  // B7:-    -
    I(2, 0),  // B8:BSFR EQ,rel
    I(2, 0),  // B9:BSFR GT,rel
    I(2, 0),  // BA:BSFR LT,rel
    I(2, 0),  // BB:ZBSR rel
    I(3, 0),  // BC:BSFA EQ,abs
    I(3, 0),  // BD:BSFA GT,abs
    I(3, 0),  // BE:BSFA LT,abs
    I(3, 0),  // BF:BSXA abs
    E(1, 0),  // C0:NOP  -
    E(1, 0),  // C1:STRZ R1
    E(1, 0),  // C2:STRZ R2
    E(1, 0),  // C3:STRZ R3
    E(0, 0),  // C4:-    -
    E(0, 0),  // C5:-    -
    E(0, 0),  // C6:-    -
    E(0, 0),  // C7:-    -
    I(2, 1),  // C8:STRR R0,rel
    I(2, 1),  // C9:STRR R1,rel
    I(2, 1),  // CA:STRR R2,rel
    I(2, 1),  // CB:STRR R3,rel
    I(3, 1),  // CC:STRA R0,abs
    I(3, 1),  // CD:STRA R1,abs
    I(3, 1),  // CE:STRA R2,abs
    I(3, 1),  // CF:STRA R3,abs
    E(1, 0),  // D0:RRL  R0
    E(1, 0),  // D1:RRL  R1
    E(1, 0),  // D2:RRL  R2
    E(1, 0),  // D3:RRL  R3
    E(2, 1),  // D4:WRTE R0,io
    E(2, 1),  // D5:WRTE R1,io
    E(2, 1),  // D6:WRTE R2,io
    E(2, 1),  // D7:WRTE R3,io
    I(2, 0),  // D8:BIRR R0,rel
    I(2, 0),  // D9:BIRR R1,rel
    I(2, 0),  // DA:BIRR R2,rel
    I(2, 0),  // DB:BIRR R3,rel
    I(3, 0),  // DC:BIRA R0,abs
    I(3, 0),  // DD:BIRA R1,abs
    I(3, 0),  // DE:BIRA R2,abs
    I(3, 0),  // DF:BIRA R3,abs
    E(1, 0),  // E0:COMZ R0
    E(1, 0),  // E1:COMZ R1
    E(1, 0),  // E2:COMZ R2
    E(1, 0),  // E3:COMZ R3
    E(2, 0),  // E4:COMI R0,nn
    E(2, 0),  // E5:COMI R1,nn
    E(2, 0),  // E6:COMI R2,nn
    E(2, 0),  // E7:COMI R3,nn
    I(2, 1),  // E8:COMR R0,rel
    I(2, 1),  // E9:COMR R1,rel
    I(2, 1),  // EA:COMR R2,rel
    I(2, 1),  // EB:COMR R3,rel
    I(3, 1),  // EC:COMA R0,abs
    I(3, 1),  // ED:COMA R1,abs
    I(3, 1),  // EE:COMA R2,abs
    I(3, 1),  // EF:COMA R3,abs
    E(1, 1),  // F0:WRTD R0
    E(1, 1),  // F1:WRTD R1
    E(1, 1),  // F2:WRTD R2
    E(1, 1),  // F3:WRTD R3
    E(2, 0),  // F4:TMI  R0,nn
    E(2, 0),  // F5:TMI  R1,nn
    E(2, 0),  // F6:TMI  R2,nn
    E(2, 0),  // F7:TMI  R3,nn
    I(2, 0),  // F8:BDRR R0,rel
    I(2, 0),  // F9:BDRR R1,rel
    I(2, 0),  // FA:BDRR R2,rel
    I(2, 0),  // FB:BDRR R3,rel
    I(3, 0),  // FC:BDRA R0,abs
    I(3, 0),  // FD:BDRA R1,abs
    I(3, 0),  // FE:BDRA R2,abs
    I(3, 0),  // FF:BDRA R3,abs
};

uint8_t Regs::insnLen(uint8_t insn) {
    return insn_len(cycles[insn]);
}

uint8_t Regs::busCycles(uint8_t insn) {
    return bus_cycles(cycles[insn]);
}

bool Regs::hasIndirect(uint8_t insn) {
    return has_indirect(cycles[insn]);
}

const char *Regs::cpu() const {
    return "2650";
}

const char *Regs::cpuName() const {
    return "SCN2650";
}

static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void Regs::print() const {
    // clang-format off
    static char buffer[] = {
        'P', 'C', '=', 0, 0, 0, 0, ' ',      // PC=3
        'P', 'S', 'U', '=', 0, 0, 0, ' ',    // PSU=12
        'S', 'P', '=', 0, ' ',               // SP=19
        'C', 'C', '=', 0, ' ',               // CC=24
        'P', 'S', 'L', '=', 0, 0, 0, 0, 0, 0, // PSL=30
        0,
    };
    // clang-format on
    outHex16(buffer + 3, _pc);
    char *p = buffer + 12;
    p[0] = bit1(_psu & 0x80, 'S');  // SENSE
    p[1] = bit1(_psu & 0x40, 'F');  // FLAG
    p[2] = bit1(_psu & 0x20, 'I');  // Inhibit Interrupt
    buffer[19] = (_psu & 7) + '0';  // Stack pointer
    static constexpr char cc[] = {'Z', 'P', 'N', '3'};
    buffer[24] = cc[_psl >> 6];  // Condition code
    p = buffer + 30;
    p[0] = bit1(_psl & 0x20, 'D');  // Interdigit carry
    p[1] = rs() + '0';              // Register select
    p[2] = bit1(_psl & 8, 'W');     // With carry
    p[3] = bit1(_psl & 4, 'V');     // Overflow
    p[4] = (_psl & 2) ? 'L' : 'A';  // Logic ot Arithmetic
    p[5] = bit1(_psl & 1, 'C');     // Carry
    cli.println(buffer);
    // clang-format off
    static char reg[] = {
        'R', '0', '=', 0, 0, ' ', // R0=3
        'R', '1', '=', 0, 0, ' ', // Rn=(n-1)*6+9
        'R', '2', '=', 0, 0, ' ',
        'R', '3', '=', 0, 0, ' ',
        '(',
        'R', '1', '=', 0, 0, ' ', // Rn=(n-1)*6+28
        'R', '2', '=', 0, 0, ' ',
        'R', '3', '=', 0, 0, ')',
        0,
    };
    // clang-format on
    outHex8(reg + 3, _r0);
    const auto bank = rs();
    for (auto n = 1; n < 4; ++n) {
        outHex8(reg + (n - 1) * 6 + 9, _r[bank][n]);
        outHex8(reg + (n - 1) * 6 + 28, _r[1 - bank][n]);
    }
    cli.println(reg);
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

void Regs::save(bool show) {
    if (debug_cycles)
        cli.println(F("@@ save"));
    if (show)
        Signals::resetCycles();
    static constexpr uint8_t SAVE_R0PS[] = {
            0xC8, 0x00,        // STRR,R0 $
            0x13, 0xC8, 0x00,  // SPSL, STRR,R0 $
            0x12, 0xC8, 0x00,  // SPSU; STRR,R0 $
    };
    uint8_t buffer[3];
    Pins.captureWrites(
            SAVE_R0PS, sizeof(SAVE_R0PS), &_pc, buffer, sizeof(buffer));
    _r0 = buffer[0];
    _psl = buffer[1];
    _psu = buffer[2];
    static constexpr uint8_t SAVE_REGS[] = {
            0xC9, 0x00,  // STRR,R1 $
            0xCA, 0x00,  // STRR,R2,$
            0xCB, 0x00,  // STRR,R3 $
    };
    Pins.captureWrites(SAVE_REGS, sizeof(SAVE_REGS), nullptr, &_r[rs()][1], 3);
    constexpr uint8_t PPSL = 0x77;
    constexpr uint8_t CPSL = 0x75;
    static uint8_t SET_RS[] = {
            0, 0x10,  // PPSL|CPSL 0x10
    };
    SET_RS[0] = rs() ? CPSL : PPSL;
    Pins.execInst(SET_RS, sizeof(SET_RS));
    Pins.captureWrites(
            SAVE_REGS, sizeof(SAVE_REGS), nullptr, &_r[1 - rs()][1], 3);
    SET_RS[0] = rs() ? PPSL : CPSL;
    Pins.execInst(SET_RS, sizeof(SET_RS));

    if (show)
        Signals::printCycles();
}

void Regs::restore(bool show) {
    if (debug_cycles)
        cli.println(F("@@ restore"));
    if (show)
        Signals::resetCycles();

    constexpr uint8_t PPSL = 0x77;
    constexpr uint8_t CPSL = 0x75;
    static uint8_t RESTORE[] = {
            0, 0x10,        // PPSL|CPSL 0x10; switch to !rs bank
            0x05, 0,        // LODI,R1 _r1
            0x06, 0,        // LODI,R2 _r2
            0x07, 0,        // LODI,R3 _r3
            0, 0x10,        // PPSL|CPSL 0x10; switch to rs bank
            0x05, 0,        // LODI,R1 _r1
            0x06, 0,        // LODI,R2 _r2
            0x07, 0,        // LODI,R3 _r3
            0x04, 0, 0x92,  // LODI,R0 _psu; LPSU
            0x04, 0,        // LODI,R0 _r0
            PPSL, 0,        // PPSL _psl; restore PSL one bits
            CPSL, 0,        // CPSL ~_psl; restore PSL zero bits
            0x1F, 0, 0,     // BCTA _pc
    };
    RESTORE[0] = rs() ? CPSL : PPSL;
    RESTORE[8] = rs() ? PPSL : CPSL;
    for (auto n = 1; n < 4; ++n) {
        RESTORE[(n - 1) * 2 + 3] = _r[1 - rs()][n];
        RESTORE[(n - 1) * 2 + 11] = _r[rs()][n];
    }
    RESTORE[17] = _psu;
    RESTORE[20] = _r0;
    RESTORE[22] = _psl;
    RESTORE[24] = ~_psl;
    RESTORE[26] = hi(_pc);
    RESTORE[27] = lo(_pc);
    Pins.execInst(RESTORE, sizeof(RESTORE));

    if (show)
        Signals::printCycles();
}

void Regs::printRegList() const {
    cli.println(F("?Reg: PC PSU/PSL RS CC R0~R3"));
}

char Regs::validUint8Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("RS")) == 0)
        return 'S';
    if (strcasecmp_P(word, PSTR("CC")) == 0)
        return 'C';
    if (strcasecmp_P(word, PSTR("PSU")) == 0)
        return 'U';
    if (strcasecmp_P(word, PSTR("PSL")) == 0)
        return 'L';
    if (toupper(*word++) == 'R' && *word >= '0' && *word < '4')
        return *word;
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("PC")) == 0)
        return 'P';
    return 0;
}

void Regs::setRegValue(char reg, uint32_t value) {
    switch (reg) {
    case 'P':
        _pc = value;
        break;
    case 'U':
        _psu = value;
        break;
    case 'C':
        value <<= 6;
        value |= _psl & 0x3F;
        // Fall-through
    case 'L':
        _psl = value;
        break;
    case 'S':
        if (value)
            _psl |= 0x10;
        else
            _psl &= ~0x10;
        break;
    case '0':
        _r0 = value;
        break;
    case '1':
        _r[rs()][1] = value;
        return;
    case '2':
        _r[rs()][2] = value;
        return;
    case '3':
        _r[rs()][3] = value;
        return;
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

uint16_t Regs::disassemble(uint16_t addr, uint16_t numInsn) const {
    disassembler.setCpu(cpu());
    disassembler.setOption("uppercase", "true");
    const auto nameMax = -(disassembler.nameMax() + 1);
    uint16_t num = 0;
    while (num < numInsn) {
        char operands[20];
        libasm::Insn insn(addr);
        Memory.setAddress(addr);
        disassembler.decode(Memory, insn, operands, sizeof(operands));
        addr += insn.length();
        num++;
        printInsn(insn);
        cli.printStr(insn.name(), nameMax);
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

uint16_t Regs::assemble(uint16_t addr, const char *line) const {
    assembler.setCpu(cpu());
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
        Memory.write(insn.address(), insn.bytes(), insn.length());
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
    return memory[addr];
}

void Memory::write(uint16_t addr, uint8_t data, const char *space) {
    memory[addr] = data;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
