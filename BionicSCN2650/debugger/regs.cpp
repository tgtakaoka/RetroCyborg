#include "regs.h"

#include <libcli.h>

#include <asm_z8.h>
#include <dis_z8.h>
#include "config.h"
#include "digital_fast.h"
#include "pins.h"
#include "string_util.h"

#include <ctype.h>
#include <string.h>

extern libcli::Cli cli;

static constexpr bool debug_cycles = false;

libasm::z8::AsmZ8 assembler;
libasm::z8::DisZ8 disassembler;

struct Regs Regs;
struct Memory Memory;

static constexpr uint8_t E(
        uint8_t insn_len, uint8_t external_cycles, uint8_t stack_cycles) {
    return static_cast<uint8_t>(
            insn_len | (external_cycles << 4) | (stack_cycles << 6));
}

static constexpr uint8_t insn_len(uint8_t e) {
    return e & 7;
}

static constexpr uint8_t external_cycles(uint8_t e) {
    return (e >> 4) & 3;
}

static constexpr uint8_t stack_cycles(uint8_t e) {
    return (e >> 6) & 3;
}

static constexpr uint8_t bus_cycles(uint8_t e) {
    return external_cycles(e)
#if Z88_EXTERNAL_STACK == 1
           + stack_cycles(e)
#endif
            ;
}

