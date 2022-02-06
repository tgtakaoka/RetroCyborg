#include "regs.h"

#include <libcli.h>

#include <asm_ins8070.h>
#include <dis_ins8070.h>
#include "config.h"
#include "digital_fast.h"
#include "mc6850.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;
extern Mc6850 Acia;

libasm::ins8070::AsmIns8070 asm8070;
libasm::ins8070::DisIns8070 dis8070;
libasm::Assembler &assembler(asm8070);
libasm::Disassembler &disassembler(dis8070);

struct Regs Regs;
struct Memory Memory;

// Effective address type
// Assumption:
// - no instruction on internal RAM.
// - two bytes operands are fit in internal RAM.
enum AddrMode : uint8_t {
    M_NO = 0,
    M_DISP = 1,  // disp,PC/SP/P2/P3
    M_PUSH = 2,  // --SP
    M_POP = 3,   // SP++
    M_AUTO = 4,  // @disp,P2/P3
    M_DIR = 5,   // direct page
    M_SSM = 6,   // SSM instruction
};

static constexpr uint8_t E(AddrMode addr_mode, uint8_t insn_len, uint8_t bus) {
    return (uint16_t(addr_mode) << 5) | (insn_len << 3) | bus;
}

static constexpr AddrMode addr_mode(uint8_t e) {
    return AddrMode(e >> 5);
}

static constexpr uint8_t insn_len(uint8_t e) {
    return (e >> 3) & 3;
}

static constexpr uint8_t bus_cycles(uint8_t e) {
    return e & 7;
}

