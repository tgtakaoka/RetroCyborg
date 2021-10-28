#include "regs.h"

#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;

struct Regs Regs;

static constexpr uint8_t cycles_table[256] = {
        0,   // 00:0 -    -
        2,   // 01:1 NOP  INH
        0,   // 02:0 -    -
        0,   // 03:0 -    -
        0,   // 04:0 -    -
        0,   // 05:0 -    -
        2,   // 06:1 TAP  INH
        2,   // 07:1 TPA  INH
        4,   // 08:1 INX  INH
        4,   // 09:1 DEX  INH
        2,   // 0A:1 CLV  INH
        2,   // 0B:1 SEV  INH
        2,   // 0C:1 CLC  INH
        2,   // 0D:1 SEC  INH
        2,   // 0E:1 CLI  INH
        2,   // 0F:1 SEI  INH
        2,   // 10:1 SBA  INH
        2,   // 11:1 CBA  INH
        0,   // 12:0 -    -
        0,   // 13:0 -    -
        0,   // 14:0 -    -
        0,   // 15:0 -    -
        2,   // 16:1 TAB  INH
        2,   // 17:1 TBA  INH
        0,   // 18:0 -    -
        2,   // 19:1 DAA  INH
        0,   // 1A:0 -    -
        2,   // 1B:1 ABA  INH
        0,   // 1C:0 -    -
        0,   // 1D:0 -    -
        0,   // 1E:0 -    -
        0,   // 1F:0 -    -
        4,   // 20:2 BRA  REL
        0,   // 21:0 -    -
        4,   // 22:2 BHI  REL
        4,   // 23:2 BLS  REL
        4,   // 24:2 BCC  REL
        4,   // 25:2 BCS  REL
        4,   // 26:2 BNE  REL
        4,   // 27:2 BEQ  REL
        4,   // 28:2 BVC  REL
        4,   // 29:2 BVS  REL
        4,   // 2A:2 BPL  REL
        4,   // 2B:2 BMI  REL
        4,   // 2C:2 BGE  REL
        4,   // 2D:2 BLT  REL
        4,   // 2E:2 BGT  REL
        4,   // 2F:2 BLE  REL
        4,   // 30:1 TSX  INH
        4,   // 31:1 INS  INH
        4,   // 32:1 PULA INH
        4,   // 33:1 PULB INH
        4,   // 34:1 DES  INH
        4,   // 35:1 TXS  INH
        4,   // 36:1 PSHA INH
        4,   // 37:1 PSHB INH
        0,   // 38:0 -    -
        5,   // 39:1 RTS  INH
        0,   // 3A:0 -    -
        10,  // 3B:1 RTI  INH
        0,   // 3C:0 -    -
        0,   // 3D:0 -    -
        9,   // 3E:1 WAI  INH
        12,  // 3F:1 SWI  INH
        2,   // 40:1 NEGA INH
        0,   // 41:0 -    -
        0,   // 42:0 -    -
        2,   // 43:1 COMA INH
        2,   // 44:1 LSRA INH
        0,   // 45:0 -    -
        2,   // 46:1 RORA INH
        2,   // 47:1 ASRA INH
        2,   // 48:1 ASLA INH
        2,   // 49:1 ROLA INH
        2,   // 4A:1 DECA INH
        0,   // 4B:0 -    -
        2,   // 4C:1 INCA INH
        2,   // 4D:1 TSTA INH
        0,   // 4E:0 -    -
        2,   // 4F:1 CLRA INH
        2,   // 50:1 NEGB INH
        0,   // 51:0 -    -
        0,   // 52:0 -    -
        2,   // 53:1 COMB INH
        2,   // 54:1 LSRB INH
        0,   // 55:0 -    -
        2,   // 56:1 RORB INH
        2,   // 57:1 ASRB INH
        2,   // 58:1 ASLB INH
        2,   // 59:1 ROLB INH
        2,   // 5A:1 DECB INH
        0,   // 5B:0 -    -
        2,   // 5C:1 INCB INH
        2,   // 5D:1 TSTB INH
        0,   // 5E:0 -    -
        2,   // 5F:1 CLRB INH
        7,   // 60:2 NEG  IDX
        0,   // 61:0 -    -
        0,   // 62:0 -    -
        7,   // 63:2 COM  IDX
        7,   // 64:2 LSR  IDX
        0,   // 65:0 -    -
        7,   // 66:2 ROR  IDX
        7,   // 67:2 ASR  IDX
        7,   // 68:2 ASL  IDX
        7,   // 69:2 ROL  IDX
        7,   // 6A:2 DEC  IDX
        0,   // 6B:0 -    -
        7,   // 6C:2 INC  IDX
        7,   // 6D:2 TST  IDX
        4,   // 6E:2 JMP  IDX
        7,   // 6F:2 CLR  IDX
        6,   // 70:3 NEG  EXT
        0,   // 71:0 -    -
        0,   // 72:0 -    -
        6,   // 73:3 COM  EXT
        6,   // 74:3 LSR  EXT
        0,   // 75:0 -    -
        6,   // 76:3 ROR  EXT
        6,   // 77:3 ASR  EXT
        6,   // 78:3 ASL  EXT
        6,   // 79:3 ROL  EXT
        6,   // 7A:3 DEC  EXT
        0,   // 7B:0 -    -
        6,   // 7C:3 INC  EXT
        6,   // 7D:3 TST  EXT
        3,   // 7E:3 JMP  EXT
        6,   // 7F:3 CLR  EXT
        2,   // 80:2 SUBA IMM
        2,   // 81:2 CMPA IMM
        2,   // 82:2 SBCA IMM
        0,   // 83:0 -    -
        2,   // 84:2 ANDA IMM
        2,   // 85:2 BITA IMM
        2,   // 86:2 LDAA IMM
        0,   // 87:0 -    -
        2,   // 88:2 EORA IMM
        2,   // 89:2 ADCA IMM
        2,   // 8A:2 ORAA IMM
        2,   // 8B:2 ADDA IMM
        3,   // 8C:3 CPX  IMM
        8,   // 8D:2 BSR  REL
        3,   // 8E:3 LDS  IMM
        0,   // 8F:0 -    -
        3,   // 90:2 SUBA DIR
        3,   // 91:2 CMPA DIR
        3,   // 92:2 SBCA DIR
        0,   // 93:0 -    -
        3,   // 94:2 ANDA DIR
        3,   // 95:2 BITA DIR
        3,   // 96:2 LDAA DIR
        4,   // 97:2 STAA DIR
        3,   // 98:2 EORA DIR
        3,   // 99:2 ADCA DIR
        3,   // 9A:2 ORAA DIR
        3,   // 9B:2 ADDA DIR
        4,   // 9C:2 CPX  DIR
        0,   // 9D:0 -    -
        4,   // 9E:2 LDS  DIR
        5,   // 9F:2 STS  DIR
        5,   // A0:2 SUBA IDX
        5,   // A1:2 CMPA IDX
        5,   // A2:2 SBCA IDX
        0,   // A3:0 -    -
        5,   // A4:2 ANDA IDX
        5,   // A5:2 BITA IDX
        5,   // A6:2 LDAA IDX
        6,   // A7:2 STAA IDX
        5,   // A8:2 EORA IDX
        5,   // A9:2 ADCA IDX
        5,   // AA:2 ORAA IDX
        5,   // AB:2 ADDA IDX
        6,   // AC:2 CPX  IDX
        8,   // AD:2 JSR  IDX
        6,   // AE:2 LDS  IDX
        7,   // AF:2 STS  IDX
        4,   // B0:3 SUBA EXT
        4,   // B1:3 CMPA EXT
        4,   // B2:3 SBCA EXT
        0,   // B3:0 -    -
        4,   // B4:3 ANDA EXT
        4,   // B5:3 BITA EXT
        4,   // B6:3 LDAA EXT
        5,   // B7:3 STAA EXT
        4,   // B8:3 EORA EXT
        4,   // B9:3 ADCA EXT
        4,   // BA:3 ORAA EXT
        4,   // BB:3 ADDA EXT
        5,   // BC:3 CPX  EXT
        9,   // BD:3 JSR  EXT
        5,   // BE:3 LDS  EXT
        6,   // BF:3 STS  EXT
        2,   // C0:2 SUBB IMM
        2,   // C1:2 CMPB IMM
        2,   // C2:2 SBCB IMM
        0,   // C3:0 -    -
        2,   // C4:2 ANDB IMM
        2,   // C5:2 BITB IMM
        2,   // C6:2 LDAB IMM
        0,   // C7:0 -    -
        2,   // C8:2 EORB IMM
        2,   // C9:2 ADCB IMM
        2,   // CA:2 ORAB IMM
        2,   // CB:2 ADDB IMM
        0,   // CC:0 -    -
        0,   // CD:0 -    -
        3,   // CE:3 LDX  IMM
        0,   // CF:0 -    -
        3,   // D0:2 SUBB DIR
        3,   // D1:2 CMPB DIR
        3,   // D2:2 SBCB DIR
        0,   // D3:0 -    -
        3,   // D4:2 ANDB DIR
        3,   // D5:2 BITB DIR
        3,   // D6:2 LDAB DIR
        4,   // D7:2 STAB DIR
        3,   // D8:2 EORB DIR
        3,   // D9:2 ADCB DIR
        3,   // DA:2 ORAB DIR
        3,   // DB:2 ADDB DIR
        0,   // DC:0 -    -
        0,   // DD:0 -    -
        4,   // DE:2 LDX  DIR
        5,   // DF:2 STX  DIR
        5,   // E0:2 SUBB IDX
        5,   // E1:2 CMPB IDX
        5,   // E2:2 SBCB IDX
        0,   // E3:0 -    -
        5,   // E4:2 ANDB IDX
        5,   // E5:2 BITB IDX
        5,   // E6:2 LDAB IDX
        6,   // E7:2 STAB IDX
        5,   // E8:2 EORB IDX
        5,   // E9:2 ADCB IDX
        5,   // EA:2 ORAB IDX
        5,   // EB:2 ADDB IDX
        0,   // EC:0 -    -
        0,   // ED:0 -    -
        6,   // EE:2 LDX  IDX
        7,   // EF:2 STX  IDX
        4,   // F0:3 SUBB EXT
        4,   // F1:3 CMPB EXT
        4,   // F2:3 SBCB EXT
        0,   // F3:0 -    -
        4,   // F4:3 ANDB EXT
        4,   // F5:3 BITB EXT
        4,   // F6:3 LDAB EXT
        5,   // F7:3 STAB EXT
        4,   // F8:3 EORB EXT
        4,   // F9:3 ADCB EXT
        4,   // FA:3 ORAB EXT
        4,   // FB:3 ADDB EXT
        0,   // FC:0 -    -
        0,   // FD:0 -    -
        5,   // FE:3 LDX  EXT
        6,   // FF:3 STX  EXT
};