// clang-format: off
static constexpr uint8_t insn_table[256] = {
        E(2, 0, 0),  // 00:DEC    r
        E(2, 0, 0),  // 01:DEC    @r
        E(2, 0, 0),  // 02:ADD    r,r
        E(2, 0, 0),  // 03:ADD    r,@r
        E(3, 0, 0),  // 04:ADD    R,R
        E(3, 0, 0),  // 05:ADD    @R,R
        E(3, 0, 0),  // 06:ADD    R,#IM
        E(3, 0, 0),  // 07:BOR    r0,R,#b
        E(2, 0, 0),  // 08:LD     r0,R
        E(2, 0, 0),  // 09:LD     R,r0
        E(2, 0, 0),  // 0A:DJNZ   r0,RA
        E(2, 0, 0),  // 0B:JR     F,RA
        E(2, 0, 0),  // 0C:LD     r0,#IM
        E(3, 0, 0),  // 0D:JP     F,DA
        E(1, 0, 0),  // 0E:INC    r0
        E(1, 2, 0),  // 0F:NEXT   -
        E(2, 0, 0),  // 10:RLC    R
        E(2, 0, 0),  // 11:RLC    @R
        E(2, 0, 0),  // 12:ADC    r,r
        E(2, 0, 0),  // 13:ADC    r,@r
        E(3, 0, 0),  // 14:ADC    R,R
        E(3, 0, 0),  // 15:ADC    R,@R
        E(3, 0, 0),  // 16:ADC    R,#IM
        E(3, 0, 0),  // 17:BCP    r0,R,#b
        E(2, 0, 0),  // 18:LD     r1,R
        E(2, 0, 0),  // 19:LD     R,r1
        E(2, 0, 0),  // 1A:DJNZ   r1,RA
        E(2, 0, 0),  // 1B:JR     LT,RA
        E(2, 0, 0),  // 1C:LD     r1,#IM
        E(3, 0, 0),  // 1D:JP     LT,DA
        E(1, 0, 0),  // 1E:INC    r1
        E(1, 2, 2),  // 1F:ENTER  -
        E(2, 0, 0),  // 20:INC    R
        E(2, 0, 0),  // 21:INC    @R
        E(2, 0, 0),  // 22:SUB    r,r
        E(2, 0, 0),  // 23:SUB    r,@r
        E(3, 0, 0),  // 24:SUB    R,R
        E(3, 0, 0),  // 25:SUB    R,@R
        E(3, 0, 0),  // 26:SUB    R,#IM
        E(3, 0, 0),  // 27:BXOR   r0,R,#b
        E(2, 0, 0),  // 28:LD     r2,R
        E(2, 0, 0),  // 29:LD     R,r2
        E(2, 0, 0),  // 2A:DJNZ   r2,RA
        E(2, 0, 0),  // 2B:JR     LE,RA
        E(2, 0, 0),  // 2C:LD     r2,#IM
        E(3, 0, 0),  // 2D:JP     LE,DA
        E(1, 0, 0),  // 2E:INC    r2
        E(1, 2, 2),  // 2F:EXIT   -
        E(2, 0, 0),  // 30:JP     @RR
        E(2, 0, 0),  // 31:SRP    #IM
        E(2, 0, 0),  // 32:SBC    r,r
        E(2, 0, 0),  // 33:SBC    r,@r
        E(3, 0, 0),  // 34:SBC    R,R
        E(3, 0, 0),  // 35:SBC    R,@R
        E(3, 0, 0),  // 36:SBC    R,#IM
        E(3, 0, 0),  // 37:BTJRF  RA,r,#b
        E(2, 0, 0),  // 38:LD     r3,R
        E(2, 0, 0),  // 39:LD     R,r3
        E(2, 0, 0),  // 3A:DJNZ   r3,RA
        E(2, 0, 0),  // 3B:JR     ULE,RA
        E(2, 0, 0),  // 3C:LD     r3,#IM
        E(3, 0, 0),  // 3D:JP     ULE,DA
        E(1, 0, 0),  // 3E:INC    r3
        E(1, 0, 0),  // 3F:WFI    -
        E(2, 0, 0),  // 40:DA     R
        E(2, 0, 0),  // 41:DA     @R
        E(2, 0, 0),  // 42:OR     r,r
        E(2, 0, 0),  // 43:OR     r,@r
        E(3, 0, 0),  // 44:OR     R,R
        E(3, 0, 0),  // 45:OR     R,@R
        E(3, 0, 0),  // 46:OR     R,#IM
        E(3, 0, 0),  // 47:LDB    r0,R,#b
        E(2, 0, 0),  // 48:LD     r4,R
        E(2, 0, 0),  // 49:LD     R,r4
        E(2, 0, 0),  // 4A:DJNZ   r4,RA
        E(2, 0, 0),  // 4B:JR     OV,RA
        E(2, 0, 0),  // 4C:LD     r4,#IM
        E(3, 0, 0),  // 4D:JP     OV,DA
        E(1, 0, 0),  // 4E:INC    r4
        E(1, 0, 0),  // 4F:SB0    -
        E(2, 0, 1),  // 50:POP    R
        E(2, 0, 1),  // 51:POP    @R
        E(2, 0, 0),  // 52:AND    r,r
        E(2, 0, 0),  // 53:AND    r,@r
        E(3, 0, 0),  // 54:AND    R,R
        E(3, 0, 0),  // 55:AND    R,@R
        E(3, 0, 0),  // 56:AND    R,#IM
        E(2, 0, 0),  // 57:BITC   r,#b
        E(2, 0, 0),  // 58:LD     r5,R
        E(2, 0, 0),  // 59:LD     R,r5
        E(2, 0, 0),  // 5A:DJNZ   r5,RA
        E(2, 0, 0),  // 5B:JR     MI,RA
        E(2, 0, 0),  // 5C:LD     r5,#IM
        E(3, 0, 0),  // 5D:JP     MI,DA
        E(1, 0, 0),  // 5E:INC    r5
        E(1, 0, 0),  // 5F:SB1    -
        E(2, 0, 0),  // 60:COM    R
        E(2, 0, 0),  // 61:COM    @R
        E(2, 0, 0),  // 62:TCM    r,r
        E(2, 0, 0),  // 63:TCM    r,@r
        E(3, 0, 0),  // 64:TCM    R,R
        E(3, 0, 0),  // 65:TCM    R,@R
        E(3, 0, 0),  // 66:TCM    R,#IM
        E(3, 0, 0),  // 67:BAND   r0,R,#b
        E(2, 0, 0),  // 68:LD     r6,R
        E(2, 0, 0),  // 69:LD     R,r6
        E(2, 0, 0),  // 6A:DJNZ   r6,RA
        E(2, 0, 0),  // 6B:JR     Z,RA
        E(2, 0, 0),  // 6C:LD     r6,#IM
        E(3, 0, 0),  // 6D:JP     Z,DA
        E(1, 0, 0),  // 6E:INC    r6
        E(0, 0, 0),  // 6F:-      -
        E(2, 0, 1),  // 70:PUSH   R
        E(2, 0, 1),  // 71:PUSH   @R
        E(2, 0, 0),  // 72:TM     r,r
        E(2, 0, 0),  // 73:TM     r,@r
        E(3, 0, 0),  // 74:TM     R,R
        E(3, 0, 0),  // 75:TM     R,@R
        E(3, 0, 0),  // 76:TM     R,#IM
        E(2, 0, 0),  // 77:BITR   r,#B
        E(2, 0, 0),  // 78:LD     r7,R
        E(2, 0, 0),  // 79:LD     R,r7
        E(2, 0, 0),  // 7A:DJNZ   r7,RA
        E(2, 0, 0),  // 7B:JR     C,RA
        E(2, 0, 0),  // 7C:LD     r7,#IM
        E(3, 0, 0),  // 7D:JP     C,DA
        E(1, 0, 0),  // 7E:INC    r7
        E(0, 0, 0),  // 7F:-      -
        E(2, 0, 0),  // 80:DECW   RR
        E(2, 0, 0),  // 81:DECW   @R
        E(3, 0, 0),  // 82:PUSHUD @R,R
        E(3, 0, 0),  // 83:PUSHUI @R,R
        E(3, 0, 0),  // 84:MULT   RR,R
        E(3, 0, 0),  // 85:MULT   RR,@IR
        E(3, 0, 0),  // 86:MULT   RR,#IM
        E(3, 0, 0),  // 87:LD     r,x(r)
        E(2, 0, 0),  // 88:LD     r8,R
        E(2, 0, 0),  // 89:LD     R,r8
        E(2, 0, 0),  // 8A:DJNZ   r8,RA
        E(2, 0, 0),  // 8B:JR     RA
        E(2, 0, 0),  // 8C:LD     r8,#IM
        E(3, 0, 0),  // 8D:JP     DA
        E(1, 0, 0),  // 8E:INC    r8
        E(1, 0, 0),  // 8F:DI     -
        E(2, 0, 0),  // 90:RL     R
        E(2, 0, 0),  // 91:RL     @R
        E(3, 0, 0),  // 92:POPUD  R,@R
        E(3, 0, 0),  // 93:POPUI  R,@R
        E(3, 0, 0),  // 94:DIV    RR,R
        E(3, 0, 0),  // 95:DIV    RR,@R
        E(3, 0, 0),  // 96:DIV    RR,#IM
        E(3, 0, 0),  // 97:LD     x(r),r
        E(2, 0, 0),  // 98:LD     r9,R
        E(2, 0, 0),  // 99:LD     R,r9
        E(2, 0, 0),  // 9A:DJNZ   r9,RA
        E(2, 0, 0),  // 9B:JR     GE,RA
        E(2, 0, 0),  // 9C:LD     r9,#IM
        E(3, 0, 0),  // 9D:JP     GE,DA
        E(1, 0, 0),  // 9E:INC    r9
        E(1, 0, 0),  // 9F:EI     -
        E(2, 0, 0),  // A0:INCW   R
        E(2, 0, 0),  // A1:INCW   @R
        E(2, 0, 0),  // A2:CP     r,r
        E(2, 0, 0),  // A3:CP     r,@r
        E(3, 0, 0),  // A4:CP     R,R
        E(3, 0, 0),  // A5:CP     @R,R
        E(3, 0, 0),  // A6:CP     R,#IM
        E(4, 1, 0),  // A7:LDE    r,xl(rr)
        E(2, 0, 0),  // A8:LD     r10,R
        E(2, 0, 0),  // A9:LD     R,r10
        E(2, 0, 0),  // AA:DJNZ   r10,RA
        E(2, 0, 0),  // AB:JR     GT,RA
        E(2, 0, 0),  // AC:LD     r10,#IM
        E(3, 0, 0),  // AD:JP     GT,DA
        E(1, 0, 0),  // AE:INC    r10
        E(1, 1, 2),  // AF:RET    -        fetch as 2 bytes instruction
        E(2, 0, 0),  // B0:CLR    R
        E(2, 0, 0),  // B1:CLR    @R
        E(2, 0, 0),  // B2:XOR    r,r
        E(2, 0, 0),  // B3:XOR    r,@r
        E(3, 0, 0),  // B4:XOR    R,R
        E(3, 0, 0),  // B5:XOR    R,@R
        E(3, 0, 0),  // B6:XOR    R,#IM
        E(4, 1, 0),  // B7:LDE    xl(rr),r
        E(2, 0, 0),  // B8:LD     r11,R
        E(2, 0, 0),  // B9:LD     R,r11
        E(2, 0, 0),  // BA:DJNZ   r11,RA
        E(2, 0, 0),  // BB:JR     UGT,RA
        E(2, 0, 0),  // BC:LD     r11,#IM
        E(3, 0, 0),  // BD:JP     UGT,DA
        E(1, 0, 0),  // BE:INC    r11
        E(1, 1, 3),  // BF:IRET   -        fetch as 2 bytes inustruction
        E(2, 0, 0),  // C0:RRC    R
        E(2, 0, 0),  // C1:RRC    @R
        E(3, 0, 0),  // C2:CPIJE  r,r,RA
        E(2, 1, 0),  // C3:LDE    r,@rr
        E(3, 0, 0),  // C4:LDW    RR,RR
        E(3, 0, 0),  // C5:LDW    RR,@R
        E(4, 0, 0),  // C6:LDW    RR,#IML
        E(2, 0, 0),  // C7:LD     r,@r
        E(2, 0, 0),  // C8:LD     r12,R
        E(2, 0, 0),  // C9:LD     R,r12
        E(2, 0, 0),  // CA:DJNZ   r12,RA
        E(2, 0, 0),  // CB:JR     NOV,RA
        E(2, 0, 0),  // CC:LD     r12,#IM
        E(3, 0, 0),  // CD:JP     NOV,DA
        E(1, 0, 0),  // CE:INC    r12
        E(1, 0, 0),  // CF:RCF    -
        E(2, 0, 0),  // D0:SRA    R
        E(2, 0, 0),  // D1:SRA    @R
        E(3, 0, 0),  // D2:CPIJNE r,r,RA
        E(2, 1, 0),  // D3:LDE    @rr,r
        E(2, 2, 2),  // D4:CALL   #IA
        E(0, 0, 0),  // D5:-      -
        E(3, 0, 0),  // D6:LD     @R,#IM
        E(2, 0, 0),  // D7:LD     @r,r
        E(2, 0, 0),  // D8:LD     r13,R
        E(2, 0, 0),  // D9:LD     R,r13
        E(2, 0, 0),  // DA:DJNZ   r13,RA
        E(2, 0, 0),  // DB:JR     PL,RA
        E(2, 0, 0),  // DC:LD     r13,#IM
        E(3, 0, 0),  // DD:JP     PL,DA
        E(1, 0, 0),  // DE:INC    r13
        E(1, 0, 0),  // DF:SCF    -
        E(2, 0, 0),  // E0:RR     R
        E(2, 0, 0),  // E1:RR     @R
        E(2, 1, 0),  // E2:LDED   r,@rr
        E(2, 1, 0),  // E3:LDEI   r,@rr
        E(3, 0, 0),  // E4:LD     R,R
        E(3, 0, 0),  // E5:LD     R,@R
        E(3, 0, 0),  // E6:LD     R,#IM
        E(3, 1, 0),  // E7:LDE    r,xs(rr)
        E(2, 0, 0),  // E8:LD     r14,R
        E(2, 0, 0),  // E9:LD     R,r14
        E(2, 0, 0),  // EA:DJNZ   r14,RA
        E(2, 0, 0),  // EB:JR     NZ,RA
        E(2, 0, 0),  // EC:LD     r14,#IM
        E(3, 0, 0),  // ED:JP     NZ,DA
        E(1, 0, 0),  // EE:INC    r14
        E(1, 0, 0),  // EF:CCF    -
        E(2, 0, 0),  // F0:SWAP   R
        E(2, 0, 0),  // F1:SWAP   @R
        E(2, 1, 0),  // F2:LDEPD  @rr,r
        E(2, 1, 0),  // F3:LDEPI  @rr,r
        E(2, 1, 2),  // F4:CALL   @RR     fetch as 3 bytes instruction
        E(3, 0, 0),  // F5:LD     @R,R
        E(3, 0, 2),  // F6:CALL   DA
        E(3, 1, 0),  // F7:LDE    xs(rr),r
        E(2, 0, 0),  // F8:LD     r15,R
        E(2, 0, 0),  // F9:LD     R,r15
        E(2, 0, 0),  // FA:DJNZ   r15,RA
        E(2, 0, 0),  // FB:JR     NC,RA
        E(2, 0, 0),  // FC:LD     r15,#IM
        E(3, 0, 0),  // FD:JP     NC,DA
        E(1, 0, 0),  // FE:INC    r15
        E(1, 0, 0),  // FF:NOP    -
};
// clang-format: on