static constexpr uint8_t insn_table[256] = {
        E(M_NO, 1, 1),    // 00:NOP  -
        E(M_NO, 1, 1),    // 01:XCH  A,E
        E(M_NO, 0, 0),    // 02:-    -
        E(M_NO, 0, 0),    // 03:-    -
        E(M_NO, 0, 0),    // 04:-    -
        E(M_NO, 0, 0),    // 05:-    -
        E(M_NO, 1, 1),    // 06:LD   A,S
        E(M_NO, 1, 1),    // 07:LD   S,A
        E(M_PUSH, 1, 3),  // 08:PUSH EA
        E(M_NO, 1, 1),    // 09:LD   T,EA
        E(M_PUSH, 1, 2),  // 0A:PUSH A
        E(M_NO, 1, 1),    // 0B:LD   EA,T
        E(M_NO, 1, 1),    // 0C:SR   EA
        E(M_NO, 1, 1),    // 0D:DIV  EA,T
        E(M_NO, 1, 1),    // 0E:SL   A
        E(M_NO, 1, 1),    // 0F:SL   EA
        E(M_PUSH, 1, 5),  // 10:CALL 0
        E(M_PUSH, 1, 5),  // 11:CALL 1
        E(M_PUSH, 1, 5),  // 12:CALL 2
        E(M_PUSH, 1, 5),  // 13:CALL 3
        E(M_PUSH, 1, 5),  // 14:CALL 4
        E(M_PUSH, 1, 5),  // 15:CALL 5
        E(M_PUSH, 1, 5),  // 16:CALL 6
        E(M_PUSH, 1, 5),  // 17:CALL 7
        E(M_PUSH, 1, 5),  // 18:CALL 8
        E(M_PUSH, 1, 5),  // 19:CALL 9
        E(M_PUSH, 1, 5),  // 1A:CALL 10
        E(M_PUSH, 1, 5),  // 1B:CALL 11
        E(M_PUSH, 1, 5),  // 1C:CALL 12
        E(M_PUSH, 1, 5),  // 1D:CALL 13
        E(M_PUSH, 1, 5),  // 1E:CALL 14
        E(M_PUSH, 1, 5),  // 1F:CALL 15
        E(M_PUSH, 3, 5),  // 20:JSR  ADDR
        E(M_NO, 0, 0),    // 21:-    -
        E(M_PUSH, 3, 5),  // 22:PLI  P2,=ADDR
        E(M_PUSH, 3, 5),  // 23:PLI  P3,=ADDR
        E(M_NO, 3, 3),    // 24:JMP  ADDR
        E(M_NO, 3, 3),    // 25:LD   SP,=ADDR
        E(M_NO, 3, 3),    // 26:LD   P2,=ADDR
        E(M_NO, 3, 3),    // 27:LD   P3,=ADDR
        E(M_NO, 0, 0),    // 28:-    -
        E(M_NO, 0, 0),    // 29:-    -
        E(M_NO, 0, 0),    // 2A:-    -
        E(M_NO, 0, 0),    // 2B:-    -
        E(M_NO, 1, 1),    // 2C:MPY  EA,T
        E(M_NO, 2, 2),    // 2D:BND  ADDR
        // TODO: Implement step SSM instruction
        E(M_SSM, 1, 1),   // 2E:SSM  P2
        E(M_SSM, 1, 1),   // 2F:SSM  P3
        E(M_NO, 1, 1),    // 30:LD   EA,PC
        E(M_NO, 1, 1),    // 31:LD   EA,SP
        E(M_NO, 1, 1),    // 32:LD   EA,P2
        E(M_NO, 1, 1),    // 33:LD   EA,P3
        E(M_NO, 0, 0),    // 34:-    -
        E(M_NO, 0, 0),    // 35:-    -
        E(M_NO, 0, 0),    // 36:-    -
        E(M_NO, 0, 0),    // 37:-    -
        E(M_POP, 1, 2),   // 38:POP  A
        E(M_NO, 2, 2),    // 39:AND  S,=DATA
        E(M_POP, 1, 3),   // 3A:POP  EA
        E(M_NO, 2, 2),    // 3B:OR   S,=DATA
        E(M_NO, 1, 1),    // 3C:SR   A
        E(M_NO, 1, 1),    // 3D:SRL  A
        E(M_NO, 1, 1),    // 3E:RR   A
        E(M_NO, 1, 1),    // 3F:RRL  A
        E(M_NO, 1, 1),    // 40:LD   A,E
        E(M_NO, 0, 0),    // 41:-    -
        E(M_NO, 0, 0),    // 42:-    -
        E(M_NO, 0, 0),    // 43:-    -
        E(M_NO, 1, 1),    // 44:LD   PC,EA
        E(M_NO, 1, 1),    // 45:LD   SP,EA
        E(M_NO, 1, 1),    // 46:LD   P2,EA
        E(M_NO, 1, 1),    // 47:LD   P3,EA
        E(M_NO, 1, 1),    // 48:LD   E,A
        E(M_NO, 0, 0),    // 49:-    -
        E(M_NO, 0, 0),    // 4A:-    -
        E(M_NO, 0, 0),    // 4B:-    -
        E(M_NO, 1, 1),    // 4C:XCH  EA,PC
        E(M_NO, 1, 1),    // 4D:XCH  EA,SP
        E(M_NO, 1, 1),    // 4E:XCH  EA,P2
        E(M_NO, 1, 1),    // 4F:XCH  EA,P3
        E(M_NO, 1, 1),    // 50:AND  A,E
        E(M_NO, 0, 0),    // 51:-    -
        E(M_NO, 0, 0),    // 52:-    -
        E(M_NO, 0, 0),    // 53:-    -
        E(M_PUSH, 1, 3),  // 54:PUSH PC
        E(M_PUSH, 0, 0),  // 55:-    -
        E(M_PUSH, 1, 3),  // 56:PUSH P2
        E(M_PUSH, 1, 3),  // 57:PUSH P3
        E(M_NO, 1, 1),    // 58:OR   A,E
        E(M_NO, 0, 0),    // 59:-    -
        E(M_NO, 0, 0),    // 5A:-    -
        E(M_NO, 0, 0),    // 5B:-    -
        E(M_POP, 1, 3),   // 5C:RET  -
        E(M_NO, 0, 0),    // 5D:-    -
        E(M_POP, 1, 3),   // 5E:POP  P2
        E(M_POP, 1, 3),   // 5F:POP  P3
        E(M_NO, 1, 1),    // 60:XOR  A,E
        E(M_NO, 0, 0),    // 61:-    -
        E(M_NO, 0, 0),    // 62:-    -
        E(M_NO, 0, 0),    // 63:-    -
        E(M_NO, 2, 2),    // 64:BP   ADDR
        E(M_NO, 0, 0),    // 65:-    -
        E(M_NO, 2, 2),    // 66:BP   DISP,P2
        E(M_NO, 2, 2),    // 67:BP   DISP,P3
        E(M_NO, 0, 0),    // 68:-    -
        E(M_NO, 0, 0),    // 69:-    -
        E(M_NO, 0, 0),    // 6A:-    -
        E(M_NO, 0, 0),    // 6B:-    -
        E(M_NO, 2, 2),    // 6C:BZ   ADDR
        E(M_NO, 0, 0),    // 6D:-    -
        E(M_NO, 2, 2),    // 6E:BZ   DISP,P2
        E(M_NO, 2, 2),    // 6F:BZ   DISP,P3
        E(M_NO, 1, 1),    // 70:ADD  A,E
        E(M_NO, 0, 0),    // 71:-    -
        E(M_NO, 0, 0),    // 72:-    -
        E(M_NO, 0, 0),    // 73:-    -
        E(M_NO, 2, 2),    // 74:BRA  ADDR
        E(M_NO, 0, 0),    // 75:-    -
        E(M_NO, 2, 2),    // 76:BRA  DISP,P2
        E(M_NO, 2, 2),    // 77:BRA  DISP.P3
        E(M_NO, 1, 1),    // 78:SUB  A,E
        E(M_NO, 0, 0),    // 79:-    -
        E(M_NO, 0, 0),    // 7A:-    -
        E(M_NO, 0, 0),    // 7B:-    -
        E(M_NO, 2, 2),    // 7C:BNZ  ADDR
        E(M_NO, 0, 0),    // 7D:-    -
        E(M_NO, 2, 2),    // 7E:BNZ  DISP,P2
        E(M_NO, 2, 2),    // 7F:BNZ  DISP,P3
        E(M_DISP, 2, 4),  // 80:LD   EA,DIPS,PC
        E(M_DISP, 2, 4),  // 81:LD   EA,DISP,SP
        E(M_DISP, 2, 4),  // 82:LD   EA,DISP,P2
        E(M_DISP, 2, 4),  // 83:LD   EA,DISP,P3
        E(M_NO, 3, 3),    // 84:LD   EA,=DATA
        E(M_DIR, 2, 4),   // 85:LD   EA,DIR
        E(M_AUTO, 2, 4),  // 86:LD   EA,@AUTO,P2
        E(M_AUTO, 2, 4),  // 87:LD   EA,@AUTO,P3
        E(M_DISP, 2, 4),  // 88:ST   EA,DISP,PC
        E(M_DISP, 2, 4),  // 89:ST   EA,DISP,SP
        E(M_DISP, 2, 4),  // 8A:ST   EA,DISP,P2
        E(M_DISP, 2, 4),  // 8B:ST   EA,DISP,P3
        E(M_NO, 0, 0),    // 8C:-    -
        E(M_DIR, 2, 4),   // 8D:ST   EA,DIR
        E(M_AUTO, 2, 4),  // 8E:ST   EA,@AUTO,P2
        E(M_AUTO, 2, 4),  // 8F:ST   EA,@AUTO,P3
        E(M_DISP, 2, 4),  // 90:ILD  A,DISP,PC
        E(M_DISP, 2, 4),  // 91:ILD  A,DISP,SP
        E(M_DISP, 2, 4),  // 92:ILD  A,DISP,P2
        E(M_DISP, 2, 4),  // 93:ILD  A,DISP,P3
        E(M_NO, 0, 0),    // 94:-    -
        E(M_DIR, 2, 4),   // 95:ILD  A,DIR
        E(M_AUTO, 2, 4),  // 96:ILD  A,@AUTO,P2
        E(M_AUTO, 2, 4),  // 97:ILD  A,@AUTO,P3
        E(M_DISP, 2, 4),  // 98:DLD  A,DISP,PC
        E(M_DISP, 2, 4),  // 99:DLD  A,DISP,SP
        E(M_DISP, 2, 4),  // 9A:DLD  A,DISP,P2
        E(M_DISP, 2, 4),  // 9B:DLD  A,DISP,P3
        E(M_NO, 0, 0),    // 9C:-    -
        E(M_DIR, 2, 4),   // 9D:DLD  A,DIR
        E(M_AUTO, 2, 4),  // 9E:DLD  A,@AUTO,P2
        E(M_AUTO, 2, 4),  // 9F:DLD  A,@AUTO,P3
        E(M_DISP, 2, 4),  // A0:LD   T,DISP,PC
        E(M_DISP, 2, 4),  // A1:LD   T,DISP,SP
        E(M_DISP, 2, 4),  // A2:LD   T,DISP,P2
        E(M_DISP, 2, 4),  // A3:LD   T,DISP,P3
        E(M_NO, 3, 3),    // A4:LD   T,=DATA
        E(M_DIR, 2, 4),   // A5:LD   T,DIR
        E(M_AUTO, 2, 4),  // A6:LD   T,@AUTO,P2
        E(M_AUTO, 2, 4),  // A7:LD   T,@AUTO,P3
        E(M_NO, 0, 0),    // A8:-    -
        E(M_NO, 0, 0),    // A9:-    -
        E(M_NO, 0, 0),    // AA:-    -
        E(M_NO, 0, 0),    // AB:-    -
        E(M_NO, 0, 0),    // AC:-    -
        E(M_NO, 0, 0),    // AD:-    -
        E(M_NO, 0, 0),    // AE:-    -
        E(M_NO, 0, 0),    // AF:-    -
        E(M_DISP, 2, 4),  // B0:ADD  EA,DISP,PC
        E(M_DISP, 2, 4),  // B1:ADD  EA,DISP,SP
        E(M_DISP, 2, 4),  // B2:ADD  EA,DISP,P2
        E(M_DISP, 2, 4),  // B3:ADD  EA,DISP,P3
        E(M_NO, 2, 3),    // B4:ADD  EA,=DATA
        E(M_DIR, 2, 4),   // B5:ADD  EA,DIR
        E(M_AUTO, 2, 4),  // B6:ADD  EA,@AUTO,P2
        E(M_AUTO, 2, 4),  // B7:ADD  EA,@AUTO,P3
        E(M_DISP, 2, 4),  // B8:SUB  EA,DISP,PC
        E(M_DISP, 2, 4),  // B9:SUB  EA,DISP,SP
        E(M_DISP, 2, 4),  // BA:SUB  EA,DISP,P2
        E(M_DISP, 2, 4),  // BB:SUB  EA,DISP,P3
        E(M_NO, 2, 3),    // BC:SUB  EA,=DATA
        E(M_DIR, 2, 4),   // BD:SUB  EA,DIR
        E(M_AUTO, 2, 4),  // BE:SUB  EA,@AUTO,P2
        E(M_AUTO, 2, 4),  // BF:SUB  EA,@AUTO,P3
        E(M_DISP, 2, 3),  // C0:LD   A,DIPS,PC
        E(M_DISP, 2, 3),  // C1:LD   A,DISP,SP
        E(M_DISP, 2, 3),  // C2:LD   A,DISP,P2
        E(M_DISP, 2, 3),  // C3:LD   A,DISP,P3
        E(M_NO, 2, 2),    // C4:LD   A,=DATA
        E(M_DIR, 2, 3),   // C5:LD   A,DIR
        E(M_AUTO, 2, 3),  // C6:LD   A,@AUTO,P2
        E(M_AUTO, 2, 3),  // C7:LD   A,@AUTO,P3
        E(M_DISP, 2, 3),  // C8:ST   A,DISP,PC
        E(M_DISP, 2, 3),  // C9:ST   A,DISP,SP
        E(M_DISP, 2, 3),  // CA:ST   A,DISP,P2
        E(M_DISP, 2, 3),  // CB:ST   A,DISP,P3
        E(M_NO, 0, 0),    // CC:-    -
        E(M_DIR, 2, 3),   // CD:ST   A,DIR
        E(M_AUTO, 2, 3),  // CE:ST   A,@AUTO,P2
        E(M_AUTO, 2, 3),  // CF:ST   A,@AUTO,P3
        E(M_DISP, 2, 3),  // D0:AND  A,DISP,PC
        E(M_DISP, 2, 3),  // D1:AND  A,DISP,SP
        E(M_DISP, 2, 3),  // D2:AND  A,DISP,P2
        E(M_DISP, 2, 3),  // D3:AND  A,DISP,P3
        E(M_NO, 2, 2),    // D4:AND  A,=DATA
        E(M_DIR, 2, 3),   // D5:AND  A,DIR
        E(M_AUTO, 2, 3),  // D6:AND  A,@AUTO,P2
        E(M_AUTO, 2, 3),  // D7:AND  A,@AUTO,P3
        E(M_DISP, 2, 3),  // D8:OR   A,DISP,PC
        E(M_DISP, 2, 3),  // D9:OR   A,DISP,SP
        E(M_DISP, 2, 3),  // DA:OR   A,DISP,P2
        E(M_DISP, 2, 3),  // DB:OR   A,DISP,P3
        E(M_NO, 2, 2),    // DC:OR   A,=DATA
        E(M_DIR, 2, 3),   // DD:OR   A,DIR
        E(M_AUTO, 2, 3),  // DE:OR   A,@AUTO,P2
        E(M_AUTO, 2, 3),  // DF:OR   A,@AUTO,P3
        E(M_DISP, 2, 3),  // E0:XOR  A,DISP,PC
        E(M_DISP, 2, 3),  // E1:XOR  A,DISP,SP
        E(M_DISP, 2, 3),  // E2:XOR  A,DISP,P2
        E(M_DISP, 2, 3),  // E3:XOR  A,DISP,P3
        E(M_NO, 2, 2),    // E4:XOR  A,=DATA
        E(M_DIR, 2, 3),   // E5:XOR  A,DIR
        E(M_AUTO, 2, 3),  // E6:XOR  A,@AUTO,P2
        E(M_AUTO, 2, 3),  // E7:XOR  A,@AUTO,P3
        E(M_NO, 0, 0),    // E8:-    -
        E(M_NO, 0, 0),    // E9:-    -
        E(M_NO, 0, 0),    // EA:-    -
        E(M_NO, 0, 0),    // EB:-    -
        E(M_NO, 0, 0),    // EC:-    -
        E(M_NO, 0, 0),    // ED:-    -
        E(M_NO, 0, 0),    // EE:-    -
        E(M_NO, 0, 0),    // EF:-    -
        E(M_DISP, 2, 3),  // F0:ADD  A,DISP,PC
        E(M_DISP, 2, 3),  // F1:ADD  A,DISP,SP
        E(M_DISP, 2, 3),  // F2:ADD  A,DISP,P2
        E(M_DISP, 2, 3),  // F3:ADD  A,DISP,P3
        E(M_NO, 2, 2),    // F4:ADD  A,=DATA
        E(M_DIR, 2, 3),   // F5:ADD  A,DIR
        E(M_AUTO, 2, 3),  // F6:ADD  A,@AUTO,P2
        E(M_AUTO, 2, 3),  // F7:ADD  A,@AUTO,P3
        E(M_DISP, 2, 3),  // F8:SUB  A,DISP,PC
        E(M_DISP, 2, 3),  // F9:SUB  A,DISP,SP
        E(M_DISP, 2, 3),  // FA:SUB  A,DISP,P2
        E(M_DISP, 2, 3),  // FB:SUB  A,DISP,P3
        E(M_NO, 2, 2),    // FC:SUB  A,=DATA
        E(M_DIR, 2, 3),   // FD:SUB  A,DIR
        E(M_AUTO, 2, 3),  // FE:SUB  A,@AUTO,P2
        E(M_AUTO, 2, 3),  // FF:SUB  A,@AUTO,P3
};

