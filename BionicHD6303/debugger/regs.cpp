#include "regs.h"

#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;

struct Regs Regs;

static constexpr uint8_t cycles_table[256] = {
#if defined(BIONIC_HD6303)
// clang-format off
        0,   // 00:0 -    -
        1,   // 01:1 NOP  INH
        0,   // 02:0 -    -
        0,   // 03:0 -    -
        1,   // 04:1 LSRD INH
        1,   // 05:1 ASLD INH
        1,   // 06:1 TAP  INH
        1,   // 07:1 TPA  INH
        1,   // 08:1 INX  INH
        1,   // 09:1 DEX  INH
        1,   // 0A:1 CLV  INH
        1,   // 0B:1 SEV  INH
        1,   // 0C:1 CLC  INH
        1,   // 0D:1 SEC  INH
        1,   // 0E:1 CLI  INH
        1,   // 0F:1 SEI  INH
        1,   // 10:1 SBA  INH
        1,   // 11:1 CBA  INH
        0,   // 12:0 -    -
        0,   // 13:0 -    -
        0,   // 14:0 -    -
        0,   // 15:0 -    -
        1,   // 16:1 TAB  INH
        1,   // 17:1 T
   0x80|3,   // 18:1 X
   0x80|3,   // 19:1 DAA  INH
        2,   // 1A:1 SLP  INH
        1,   // 1B:1 ABA  INH
        0,   // 1C:0 -    -
        0,   // 1D:0 -    -
        0,   // 1E:0 -    -
        0,   // 1F:0 -    -
        3,   // 20:2 BRA  REL
        3,   // 21:2 BRN  REL
        3,   // 22:2 BHI  REL
        3,   // 23:2 BLS  REL
        3,   // 24:2 BCC  REL
        3,   // 25:2 BCS  REL
        3,   // 26:2 BNE  REL
        3,   // 27:2 BEQ  REL
        3,   // 28:2 BVC  REL
        3,   // 29:2 BVS  REL
        3,   // 2A:2 BPL  REL
        3,   // 2B:2 BMI  REL
        3,   // 2C:2 BGE  REL
        3,   // 2D:2 BLT  REL
        3,   // 2E:2 BGT  REL
        3,   // 2F:2 BLE  REL
        1,   // 30:1 TSX  INH
        1,   // 31:1 I
   0x80|4,   // 32:1 P
   0x80|4,   // 33:1 PULB INH
        1,   // 34:1 DES  INH
        1,   // 35:1 TXS  INH
        4,   // 36:1 PSHA INH
        4,   // 37:1 P
   0x80|5,   // 38:1 PULX INH
        5,   // 39:1 RTS  INH
        1,   // 3A:1 ABX  INH
        10,  // 3B:1 RTI  INH
        5,   // 3C:1 P
   0x80|8,   // 3D:1 M
   0x80|10,  // 3E:1 WAI  INH
        12,  // 3F:1 SWI  INH
        1,   // 40:1 NEGA INH
        0,   // 41:0 -    -
        0,   // 42:0 -    -
        1,   // 43:1 COMA INH
        1,   // 44:1 LSRA INH
        0,   // 45:0 -    -
        1,   // 46:1 RORA INH
        1,   // 47:1 ASRA INH
        1,   // 48:1 ASLA INH
        1,   // 49:1 ROLA INH
        1,   // 4A:1 DECA INH
        0,   // 4B:0 -    -
        1,   // 4C:1 INCA INH
        1,   // 4D:1 TSTA INH
        0,   // 4E:0 -    -
        1,   // 4F:1 CLRA INH
        1,   // 50:1 NEGB INH
        0,   // 51:0 -    -
        0,   // 52:0 -    -
        1,   // 53:1 COMB INH
        1,   // 54:1 LSRB INH
        0,   // 55:0 -    -
        1,   // 56:1 RORB INH
        1,   // 57:1 ASRB INH
        1,   // 58:1 ASLB INH
        1,   // 59:1 ROLB INH
        1,   // 5A:1 DECB INH
        0,   // 5B:0 -    -
        1,   // 5C:1 INCB INH
        1,   // 5D:1 TSTB INH
        0,   // 5E:0 -    -
        1,   // 5F:1 CLRB INH
        6,   // 60:2 NEG  IDX
        7,   // 61:2 AIM  IDX
        7,   // 62:2 OIM  IDX
        6,   // 63:2 COM  IDX
        6,   // 64:2 LSR  IDX
        7,   // 65:2 EIM  IDX
        6,   // 66:2 ROR  IDX
        6,   // 67:2 ASR  IDX
        6,   // 68:2 ASL  IDX
        6,   // 69:2 ROL  IDX
        6,   // 6A:2 DEC  IDX
        5,   // 6B:2 TIM  IDX
        6,   // 6C:2 INC  IDX
        6,   // 6D:2 TST  IDX
        4,   // 6E:2 JMP  IDX
        5,   // 6F:2 CLR  IDX
        6,   // 70:3 NEG  EXT
        6,   // 71:2 AIM  DIR
        6,   // 72:2 OIM  DIR
        6,   // 73:3 COM  EXT
        6,   // 74:3 LSR  EXT
        6,   // 75:2 EIM  DIR
        6,   // 76:3 ROR  EXT
        6,   // 77:3 ASR  EXT
        6,   // 78:3 ASL  EXT
        6,   // 79:3 ROL  EXT
        6,   // 7A:3 DEC  EXT
        4,   // 7B:2 TIM  DIR
        6,   // 7C:3 INC  EXT
        6,   // 7D:3 TST  EXT
        3,   // 7E:3 JMP  EXT
        6,   // 7F:3 CLR  EXT
        2,   // 80:2 SUBA IMM
        2,   // 81:2 CMPA IMM
        2,   // 82:2 SBCA IMM
        3,   // 83:3 SUBD IMM
        2,   // 84:2 ANDA IMM
        2,   // 85:2 BITA IMM
        2,   // 86:2 LDAA IMM
        0,   // 87:0 -    -
        2,   // 88:2 EORA IMM
        2,   // 89:2 ADCA IMM
        2,   // 8A:2 ORAA IMM
        2,   // 8B:2 ADDA IMM
        3,   // 8C:3 CPX  IMM
        5,   // 8D:2 BSR  REL
        3,   // 8E:3 LDS  IMM
        0,   // 8F:0 -    -
        3,   // 90:2 SUBA DIR
        3,   // 91:2 CMPA DIR
        3,   // 92:2 SBCA DIR
        4,   // 93:2 SUBD DIR
        3,   // 94:2 ANDA DIR
        3,   // 95:2 BITA DIR
        3,   // 96:2 LDAA DIR
        3,   // 97:2 STAA DIR
        3,   // 98:2 EORA DIR
        3,   // 99:2 ADCA DIR
        3,   // 9A:2 ORAA DIR
        3,   // 9B:2 ADDA DIR
        4,   // 9C:2 CPX  DIR
        5,   // 9D:2 JSR  DIR
        4,   // 9E:2 LDS  DIR
        4,   // 9F:2 STS  DIR
        4,   // A0:2 SUBA IDX
        4,   // A1:2 CMPA IDX
        4,   // A2:2 SBCA IDX
        5,   // A3:2 SUBD IDX
        4,   // A4:2 ANDA IDX
        4,   // A5:2 BITA IDX
        4,   // A6:2 LDAA IDX
        4,   // A7:2 STAA IDX
        4,   // A8:2 EORA IDX
        4,   // A9:2 ADCA IDX
        4,   // AA:2 ORAA IDX
        4,   // AB:2 ADDA IDX
        5,   // AC:2 CPX  IDX
        5,   // AD:2 JSR  IDX
        5,   // AE:2 LDS  IDX
        5,   // AF:2 STS  IDX
        4,   // B0:3 SUBA EXT
        4,   // B1:3 CMPA EXT
        4,   // B2:3 SBCA EXT
        5,   // B3:3 SUBD EXT
        4,   // B4:3 ANDA EXT
        4,   // B5:3 BITA EXT
        4,   // B6:3 LDAA EXT
        4,   // B7:3 STAA EXT
        4,   // B8:3 EORA EXT
        4,   // B9:3 ADCA EXT
        4,   // BA:3 ORAA EXT
        4,   // BB:3 ADDA EXT
        6,   // BC:3 CPX  EXT
        6,   // BD:3 JSR  EXT
        5,   // BE:3 LDS  EXT
        5,   // BF:3 STS  EXT
        2,   // C0:2 SUBB IMM
        2,   // C1:2 CMPB IMM
        2,   // C2:2 SBCB IMM
        3,   // C3:3 ADDD IMM
        2,   // C4:2 ANDB IMM
        2,   // C5:2 BITB IMM
        2,   // C6:2 LDAB IMM
        0,   // C7:0 -    -
        2,   // C8:2 EORB IMM
        2,   // C9:2 ADCB IMM
        2,   // CA:2 ORAB IMM
        2,   // CB:2 ADDB IMM
        3,   // CC:3 LDD  IMM
        0,   // CD:0 -    -
        3,   // CE:3 LDX  IMM
        0,   // CF:0 -    -
        3,   // D0:2 SUBB DIR
        3,   // D1:2 CMPB DIR
        3,   // D2:2 SBCB DIR
        4,   // D3:2 ADDD DIR
        3,   // D4:2 ANDB DIR
        3,   // D5:2 BITB DIR
        3,   // D6:2 LDAB DIR
        3,   // D7:2 STAB DIR
        3,   // D8:2 EORB DIR
        3,   // D9:2 ADCB DIR
        3,   // DA:2 ORAB DIR
        3,   // DB:2 ADDB DIR
        4,   // DC:2 LDD  DIR
        4,   // DD:2 STD  DIR
        4,   // DE:2 LDX  DIR
        4,   // DF:2 STX  DIR
        4,   // E0:2 SUBB IDX
        4,   // E1:2 CMPB IDX
        4,   // E2:2 SBCB IDX
        5,   // E3:2 ADDD IDX
        4,   // E4:2 ANDB IDX
        4,   // E5:2 BITB IDX
        4,   // E6:2 LDAB IDX
        4,   // E7:2 STAB IDX
        4,   // E8:2 EORB IDX
        4,   // E9:2 ADCB IDX
        4,   // EA:2 ORAB IDX
        4,   // EB:2 ADDB IDX
        5,   // EC:2 LDD  IDX
        5,   // ED:2 STD  IDX
        5,   // EE:2 LDX  IDX
        5,   // EF:2 STX  IDX
        4,   // F0:3 SUBB EXT
        4,   // F1:3 CMPB EXT
        4,   // F2:3 SBCB EXT
        5,   // F3:3 ADDD EXT
        4,   // F4:3 ANDB EXT
        4,   // F5:3 BITB EXT
        4,   // F6:3 LDAB EXT
        4,   // F7:3 STAB EXT
        4,   // F8:3 EORB EXT
        4,   // F9:3 ADCB EXT
        4,   // FA:3 ORAB EXT
        4,   // FB:3 ADDB EXT
        5,   // FC:3 LDD  EXT
        5,   // FD:3 STD  EXT
        5,   // FE:3 LDX  EXT
        5,   // FF:3 STX  EXT
// clang-format on
#else
        0,   // 00:0 -    -
        2,   // 01:1 NOP  INH
        0,   // 02:0 -    -
        0,   // 03:0 -    -
        3,   // 04:1 LSRD INH
        3,   // 05:1 ASLD INH
        2,   // 06:1 TAP  INH
        2,   // 07:1 TPA  INH
        3,   // 08:1 INX  INH
        3,   // 09:1 DEX  INH
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
        2,   // 17:1 TAB  INH
        0,   // 18:0 -    -
        2,   // 19:1 DAA  INH
        0,   // 1A:0 -    -
        2,   // 1B:1 ABA  INH
        0,   // 1C:0 -    -
        0,   // 1D:0 -    -
        0,   // 1E:0 -    -
        0,   // 1F:0 -    -
        3,   // 20:2 BRA  REL
        3,   // 21:2 BRN  REL
        3,   // 22:2 BHI  REL
        3,   // 23:2 BLS  REL
        3,   // 24:2 BCC  REL
        3,   // 25:2 BCS  REL
        3,   // 26:2 BNE  REL
        3,   // 27:2 BEQ  REL
        3,   // 28:2 BVC  REL
        3,   // 29:2 BVS  REL
        3,   // 2A:2 BPL  REL
        3,   // 2B:2 BMI  REL
        3,   // 2C:2 BGE  REL
        3,   // 2D:2 BLT  REL
        3,   // 2E:2 BGT  REL
        3,   // 2F:2 BLE  REL
        3,   // 30:1 TSX  INH
        3,   // 31:1 INS  INH
        4,   // 32:1 PULA INH
        4,   // 33:1 PULB INH
        3,   // 34:1 DES  INH
        3,   // 35:1 TXS  INH
        3,   // 36:1 PSHA INH
        3,   // 37:1 PSHB INH
        5,   // 38:1 PULX INH
        5,   // 39:1 RTS  INH
        3,   // 3A:1 ABX  INH
        10,  // 3B:1 RTI  INH
        4,   // 3C:1 PSHX INH
        10,  // 3D:1 MUL  INH
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
        6,   // 60:2 NEG  IDX
        0,   // 61:0 -    -
        0,   // 62:0 -    -
        6,   // 63:2 COM  IDX
        6,   // 64:2 LSR  IDX
        0,   // 65:0 -    -
        6,   // 66:2 ROR  IDX
        6,   // 67:2 ASR  IDX
        6,   // 68:2 ASL  IDX
        6,   // 69:2 ROL  IDX
        6,   // 6A:2 DEC  IDX
        0,   // 6B:0 -    -
        6,   // 6C:2 INC  IDX
        6,   // 6D:2 TST  IDX
        3,   // 6E:2 JMP  IDX
        6,   // 6F:2 CLR  IDX
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
        4,   // 83:3 SUBD IMM
        2,   // 84:2 ANDA IMM
        2,   // 85:2 BITA IMM
        2,   // 86:2 LDAA IMM
        0,   // 87:0 -    -
        2,   // 88:2 EORA IMM
        2,   // 89:2 ADCA IMM
        2,   // 8A:2 ORAA IMM
        2,   // 8B:2 ADDA IMM
        4,   // 8C:3 CPX  IMM
        6,   // 8D:2 BSR  REL
        3,   // 8E:3 LDS  IMM
        0,   // 8F:0 -    -
        3,   // 90:2 SUBA DIR
        3,   // 91:2 CMPA DIR
        3,   // 92:2 SBCA DIR
        5,   // 93:2 SUBD DIR
        3,   // 94:2 ANDA DIR
        3,   // 95:2 BITA DIR
        3,   // 96:2 LDAA DIR
        3,   // 97:2 STAA DIR
        3,   // 98:2 EORA DIR
        3,   // 99:2 ADCA DIR
        3,   // 9A:2 ORAA DIR
        3,   // 9B:2 ADDA DIR
        5,   // 9C:2 CPX  DIR
        5,   // 9D:2 JSR  DIR
        4,   // 9E:2 LDS  DIR
        4,   // 9F:2 STS  DIR
        4,   // A0:2 SUBA IDX
        4,   // A1:2 CMPA IDX
        4,   // A2:2 SBCA IDX
        6,   // A3:2 SUBD IDX
        4,   // A4:2 ANDA IDX
        4,   // A5:2 BITA IDX
        4,   // A6:2 LDAA IDX
        4,   // A7:2 STAA IDX
        4,   // A8:2 EORA IDX
        4,   // A9:2 ADCA IDX
        4,   // AA:2 ORAA IDX
        4,   // AB:2 ADCA IDX
        6,   // AC:2 CPX  IDX
        6,   // AD:2 JSR  IDX
        5,   // AE:2 LDS  IDX
        5,   // AF:2 STS  IDX
        4,   // B0:3 SUBA EXT
        4,   // B1:3 CMPA EXT
        4,   // B2:3 SBCA EXT
        6,   // B3:3 SUBD EXT
        4,   // B4:3 ANDA EXT
        4,   // B5:3 BITA EXT
        4,   // B6:3 LDAA EXT
        4,   // B7:3 STAA EXT
        4,   // B8:3 EORA EXT
        4,   // B9:3 ADCA EXT
        4,   // BA:3 ORAA EXT
        4,   // BB:3 ADDA EXT
        6,   // BC:3 CPX  EXT
        6,   // BD:3 JSR  EXT
        5,   // BE:3 LDS  EXT
        5,   // BF:3 STS  EXT
        2,   // C0:2 SUBB IMM
        2,   // C1:2 CMPB IMM
        2,   // C2:2 SBCB IMM
        4,   // C3:3 ADDD IMM
        2,   // C4:2 ANDB IMM
        2,   // C5:2 BITB IMM
        2,   // C6:2 LDAB IMM
        0,   // C7:0 -    -
        2,   // C8:2 EORB IMM
        2,   // C9:2 ADCB IMM
        2,   // CA:2 ORAB IMM
        2,   // CB:2 ADDB IMM
        3,   // CC:3 LDD  IMM
        0,   // CD:0 -    -
        3,   // CE:3 LDX  IMM
        0,   // CF:0 -    -
        3,   // D0:2 SUBB DIR
        3,   // D1:2 CMPB DIR
        3,   // D2:2 SBCB DIR
        5,   // D3:2 ADDD DIR
        3,   // D4:2 ANDB DIR
        3,   // D5:2 BITB DIR
        3,   // D6:2 LDAB DIR
        3,   // D7:2 STAB DIR
        3,   // D8:2 EORB DIR
        3,   // D9:2 ADCB DIR
        3,   // DA:2 ORAB DIR
        3,   // DB:2 ADDB DIR
        4,   // DC:2 LDD  DIR
        4,   // DD:2 STD  DIR
        4,   // DE:2 LDX  DIR
        4,   // DF:2 STX  DIR
        4,   // E0:2 SUBB IDX
        4,   // E1:2 CMPB IDX
        4,   // E2:2 SBCB IDX
        6,   // E3:2 ADDD IDX
        4,   // E4:2 ANDB IDX
        4,   // E5:2 BITB IDX
        4,   // E6:2 LDAB IDX
        4,   // E7:2 STAB IDX
        4,   // E8:2 EORB IDX
        4,   // E9:2 ADCB IDX
        4,   // EA:2 ORAB IDX
        4,   // EB:2 ADCB IDX
        5,   // EC:2 LDD  IDX
        5,   // ED:2 STD  IDX
        5,   // EE:2 LDX  IDX
        5,   // EF:2 STX  IDX
        4,   // F0:3 SUBB EXT
        4,   // F1:3 CMPB EXT
        4,   // F2:3 SBCB EXT
        6,   // F3:3 ADDD EXT
        4,   // F4:3 ANDB EXT
        4,   // F5:3 BITB EXT
        4,   // F6:3 LDAB EXT
        4,   // F7:3 STAB EXT
        4,   // F8:3 EORB EXT
        4,   // F9:3 ADCB EXT
        4,   // FA:3 ORAB EXT
        4,   // FB:3 ADDB EXT
        5,   // FC:3 LDD  EXT
        5,   // FD:3 STD  EXT
        5,   // FE:3 LDX  EXT
        5,   // FF:3 STX  EXT
#endif
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
#if defined(BIONIC_HD6303)
    return "6301";
#else
    return "6801";
#endif
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

void Regs::save(bool show, bool undoPrefetch) {
    static const uint8_t SWI[3] = {0x3F, 0xFF, 0xFF};  // SWI

    uint8_t bytes[10];
    Signals::resetCycles();
#if defined(BIONIC_HD6303)
    const uint8_t SWI_cycles = 3;
#else
    const uint8_t SWI_cycles = 2;
#endif
    Pins.captureWrites(SWI, SWI_cycles, &sp, bytes, 7);
    // Capturing writes to stack in little endian order.
    pc = le16(bytes) - 1;  //  offset SWI instruction.
    if (undoPrefetch)
        pc--;
    // Injecting PC as SWI vector (with irrelevant read ahead).
#if defined(BIONIC_HD6303)
    bytes[7] = hi(pc);
    bytes[8] = lo(pc);
    Pins.execInst(bytes + 7, 2);
#else
    bytes[7] = 0;
    bytes[8] = hi(pc);
    bytes[9] = lo(pc);
    Pins.execInst(bytes + 7, 3);
#endif
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
    cli.println(F("?Reg: pc sp x a b d cc"));
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
    if (reg == 'p' || reg == 's' || reg == 'x' || reg == 'd') {
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
    case 'd':
        setD(value);
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