uint8_t Regs::insnLen(uint8_t insn) {
    return insn_len(insn_table[insn]);
}

uint8_t Regs::busCycles(uint8_t insn) {
    return bus_cycles(insn_table[insn]);
}

const char *Regs::cpu() const {
    return "Z88";
}

const char *Regs::cpuName() const {
    return "Z88C00";
}

static char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void Regs::print() const {
    // clang-format off
    static char buffer[] = {
        'P', 'C', '=', 0, 0, 0, 0, ' ',      // PC=3
        'S', 'P', '=', 0, 0, 0, 0, ' ',      // SP=11
        'I', 'P', '=', 0, 0, 0, 0, ' ',      // SP=19
        'R', 'P', '=', 0, 0, ':', 0, 0, ' ', // RP0=27, RP1=30
        'F', '=',                            // F=35
        0, 0, 0, 0, 0, 0, 0, 0,
        0,
    };
    // clang-format on
    outHex16(buffer + 3, _pc);
    outHex8(buffer + 11, get_sfr(sfr_sph));
    outHex8(buffer + 13, get_sfr(sfr_spl));
    outHex8(buffer + 19, get_sfr(sfr_iph));
    outHex8(buffer + 21, get_sfr(sfr_ipl));
    outHex8(buffer + 27, get_sfr(sfr_rp0));
    outHex8(buffer + 30, get_sfr(sfr_rp1));
    const auto flags = get_sfr(sfr_flags);
    constexpr char *flags_bits = buffer + 35;
    flags_bits[0] = bit1(flags & 0x80, 'C');
    flags_bits[1] = bit1(flags & 0x40, 'Z');
    flags_bits[2] = bit1(flags & 0x20, 'S');
    flags_bits[3] = bit1(flags & 0x10, 'V');
    flags_bits[4] = bit1(flags & 0x08, 'D');
    flags_bits[5] = bit1(flags & 0x04, 'H');
    flags_bits[6] = bit1(flags & 0x02, 'F');
    flags_bits[7] = (flags & 0x01) ? '1' : '0';  // bank address
    cli.println(buffer);
    Pins.idle();
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
    Pins.idle();
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
    Pins.idle();
}