uint8_t Regs::busCycles(uint8_t insn) {
    return bus_cycles(insn_table[insn]);
}

uint8_t Regs::insnLen(uint8_t insn) {
    return insn_len(insn_table[insn]);
}

uint16_t Regs::selectBase(uint8_t insn) const {
    switch (insn & 3) {
    case 1:
        return sp;
    case 2:
        return p2;
    case 3:
        return p3;
    default:
        return pc;
    }
}

uint16_t Regs::effectiveAddr(uint8_t insn, uint16_t insnp) const {
    const auto e = insn_table[insn];
    const auto mode = addr_mode(e);
    const auto opr = Memory.raw_read(insnp + 1);
    const auto disp = static_cast<int8_t>(opr);
    const auto base = selectBase(insn);
    switch (mode) {
    case M_DISP:
        return base + disp;
    case M_PUSH:
        return sp - 1;
    case M_POP:
        return sp;
    case M_AUTO:
        return disp < 0 ? base - disp : base;
    case M_DIR:
        return 0xff00 + opr;
    case M_SSM:
        return base;
    default:
        return 0;
    }
}

const char *Regs::cpu() const {
    return "INS8070";
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
        'P', '2', '=', 0, 0, 0, 0, ' ',  // P2=19
        'P', '3', '=', 0, 0, 0, 0, ' ',  // P3=27
        'E', '=', 0, 0, ' ',             // E=34
        'A', '=', 0, 0, ' ',             // A=39
        'T', '=', 0, 0, 0, 0,            // T=44
        0,                               // EOS
    };
    static char s_bits[] = {
        ' ', 'S', '=',
        0, 0, 0, 0, 0, 0, 0, 0,          // S=3
        0,
    };
    outHex16(buffer + 3, pc);
    outHex16(buffer + 11, sp);
    outHex16(buffer + 19, p2);
    outHex16(buffer + 27, p3);
    outHex8(buffer + 34, e);
    outHex8(buffer + 39, a);
    outHex16(buffer + 44, t);
    cli.print(buffer);
    s_bits[3] = bit1(s & 0x80, 'C');
    s_bits[4] = bit1(s & 0x40, 'V');
    s_bits[5] = bit1(s & 0x20, 'B');
    s_bits[6] = bit1(s & 0x10, 'A');
    s_bits[7] = bit1(s & 0x08, '3');
    s_bits[8] = bit1(s & 0x04, '2');
    s_bits[9] = bit1(s & 0x02, '1');
    s_bits[10] = bit1(s & 0x01, 'I');
    cli.println(s_bits);
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
    static const uint8_t PUSH_ALL[] = {
        0x88, 0xFE,             // ST EA,-1,PC
        0x31, 0x88, 0xFE,       // LD EA,SP; ST EA,-1,PC
        0x25, 0x00, 0x01,       // LD SP,=0x0100
        0x56,                   // PUSH P2
        0x57,                   // PUSH P3
        0x0B, 0x08,             // LD EA,T; PUSH EA
        0x06, 0x0A,             // LD A,S; PUSH A
    };
    static uint8_t buffer[11];
    if (show)
        Signals::resetCycles();
    Pins.captureWrites(PUSH_ALL, sizeof(PUSH_ALL), &pc, buffer, sizeof(buffer));
    if (show)
        Signals::printCycles();
    a = buffer[0];
    e = buffer[1];
    sp = le16(buffer + 2);
    p2 = be16(buffer + 4);
    p3 = be16(buffer + 6);
    t = be16(buffer + 8);
    s = buffer[10];
}