uint8_t Regs::cycles(uint8_t insn) const {
    return cycles_table[insn];
}

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
    return "6801";
}

static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void Regs::print() const {
    // text=35, reg=8*2, cc=8, eos=1
    char buffer[20 + 8 * 2 + 8 + 1];
    char *p = buffer;
    p = outHex16(outText(p, "PC="), pc);
    p = outHex16(outText(p, " SP="), sp);
    p = outHex16(outText(p, " X="), x);
    p = outHex8(outText(p, " A="), a);
    p = outHex8(outText(p, " B="), b);
    p = outText(p, " CC=");
    *p++ = bit1(cc & 0x20, 'H');
    *p++ = bit1(cc & 0x10, 'I');
    *p++ = bit1(cc & 0x08, 'N');
    *p++ = bit1(cc & 0x04, 'Z');
    *p++ = bit1(cc & 0x02, 'V');
    *p++ = bit1(cc & 0x01, 'C');
    *p = 0;
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

void Regs::save(bool show) {
    static const uint8_t SWI[2] = {0x3F, 0xFF};  // SWI

    uint8_t bytes[10];
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
    static uint8_t LDS[3] = {0x8E}; // LDS #sp
    static uint8_t RTI[10] = {0x3B, 0xFF, 0xFF}; // RTI
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

    Signals::resetCycles();
    Pins.execInst(LDS, sizeof(LDS));
    Pins.execInst(RTI, sizeof(RTI));
    if (show)
        Signals::printCycles();
}

void Regs::set(const Signals *stack) {
    sp = stack[0].addr;
    pc = uint16(stack[1].data, stack[0].data);
    x = uint16(stack[3].data, stack[2].data);
    a = stack[4].data;
    b = stack[5].data;
    cc = stack[6].data;
}

void Regs::printRegList() const {
    cli.println(F("?Reg: pc sp x a b cc"));
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
    if (reg == 'p' || reg == 's' || reg == 'x') {
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
    print();
    return true;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