void Regs::switchBank(RegSpace space) {
    if (space == SET_BANK1) {
        static constexpr uint8_t BANK1[] = {
                0x46, 0xD5, 0x01,  // OR FLAGS, #F_BANK
        };
        Pins.execInst(BANK1, sizeof(BANK1));
    } else {
        static constexpr uint8_t BANK0[] = {
                0x56, 0xD5, 0xFE,  // AND FLAGS, #LNOT F_BANK
        };
        Pins.execInst(BANK0, sizeof(BANK0));
    }
}

uint8_t Regs::read_reg(uint8_t addr, RegSpace space) {
    uint8_t val;
    if (space == SET_TWO && addr >= 0xC0) {
        static uint8_t READ_TWO[] = {
                0xFC, 0,     // LD r15,IM
                0xC7, 0xFF,  // LD r15,@r15
                0xD3, 0xFE,  // LDE @rr14,r15
                0xFC, 0,     // LD r15,IM
        };
        READ_TWO[1] = addr;
        READ_TWO[7] = _r[15];
        Pins.captureWrites(
                READ_TWO, sizeof(READ_TWO), nullptr, &val, sizeof(val));
        return val;
    } else {
        if (addr >= 0xE0)
            switchBank(space);
        static uint8_t READ_REG[] = {
                0xF8, 0,     // LD r15,Rn
                0xD3, 0xFE,  // LDE @rr14,r15
                0xFC, 0,     // LD r15,IM
        };
        READ_REG[1] = addr;
        READ_REG[5] = _r[15];
        Pins.captureWrites(
                READ_REG, sizeof(READ_REG), nullptr, &val, sizeof(val));
    }
    if (space == SET_ONE || space == SET_TWO)
        update_r(addr, val);
    return val;
}