void Regs::restore(bool show) {
    static uint8_t LD_ALL[] = {
        0x27, 0, 0,             // p3=2:1; LD P3,=p3
        0x26, 0, 0,             // p2=5:4; LD P2,=p2
        0x25, 0, 0,             // sp=8:7; LD SP,=sp
        0xA4, 0, 0,             // t=11:10; LD T,=t
        0x84, 0, 0,             // e=14 s=13; LD EA,=e|s
        0x07,                   // LD S,A
        0xC4, 0,                // a=17; LD A,=a
        0x24, 0, 0,             // pc=20:19; JMP =pc
    };

    if (show)
        Signals::resetCycles();
    LD_ALL[1] = lo(p3);
    LD_ALL[2] = hi(p3);
    LD_ALL[4] = lo(p2);
    LD_ALL[5] = hi(p2);
    LD_ALL[7] = lo(sp);
    LD_ALL[8] = hi(sp);
    LD_ALL[10] = lo(t);
    LD_ALL[11] = hi(t);
    LD_ALL[13] = s;
    LD_ALL[14] = e;
    LD_ALL[17] = a;
    LD_ALL[19] = lo(pc);
    LD_ALL[20] = hi(pc);
    Pins.execInst(LD_ALL, sizeof(LD_ALL));
    if (show)
        Signals::printCycles();
}

void Regs::printRegList() const {
    cli.println(F("?Reg: PC SP P2 P3 A E EA T S"));
}

char Regs::validUint8Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("A")) == 0)
        return 'a';
    if (strcasecmp_P(word, PSTR("E")) == 0)
        return 'e';
    if (strcasecmp_P(word, PSTR("S")) == 0)
        return 's';
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("PC")) == 0)
        return 'p';
    if (strcasecmp_P(word, PSTR("SP")) == 0)
        return 'S';
    if (strcasecmp_P(word, PSTR("P2")) == 0)
        return '2';
    if (strcasecmp_P(word, PSTR("P3")) == 0)
        return '3';
    if (strcasecmp_P(word, PSTR("EA")) == 0)
        return 'E';
    if (strcasecmp_P(word, PSTR("T")) == 0)
        return 'T';
    return 0;
}

void Regs::setRegValue(char reg, uint32_t value) {
    switch (reg) {
    case 'p':
        pc = value;
        break;
    case 'S':
        sp = value;
        break;
    case '2':
        p2 = value;
        break;
    case '3':
        p3 = value;
        break;
    case 'a':
        a = value;
        break;
    case 'e':
        e = value;
        break;
    case 'E':
        setEa(value);
        break;
    case 'T':
        t = value;
        break;
    case 's':
        s = value;
        break;
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
    disassembler.setUppercase(true);
    uint16_t num = 0;
    while (num < numInsn) {
        char operands[20];
        libasm::Insn insn(addr);
        Memory.setAddress(addr);
        disassembler.decode(Memory, insn, operands, sizeof(operands));
        addr += insn.length();
        num++;
        printInsn(insn);
        if (disassembler.getError()) {
            cli.print(F("Error: "));
            cli.println(disassembler.errorText(disassembler.getError()));
            continue;
        }
        cli.printStr(insn.name(), -6);
        cli.printlnStr(operands, -12);
    }
    return addr;
}

uint16_t Regs::assemble(uint16_t addr, const char *line) const {
    assembler.setCpu(cpu());
    libasm::Insn insn(addr);
    if (assembler.encode(line, insn)) {
        cli.print(F("Error: "));
        cli.println(assembler.errorText(assembler.getError()));
    } else {
        Memory.write(insn.address(), insn.bytes(), insn.length());
        disassemble(insn.address(), 1);
        addr += insn.length();
    }
    return addr;
}

uint8_t Memory::internal_read(uint8_t addr) const {
    // No bus signals while internal RAM bus cycle.
    static uint8_t LD_ST[] = {
        0xC5, 0,                // LD A,dir[addr]
        0xCD, 0x00              // ST A,dir[0x00]
    };
    LD_ST[1] = addr;
    uint8_t data;
    Pins.captureWrites(LD_ST, sizeof(LD_ST), nullptr, &data, sizeof(data));
    return data;
}

void Memory::internal_write(uint8_t addr, uint8_t data) const {
    // No bus signals while internal RAM bus cycle.
    static uint8_t LD_ST[] = {
        0xC4, 0,                // LD A,data
        0xCD, 0                 // ST A,dir[addr]
    };
    LD_ST[1] = data;
    LD_ST[3] = addr;
    Pins.execInst(LD_ST, sizeof(LD_ST));
}

bool Memory::is_internal(uint16_t addr) {
    return addr >= 0xFFC0;
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