void Regs::write_reg(uint8_t addr, uint8_t val) {
    static uint8_t WRITE_REG[] = {
            0xE6, 0, 0,  // LD Rn,IM
    };
    WRITE_REG[1] = addr;
    WRITE_REG[2] = val;
    Pins.execInst(WRITE_REG, sizeof(WRITE_REG));
    update_r(addr, val);
}

void Regs::reset(bool show) {
    if (show)
        Signals::resetCycles();
    constexpr auto EMT = 254;  // External Memory Timing
    write_reg(EMT, (Z88_EXTERNAL_STACK << 1));
    constexpr auto P0M = 240;  // Port 0 Mode
    write_reg(P0M, 0xFF);      // Port 0 = A15-A8
    constexpr auto PM = 241;   // Port 1 Mode
    write_reg(PM, (Z88_DATA_MEMORY << 3) | (1 << 6));
    static constexpr uint8_t JP_RESET[] = {0x8D, 0x00, 0x20};  // JP %0020
    Pins.execInst(JP_RESET, sizeof(JP_RESET));
    if (show)
        Signals::printCycles();
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

void Regs::update_r(uint8_t addr, uint8_t val) {
    const auto rp0 = get_sfr(sfr_rp0) & 0xF8;
    if ((addr & 0xF8) == rp0)
        _r[addr & 7] = val;
    const auto rp1 = get_sfr(sfr_rp1) & 0xF8;
    if ((addr & 0xF8) == rp1)
        _r[(addr & 7) + 8] = val;
}

void Regs::save_r(uint8_t num) {
    static uint8_t SAVE_R[] = {
            0xD3, 0x0E,  // LDE @rr14, rn
    };
    SAVE_R[1] = (num << 4) | 14;  // LDE @rr14,ri
    Pins.captureWrites(
            SAVE_R, sizeof(SAVE_R), nullptr, &_r[num], sizeof(_r[0]));
}

void Regs::save(bool show) {
    if (debug_cycles)
        cli.println(F("@@ save"));
    if (show)
        Signals::resetCycles();
    const auto &signals = Pins.cycle(0xFF);  // NOP
    _pc = signals.addr;

    for (uint8_t num = 0; num < sizeof(_r); num++)
        save_r(num);
    for (uint8_t num = 0; num < sizeof(_sfr); num++)
        _sfr[num] = read_reg(sfr_base + num);
    restore_r(15);

    if (show)
        Signals::printCycles();
}

void Regs::restore_r(uint8_t num) {
    static uint8_t LOAD_R[] = {
            0x0C, 0,  // LD r,IM
    };
    LOAD_R[0] = 0x0C | (num << 4);
    LOAD_R[1] = _r[num];
    Pins.execInst(LOAD_R, sizeof(LOAD_R));
}

void Regs::restore(bool show) {
    static uint8_t JP[] = {
            0x8D, 0, 0,  // JP DA
    };
    if (debug_cycles)
        cli.println(F("@@ restore"));
    if (show)
        Signals::resetCycles();

    for (uint8_t num = 0; num < sizeof(_sfr); num++)
        write_reg(sfr_base + num, _sfr[num]);
    for (uint8_t num = 0; num < sizeof(_r); num++)
        restore_r(num);

    JP[1] = hi(_pc);
    JP[2] = lo(_pc);
    Pins.execInst(JP, sizeof(JP));
    if (show)
        Signals::printCycles();
}

void Regs::set_rp0(uint8_t val) {
    set_sfr(sfr_rp0, val);
    for (auto num = 0; num < 8; num++)
        save_r(num);
}

void Regs::set_rp1(uint8_t val) {
    set_sfr(sfr_rp1, val);
    for (auto num = 8; num < 16; num++)
        save_r(num);
}

void Regs::set_sp(uint16_t val) {
    set_sfr(sfr_sph, hi(val));
    set_sfr(sfr_spl, lo(val));
}

void Regs::set_ip(uint16_t val) {
    set_sfr(sfr_iph, hi(val));
    set_sfr(sfr_ipl, lo(val));
}

void Regs::set_sfr(uint8_t name, uint8_t val) {
    const auto num = name - sfr_base;
    _sfr[num] = val;
    write_reg(name, val);
}

void Regs::set_r(uint8_t num, uint8_t val) {
    _r[num] = val;
    restore_r(num);
}

void Regs::printRegList() const {
    cli.println(F("?Reg: PC SP IP RP/0/1 FLAGS R0~R15 RR0~14"));
}

char Regs::validUint8Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("RP")) == 0)
        return 'R';
    if (strcasecmp_P(word, PSTR("RP0")) == 0)
        return 'r';
    if (strcasecmp_P(word, PSTR("RP1")) == 0)
        return 's';
    if (strcasecmp_P(word, PSTR("FLAGS")) == 0)
        return 'X';
    if (toupper(*word++) == 'R' && isdigit(*word)) {
        uint8_t num = *word++ - '0';
        if (isdigit(*word)) {
            num *= 10;
            num += *word++ - '0';
        }
        if (*word == 0 && num < 16)
            return num < 10 ? num + '0' : num - 10 + 'A';
    }
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("PC")) == 0)
        return 'P';
    if (strcasecmp_P(word, PSTR("SP")) == 0)
        return 'S';
    if (strcasecmp_P(word, PSTR("IP")) == 0)
        return 'I';
    if (toupper(*word++) == 'R' && toupper(*word++) == 'R' && isdigit(*word)) {
        uint8_t num = *word++ - '0';
        if (isdigit(*word)) {
            num *= 10;
            num += *word++ - '0';
        }
        if (*word == 0 && num < 16 && num % 2 == 0)
            return num + 'a';
    }
    return 0;
}

void Regs::setRegValue(char reg, uint32_t value) {
    switch (reg) {
    case 'P':
        _pc = value;
        break;
    case 'S':
        set_sp(value);
        break;
    case 'I':
        set_ip(value);
        break;
    case 'R':
        set_rp0(value);
        set_rp1(value + 8);
        break;
    case 'r':
        set_rp0(value);
        break;
    case 's':
        set_rp1(value);
        break;
    case 'X':
        set_flags(value);
        break;
    default:
        if (islower(reg)) {
            const auto num = reg - 'a';
            set_rr(num, value);
        } else if (isxdigit(reg)) {
            const auto num = isdigit(reg) ? reg - '0' : reg - 'A' + 10;
            set_r(num, value);
        }
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
    disassembler.setOption("uppercase", "true");
    uint16_t num = 0;
    while (num < numInsn) {
        char operands[20];
        libasm::Insn insn(addr);
        Memory.setAddress(addr);
        disassembler.decode(Memory, insn, operands, sizeof(operands));
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
    if (space == nullptr) {
        return memory[addr];
    } else {
        auto spc = SET_ONE;
        if (strcasecmp_P(space, PSTR("set2")) == 0)
            spc = SET_TWO;
        if (strcasecmp_P(space, PSTR("bank1")) == 0)
            spc = SET_BANK1;
        return Regs.read_reg(addr, spc);
    }
}

void Memory::write(uint16_t addr, uint8_t data, const char *space) {
    if (space == nullptr) {
        memory[addr] = data;
    } else {
        Regs.write_reg(addr, data);
    }
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
