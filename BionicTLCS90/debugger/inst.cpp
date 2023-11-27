#include "inst.h"

#include <string.h>

#include "regs.h"

#include <libcli.h>
extern libcli::Cli cli;

#define LOG_GET(e)
//#define LOG_GET(e) e
#define LOG_MATCH(e)
//#define LOG_MATCH(e) e

namespace {

/**
 * Legend for BUS_SEQUENCE
 *  1: new instruction address
 *  2-9: part of instruction
 *  0: repeat current instruction
 *  N: prefetch next instruction
 *  n: dummy prefetch next instruction
 *  B: 8-bit relative branch
 *  P: popped from stack address
 *  R: read 1 byte
 *  W: write 1 byte, the same address if R is preceeded
 *  r: read 1 byte at address R+1
 *  w: write 1 byte at address W+1
 *  E: read 1 byte from direct page (FFxx)
 *  e: read 1 byte at address E+1
 *  F: write 1 byte to direct page (FFxx), the same address if E is preseeded
 *  f: write 1 byte at address F+1
 *  S: read 1 byte at R+2
 *  s: read 1 byte at R+3
 *  X: write 1 byte at W+2
 *  x: write 1 byte at W+3
 *  @: separator between true and false conditions, the next entry is for false
 *     condition
 */
constexpr const char *const BUS_SEQUENCE[/*seq*/] = {
        "",         // 0
        "N",        // 1
        "1",        // 2
        "wW1",      // 3
        "nRr1",     // 4
        "nRrSs1",   // 5
        "EN",       // 6
        "NF",       // 7
        "FN",       // 8
        "NFf",      // 9
        "EeN",      // 10
        "NwW",      // 11
        "nRrN",     // 12
        "ENF",      // 13
        "EeNFf",    // 14
        "nB",       // 15
        "n1xXwW1",  // 16
        "nN",       // 17
        "nRWN",     // 18
        "nRN",      // 19
        "RNW",      // 20
        "RN",       // 21
        "RrN",      // 22
        "RrWwN",    // 23
        "RrNWw",    // 24
        "NW",       // 25
        "WN",       // 26
        "NWw",      // 27
        "n1",       // 28
        "nwW1",     // 29
        "ENF@",     // 30
        "N",        // 31
        "B@",       // 32
        "N",        // 33
        "nB@",      // 34
        "N",        // 35
        "nRW0@",    // 36
        "nRWN",     // 37
        "nR0@",     // 38
        "nRN",      // 39
        "nRrP@",    // 40
        "N",        // 41
        "n1@",      // 42
        "N",        // 43
        "nwW1@",    // 44
        "N",        // 45
};

constexpr const uint8_t BUS_CYCLES[/*seq*/] = {
        0,  //
        1,  // N
        1,  // 1
        3,  // wW1
        4,  // nRr1
        6,  // nRrSs1
        2,  // EN
        2,  // NF
        2,  // FN
        3,  // NFf
        3,  // EeN
        3,  // NwW
        4,  // nRrN
        3,  // ENF
        5,  // EeNFf
        2,  // nB
        7,  // n1xXwW1
        2,  // nN
        4,  // nRWN
        3,  // nRN
        3,  // RNW
        2,  // RN
        3,  // RrN
        5,  // RrWwN
        5,  // RrNWw
        2,  // NW
        2,  // WN
        3,  // NWw
        2,  // n1
        4,  // nwW1
        3,  // ENF@
        1,  // N
        1,  // B@
        1,  // N
        2,  // nB@
        1,  // N
        4,  // nRW0@
        4,  // nRWN
        3,  // nR0@
        3,  // nRN
        4,  // nRrP@
        1,  // N
        2,  // n1@
        1,  // N
        4,  // nwW1@
        1,  // N
};

constexpr const uint8_t PREFETCH_CYCLES[/*seq*/] = {
        0,  //
        0,  // N
        0,  // 1
        2,  // wW1
        3,  // nRr1
        5,  // nRrSs1
        1,  // EN
        0,  // NF
        1,  // FN
        0,  // NFf
        2,  // EeN
        0,  // NwW
        3,  // nRrN
        1,  // ENF
        2,  // EeNFf
        1,  // nB
        6,  // n1xXwW1
        1,  // nN
        3,  // nRWN
        2,  // nRN
        1,  // RNW
        1,  // RN
        2,  // RrN
        4,  // RrWwN
        2,  // RrNWw
        0,  // NW
        1,  // WN
        0,  // NWw
        1,  // n1
        3,  // nwW1
        1,  // ENF@
        0,  // N
        0,  // B@
        0,  // N
        1,  // nB@
        0,  // N
        3,  // nRW0@
        3,  // nRWN
        2,  // nR0@
        2,  // nRN
        3,  // nRrP@
        0,  // N
        1,  // n1@
        0,  // N
        3,  // nwW1@
        0,  // N
};

constexpr uint8_t inst_len(uint8_t e) {
    return e == sizeof(BUS_SEQUENCE) ? 4 : (e >> 6);
}

constexpr uint8_t inst_sequence(uint8_t e) {
    return e == sizeof(BUS_SEQUENCE) ? 9 : (e & 0x3F);
}

constexpr uint8_t prefetch_cycle(uint8_t seq) {
    return PREFETCH_CYCLES[seq];
}

constexpr uint8_t bus_cycles(uint8_t seq) {
    return BUS_CYCLES[seq];
}

constexpr uint8_t E(uint8_t len, uint8_t seq) {
    // len: 0~4, seq: 0~45
    return len >= 4 ? sizeof(BUS_SEQUENCE) : ((len << 6) | seq);
}

constexpr uint8_t PAGE0[] = {
        E(1, 1),   // 00: NOP  -      ; N                   -
        E(1, 1),   // 01: HALT -      ; N:d                 -
        E(1, 1),   // 02: DI   -      ; N                   -
        E(1, 1),   // 03: EI   -      ; N                   -
        E(0, 0),   // 04: -    -      ; -                   -
        E(0, 0),   // 05: -    -      ; -                   -
        E(0, 0),   // 06: -    -      ; -                   -
        E(2, 30),  // 07: INCX (n)    ; 2:d:E:N:F           2:d:N
        E(1, 1),   // 08: EX   DE,HL  ; N                   -
        E(1, 1),   // 09: EX   AF,AF' ; N                   -
        E(1, 1),   // 0A: EXX  -      ; N                   -
        E(1, 1),   // 0B: DAA  A      ; N:d                 -
        E(1, 1),   // 0C: RCF  -      ; N                   -
        E(1, 1),   // 0D: SCF  -      ; N                   -
        E(1, 1),   // 0E: CCF  -      ; N                   -
        E(2, 30),  // 0F: DECX (n)    ; 2:d:E:N:F           2:d:N
        E(1, 1),   // 10: CPL  A      ; N                   -
        E(1, 1),   // 11: NEG  A      ; N                   -
        E(2, 1),   // 12: MUL  HL,n   ; 2:d:d:d:d:d:d:N     -
        E(2, 1),   // 13: DIV  HL,n   ; 2:d:d:d:d:d:d:N     -
        E(3, 1),   // 14: ADD  IX,mn  ; 2:3:N               -
        E(3, 1),   // 15: ADD  IY,mn  ; 2:3:N               -
        E(3, 1),   // 16: ADD  SP,mn  ; 2:3:N               -
        E(3, 1),   // 17: LDAR HL,cd  ; 2:3:N:d             -
        E(2, 32),  // 18: DJNZ d      ; 2:d:d:d:B           2:d:d:d:N
        E(2, 32),  // 19: DJNZ BC,d   ; 2:d:d:d:B           2:d:d:d:N
        E(3, 2),   // 1A: JP   mn     ; 2:3:d:1             -
        E(3, 2),   // 1B: JRL  cd     ; 2:3:d:d:1           -
        E(3, 3),   // 1C: CALL mn     ; 2:3:d:d:w:W:1       -
        E(3, 3),   // 1D: CALR cd     ; 2:3:d:d:d:w:W:1     -
        E(1, 4),   // 1E: RET  -      ; n:R:r:d:1           -
        E(1, 5),   // 1F: RETI -      ; n:R:r:S:s:d:1       -
        E(1, 1),   // 20: LD   A,B    ; N                   -
        E(1, 1),   // 21: LD   A,C    ; N                   -
        E(1, 1),   // 22: LD   A,D    ; N                   -
        E(1, 1),   // 23: LD   A,E    ; N                   -
        E(1, 1),   // 24: LD   A,H    ; N                   -
        E(1, 1),   // 25: LD   A,L    ; N                   -
        E(1, 1),   // 26: LD   A,A    ; N                   -
        E(2, 6),   // 27: LD   A,(n)  ; 2:d:E:N             -
        E(1, 1),   // 28: LD   B,A    ; N                   -
        E(1, 1),   // 29: LD   C,A    ; N                   -
        E(1, 1),   // 2A: LD   D,A    ; N                   -
        E(1, 1),   // 2B: LD   E,A    ; N                   -
        E(1, 1),   // 2C: LD   H,A    ; N                   -
        E(1, 1),   // 2D: LD   L,A    ; N                   -
        E(1, 1),   // 2E: LD   A,A    ; N                   -
        E(2, 7),   // 2F: LD   (n),A  ; 2:d:N:F             -
        E(2, 1),   // 30: LD   B,n    ; 2:N                 -
        E(2, 1),   // 31: LD   C,n    ; 2:N                 -
        E(2, 1),   // 32: LD   D,n    ; 2:N                 -
        E(2, 1),   // 33: LD   E,n    ; 2:N                 -
        E(2, 1),   // 34: LD   H,n    ; 2:N                 -
        E(2, 1),   // 35: LD   L,n    ; 2:N                 -
        E(2, 1),   // 36: LD   A,n    ; 2:N                 -
        E(3, 8),   // 37: LD   (w),n  ; 2:d:3:F:N           -
        E(3, 1),   // 38: LD   BC,mn  ; 2:3:N               -
        E(3, 1),   // 39: LD   DE,mn  ; 2:3:N               -
        E(3, 1),   // 3A: LD   HL,mn  ; 2:3:N               -
        E(0, 0),   // 3B: -    -      ; -                   -
        E(3, 1),   // 3C: LD   IX,mn  ; 2:3:N               -
        E(3, 1),   // 3D: LD   IY,mn  ; 2:3:N               -
        E(3, 1),   // 3E: LD   SP,mn  ; 2:3:N               -
        E(4, 9),   // 3F: LDW  (w),mn ; 2:d:3:4:N:F:f       -
        E(1, 1),   // 40: LD   HL,BC  ; N:d                 -
        E(1, 1),   // 41: LD   HL,DE  ; N:d                 -
        E(1, 1),   // 42: LD   HL,HL  ; N:d                 -
        E(0, 0),   // 43: -    -      ; -                   -
        E(1, 1),   // 44: LD   HL,IX  ; N:d                 -
        E(1, 1),   // 45: LD   HL,IY  ; N:d                 -
        E(1, 1),   // 46: LD   HL,SP  ; N:d                 -
        E(2, 10),  // 47: LD   HL,(n) ; 2:d:E:e:N           -
        E(1, 1),   // 48: LD   BC,HL  ; N:d                 -
        E(1, 1),   // 49: LD   DE,HL  ; N:d                 -
        E(1, 1),   // 4A: LD   HL,HL  ; N:d                 -
        E(0, 0),   // 4B: -    -      ; -                   -
        E(1, 1),   // 4C: LD   IX,HL  ; N:d                 -
        E(1, 1),   // 4D: LD   IY,HL  ; N:d                 -
        E(1, 1),   // 4E: LD   SP,HL  ; N:d                 -
        E(2, 9),   // 4F: LD   (n),HL ; 2:d:N:F:f           -
        E(1, 11),  // 50: PUSH BC     ; N:d:w:W             -
        E(1, 11),  // 51: PUSH DE     ; N:d:w:W             -
        E(1, 11),  // 52: PUSH HL     ; N:d:w:W             -
        E(0, 0),   // 53: -    -      ; -                   -
        E(1, 11),  // 54: PUSH IX     ; N:d:w:W             -
        E(1, 11),  // 55: PUSH IY     ; N:d:w:W             -
        E(1, 11),  // 56: PUSH AF     ; N:d:w:W             -
        E(0, 0),   // 57: -    -      ; -                   -
        E(1, 12),  // 58: POP  BC     ; n:R:r:d:N           -
        E(1, 12),  // 59: POP  DE     ; n:R:r:d:N           -
        E(1, 12),  // 5A: POP  HL     ; n:R:r:d:N           -
        E(0, 0),   // 5B: -    -      ; -                   -
        E(1, 12),  // 5C: POP  IX     ; n:R:r:d:N           -
        E(1, 12),  // 5D: POP  IY     ; n:R:r:d:N           -
        E(1, 12),  // 5E: POP  AF     ; n:R:r:d:N           -
        E(0, 0),   // 5F: -    -      ; -                   -
        E(2, 6),   // 60: ADD  A,(n)  ; 2:d:E:N             -
        E(2, 6),   // 61: ADC  A,(n)  ; 2:d:E:N             -
        E(2, 6),   // 62: SUB  A,(n)  ; 2:d:E:N             -
        E(2, 6),   // 63: SBC  A,(n)  ; 2:d:E:N             -
        E(2, 6),   // 64: AND  A,(n)  ; 2:d:E:N             -
        E(2, 6),   // 65: XOR  A,(n)  ; 2:d:E:N             -
        E(2, 6),   // 66: OR   A,(n)  ; 2:d:E:N             -
        E(2, 6),   // 67: CP   A,(n)  ; 2:d:E:N             -
        E(2, 1),   // 68: ADD  A,n    ; 2:N                 -
        E(2, 1),   // 69: ADC  A,n    ; 2:N                 -
        E(2, 1),   // 6A: SUB  A,n    ; 2:N                 -
        E(2, 1),   // 6B: SBC  A,n    ; 2:N                 -
        E(2, 1),   // 6C: AND  A,n    ; 2:N                 -
        E(2, 1),   // 6D: XOR  A,n    ; 2:N                 -
        E(2, 1),   // 6E: OR   A,n    ; 2:N                 -
        E(2, 1),   // 6F: CP   A,n    ; 2:N                 -
        E(2, 10),  // 70: ADD  HL,(n) ; 2:d:E:e:N           -
        E(2, 10),  // 71: ADC  HL,(n) ; 2:d:E:e:N           -
        E(2, 10),  // 72: SUB  HL,(n) ; 2:d:E:e:N           -
        E(2, 10),  // 73: SBC  HL,(n) ; 2:d:E:e:N           -
        E(2, 10),  // 74: AND  HL,(n) ; 2:d:E:e:N           -
        E(2, 10),  // 75: XOR  HL,(n) ; 2:d:E:e:N           -
        E(2, 10),  // 76: OR   HL,(n) ; 2:d:E:e:N           -
        E(2, 10),  // 77: CP   HL,(n) ; 2:d:E:e:N           -
        E(3, 1),   // 78: ADD  HL,mn  ; 2:3:N               -
        E(3, 1),   // 79: ADC  HL,mn  ; 2:3:N               -
        E(3, 1),   // 7A: SUB  HL,mn  ; 2:3:N               -
        E(3, 1),   // 7B: SBC  HL,mn  ; 2:3:N               -
        E(3, 1),   // 7C: AND  HL,mn  ; 2:3:N               -
        E(3, 1),   // 7D: XOR  HL,mn  ; 2:3:N               -
        E(3, 1),   // 7E: OR   HL,mn  ; 2:3:N               -
        E(3, 1),   // 7F: CP   HL,mn  ; 2:3:N               -
        E(1, 1),   // 80: INC  B      ; N                   -
        E(1, 1),   // 81: INC  C      ; N                   -
        E(1, 1),   // 82: INC  D      ; N                   -
        E(1, 1),   // 83: INC  E      ; N                   -
        E(1, 1),   // 84: INC  H      ; N                   -
        E(1, 1),   // 85: INC  L      ; N                   -
        E(1, 1),   // 86: INC  A      ; N                   -
        E(2, 13),  // 87: INC  (n)    ; 2:d:E:N:F           -
        E(1, 1),   // 88: DEC  B      ; N                   -
        E(1, 1),   // 89: DEC  C      ; N                   -
        E(1, 1),   // 8A: DEC  D      ; N                   -
        E(1, 1),   // 8B: DEC  E      ; N                   -
        E(1, 1),   // 8C: DEC  H      ; N                   -
        E(1, 1),   // 8D: DEC  L      ; N                   -
        E(1, 1),   // 8E: DEC  A      ; N                   -
        E(2, 13),  // 8F: DEC  (n)    ; 2:d:E:N:F           -
        E(1, 1),   // 90: INC  BC     ; N:d                 -
        E(1, 1),   // 91: INC  DE     ; N:d                 -
        E(1, 1),   // 92: INC  HL     ; N:d                 -
        E(0, 0),   // 93: -    -      ; -                   -
        E(1, 1),   // 94: INC  IX     ; N:d                 -
        E(1, 1),   // 95: INC  IY     ; N:d                 -
        E(1, 1),   // 96: INC  SP     ; N:d                 -
        E(2, 14),  // 97: INCW (n)    ; 2:d:E:e:N:F:f       -
        E(1, 1),   // 98: DEC  BC     ; N:d                 -
        E(1, 1),   // 99: DEC  DE     ; N:d                 -
        E(1, 1),   // 9A: DEC  HL     ; N:d                 -
        E(0, 0),   // 9B: -    -      ; -                   -
        E(1, 1),   // 9C: DEC  IX     ; N:d                 -
        E(1, 1),   // 9D: DEC  IY     ; N:d                 -
        E(1, 1),   // 9E: DEC  SP     ; N:d                 -
        E(2, 14),  // 9F: DECW (n)    ; 2:d:E:e:N:F:f       -
        E(1, 1),   // A0: RLCA -      ; N                   -
        E(1, 1),   // A1: RRCA -      ; N                   -
        E(1, 1),   // A2: RLA  -      ; N                   -
        E(1, 1),   // A3: RRA  -      ; N                   -
        E(1, 1),   // A4: SLAA -      ; N                   -
        E(1, 1),   // A5: SRAA -      ; N                   -
        E(1, 1),   // A6: SLLA -      ; N                   -
        E(1, 1),   // A7: SRLA -      ; N                   -
        E(2, 6),   // A8: BIT  0,(n)  ; 2:d:E:N             -
        E(2, 6),   // A9: BIT  1,(n)  ; 2:d:E:N             -
        E(2, 6),   // AA: BIT  2,(n)  ; 2:d:E:N             -
        E(2, 6),   // AB: BIT  3,(n)  ; 2:d:E:N             -
        E(2, 6),   // AC: BIT  4,(n)  ; 2:d:E:N             -
        E(2, 6),   // AD: BIT  5,(n)  ; 2:d:E:N             -
        E(2, 6),   // AE: BIT  6,(n)  ; 2:d:E:N             -
        E(2, 6),   // AF: BIT  7,(n)  ; 2:d:E:N             -
        E(2, 13),  // B0: RES  0,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // B1: RES  1,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // B2: RES  2,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // B3: RES  3,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // B4: RES  4,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // B5: RES  5,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // B6: RES  6,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // B7: RES  7,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // B8: SET  0,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // B9: SET  1,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // BA: SET  2,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // BB: SET  3,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // BC: SET  4,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // BD: SET  5,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // BE: SET  6,(n)  ; 2:d:E:N:F           -
        E(2, 13),  // BF: SET  7,(n)  ; 2:d:E:N:F           -
        E(2, 34),  // C0: JR   F,d    ; 2:n:d:B             2:N
        E(2, 34),  // C1: JR   LT,d   ; 2:n:d:B             2:N
        E(2, 34),  // C2: JR   LE,d   ; 2:n:d:B             2:N
        E(2, 34),  // C3: JR   ULE,d  ; 2:n:d:B             2:N
        E(2, 34),  // C4: JR   OV,d   ; 2:n:d:B             2:N
        E(2, 34),  // C5: JR   MI,d   ; 2:n:d:B             2:N
        E(2, 34),  // C6: JR   Z,d    ; 2:n:d:B             2:N
        E(2, 34),  // C7: JR   C,d    ; 2:n:d:B             2:N
        E(2, 15),  // C8: JR   d      ; 2:n:d:B             -
        E(2, 34),  // C9: JR   GE,d   ; 2:n:d:B             2:N
        E(2, 34),  // CA: JR   GT,d   ; 2:n:d:B             2:N
        E(2, 34),  // CB: JR   UGT,d  ; 2:n:d:B             2:N
        E(2, 34),  // CC: JR   UGE,d  ; 2:n:d:B             2:N
        E(2, 34),  // CD: JR   NOV,d  ; 2:n:d:B             2:N
        E(2, 34),  // CE: JR   NZ,d   ; 2:n:d:B             2:N
        E(2, 34),  // CF: JR   NC,d   ; 2:n:d:B             2:N
        E(0, 0),   // D0: -    -      ; -                   -
        E(0, 0),   // D1: -    -      ; -                   -
        E(0, 0),   // D2: -    -      ; -                   -
        E(0, 0),   // D3: -    -      ; -                   -
        E(0, 0),   // D4: -    -      ; -                   -
        E(0, 0),   // D5: -    -      ; -                   -
        E(0, 0),   // D6: -    -      ; -                   -
        E(0, 0),   // D7: -    -      ; -                   -
        E(0, 0),   // D8: -    -      ; -                   -
        E(0, 0),   // D9: -    -      ; -                   -
        E(0, 0),   // DA: -    -      ; -                   -
        E(0, 0),   // DB: -    -      ; -                   -
        E(0, 0),   // DC: -    -      ; -                   -
        E(0, 0),   // DD: -    -      ; -                   -
        E(0, 0),   // DE: -    -      ; -                   -
        E(0, 0),   // DF: -    -      ; -                   -
        E(1, 0),   // E0: -    (BC)   ; -                   -
        E(1, 0),   // E1: -    (DE)   ; -                   -
        E(1, 0),   // E2: -    (HL)   ; -                   -
        E(3, 0),   // E3: -    (mn)   ; -                   -
        E(1, 0),   // E4: -    (IX)   ; -                   -
        E(1, 0),   // E5: -    (IY)   ; -                   -
        E(1, 0),   // E6: -    (SP)   ; -                   -
        E(2, 0),   // E7: -    (d)    ; -                   -
        E(1, 0),   // E8: -    (BC)   ; -                   -
        E(1, 0),   // E9: -    (DE)   ; -                   -
        E(1, 0),   // EA: -    (HL)   ; -                   -
        E(3, 0),   // EB: -    (mn)   ; -                   -
        E(1, 0),   // EC: -    (IX)   ; -                   -
        E(1, 0),   // ED: -    (IY)   ; -                   -
        E(1, 0),   // EE: -    (SP)   ; -                   -
        E(2, 0),   // EF: -    (d)    ; -                   -
        E(2, 0),   // F0: -    (IX+d) ; -                   -
        E(2, 0),   // F1: -    (IY+d) ; -                   -
        E(2, 0),   // F2: -    (SP+d) ; -                   -
        E(1, 0),   // F3: -    (HL+A) ; -                   -
        E(2, 0),   // F4: -    (IX+d) ; -                   -
        E(2, 0),   // F5: -    (IY+d) ; -                   -
        E(2, 0),   // F6: -    (SP+d) ; -                   -
        E(1, 0),   // F7: -    (HL+A) ; -                   -
        E(1, 0),   // F8: -    B/BC   ; -                   -
        E(1, 0),   // F9: -    C/DE   ; -                   -
        E(1, 0),   // FA: -    D/HL   ; -                   -
        E(1, 0),   // FB: -    E      ; -                   -
        E(1, 0),   // FC: -    H/IX   ; -                   -
        E(1, 0),   // FD: -    L/IY   ; -                   -
        E(1, 0),   // FE: -    A/SP   ; -                   -
        E(1, 16),  // FF: SWI  -      ; n:d:d:1:d:x:X:w:W:1 -
};

constexpr uint8_t PAGE1[] = {
        E(0, 0),   // 00: -    -      ; -                   -
        E(0, 0),   // 01: -    -      ; -                   -
        E(0, 0),   // 02: -    -      ; -                   -
        E(0, 0),   // 03: -    -      ; -                   -
        E(0, 0),   // 04: -    -      ; -                   -
        E(0, 0),   // 05: -    -      ; -                   -
        E(0, 0),   // 06: -    -      ; -                   -
        E(0, 0),   // 07: -    -      ; -                   -
        E(0, 0),   // 08: -    -      ; -                   -
        E(0, 0),   // 09: -    -      ; -                   -
        E(0, 0),   // 0A: -    -      ; -                   -
        E(0, 0),   // 0B: -    -      ; -                   -
        E(0, 0),   // 0C: -    -      ; -                   -
        E(0, 0),   // 0D: -    -      ; -                   -
        E(0, 0),   // 0E: -    -      ; -                   -
        E(0, 0),   // 0F: -    -      ; -                   -
        E(0, 0),   // 10: -    -      ; -                   -
        E(0, 0),   // 11: -    -      ; -                   -
        E(1, 17),  // 12: MUL  HL,g   ; n:d:d:d:d:d:d:N     -
        E(1, 17),  // 13: DIV  HL,g   ; n:d:d:d:d:d:d:N     -
        E(1, 1),   // 14: ADD  IX,gg  ; N:d:d               -
        E(1, 1),   // 15: ADD  IY,gg  ; N:d:d               -
        E(1, 1),   // 16: ADD  SP,gg  ; N:d:d               -
        E(0, 0),   // 17: -    -      ; -                   -
        E(1, 1),   // 18: TSET 0,g    ; N:d:d               -
        E(1, 1),   // 19: TSET 1,g    ; N:d:d               -
        E(1, 1),   // 1A: TSET 2,g    ; N:d:d               -
        E(1, 1),   // 1B: TSET 3,g    ; N:d:d               -
        E(1, 1),   // 1C: TSET 4,g    ; N:d:d               -
        E(1, 1),   // 1D: TSET 5,g    ; N:d:d               -
        E(1, 1),   // 1E: TSET 6,g    ; N:d:d               -
        E(1, 1),   // 1F: TSET 7,g    ; N:d:d               -
        E(0, 0),   // 20: -    -      ; -                   -
        E(0, 0),   // 21: -    -      ; -                   -
        E(0, 0),   // 22: -    -      ; -                   -
        E(0, 0),   // 23: -    -      ; -                   -
        E(0, 0),   // 24: -    -      ; -                   -
        E(0, 0),   // 25: -    -      ; -                   -
        E(0, 0),   // 26: -    -      ; -                   -
        E(0, 0),   // 27: -    -      ; -                   -
        E(0, 0),   // 28: -    -      ; -                   -
        E(0, 0),   // 29: -    -      ; -                   -
        E(0, 0),   // 2A: -    -      ; -                   -
        E(0, 0),   // 2B: -    -      ; -                   -
        E(0, 0),   // 2C: -    -      ; -                   -
        E(0, 0),   // 2D: -    -      ; -                   -
        E(0, 0),   // 2E: -    -      ; -                   -
        E(0, 0),   // 2F: -    -      ; -                   -
        E(1, 1),   // 30: LD   B,g    ; N                   -
        E(1, 1),   // 31: LD   C,g    ; N                   -
        E(1, 1),   // 32: LD   D,g    ; N                   -
        E(1, 1),   // 33: LD   E,g    ; N                   -
        E(1, 1),   // 34: LD   H,g    ; N                   -
        E(1, 1),   // 35: LD   L,g    ; N                   -
        E(1, 1),   // 36: LD   A,g    ; N                   -
        E(0, 0),   // 37: -    -      ; -                   -
        E(1, 1),   // 38: LD   BC,gg  ; N:d                 -
        E(1, 1),   // 39: LD   DE,gg  ; N:d                 -
        E(1, 1),   // 3A: LD   HL,gg  ; N:d                 -
        E(0, 0),   // 3B: -    -      ; -                   -
        E(1, 1),   // 3C: LD   IX,gg  ; N:d                 -
        E(1, 1),   // 3D: LD   IY,gg  ; N:d                 -
        E(1, 1),   // 3E: LD   SP,gg  ; N:d                 -
        E(0, 0),   // 3F: -    -      ; -                   -
        E(0, 0),   // 40: -    -      ; -                   -
        E(0, 0),   // 41: -    -      ; -                   -
        E(0, 0),   // 42: -    -      ; -                   -
        E(0, 0),   // 43: -    -      ; -                   -
        E(0, 0),   // 44: -    -      ; -                   -
        E(0, 0),   // 45: -    -      ; -                   -
        E(0, 0),   // 46: -    -      ; -                   -
        E(0, 0),   // 47: -    -      ; -                   -
        E(0, 0),   // 48: -    -      ; -                   -
        E(0, 0),   // 49: -    -      ; -                   -
        E(0, 0),   // 4A: -    -      ; -                   -
        E(0, 0),   // 4B: -    -      ; -                   -
        E(0, 0),   // 4C: -    -      ; -                   -
        E(0, 0),   // 4D: -    -      ; -                   -
        E(0, 0),   // 4E: -    -      ; -                   -
        E(0, 0),   // 4F: -    -      ; -                   -
        E(0, 0),   // 50: -    -      ; -                   -
        E(0, 0),   // 51: -    -      ; -                   -
        E(0, 0),   // 52: -    -      ; -                   -
        E(0, 0),   // 53: -    -      ; -                   -
        E(0, 0),   // 54: -    -      ; -                   -
        E(0, 0),   // 55: -    -      ; -                   -
        E(0, 0),   // 56: -    -      ; -                   -
        E(0, 0),   // 57: -    -      ; -                   -
        E(1, 18),  // 58: LDI  -      ; n:R:W:d:d:N         -
        E(1, 36),  // 59: LDIR -      ; n:R:W:d:d:d:d:0     n:R:W:d:d:N
        E(1, 18),  // 5A: LDD  -      ; n:R:W:d:d:N         -
        E(1, 36),  // 5B: LDDR -      ; n:R:W:d:d:d:d:0     n:R:W:d:d:N
        E(1, 19),  // 5C: CPI  -      ; n:R:d:d:d:N         -
        E(1, 38),  // 5D: CPIR -      ; n:R:d:d:d:d:d:0     n:R:d:d:d:N
        E(1, 19),  // 5E: CPD  -      ; n:R:d:d:d:N         -
        E(1, 38),  // 5F: CPDR -      ; n:R:d:d:d:d:d:0     n:R:d:d:d:N
        E(1, 1),   // 60: ADD  A,g    ; N                   -
        E(1, 1),   // 61: ADC  A,g    ; N                   -
        E(1, 1),   // 62: SUB  A,g    ; N                   -
        E(1, 1),   // 63: SBC  A,g    ; N                   -
        E(1, 1),   // 64: AND  A,g    ; N                   -
        E(1, 1),   // 65: XOR  A,g    ; N                   -
        E(1, 1),   // 66: OR   A,g    ; N                   -
        E(1, 1),   // 67: CP   A,g    ; N                   -
        E(2, 1),   // 68: ADD  g,n    ; 2:N                 -
        E(2, 1),   // 69: ADC  g,n    ; 2:N                 -
        E(2, 1),   // 6A: SUB  g,n    ; 2:N                 -
        E(2, 1),   // 6B: SBC  g,n    ; 2:N                 -
        E(2, 1),   // 6C: AND  g,n    ; 2:N                 -
        E(2, 1),   // 6D: XOR  g,n    ; 2:N                 -
        E(2, 1),   // 6E: OR   g,n    ; 2:N                 -
        E(2, 1),   // 6F: CP   g,n    ; 2:N                 -
        E(1, 1),   // 70: ADD  HL,gg  ; N:d:d               -
        E(1, 1),   // 71: ADC  HL,gg  ; N:d:d               -
        E(1, 1),   // 72: SUB  HL,gg  ; N:d:d               -
        E(1, 1),   // 73: SBC  HL,gg  ; N:d:d               -
        E(1, 1),   // 74: AND  HL,gg  ; N:d:d               -
        E(1, 1),   // 75: XOR  HL,gg  ; N:d:d               -
        E(1, 1),   // 76: OR   HL,gg  ; N:d:d               -
        E(1, 1),   // 77: CP   HL,gg  ; N:d:d               -
        E(0, 0),   // 78: -    -      ; -                   -
        E(0, 0),   // 79: -    -      ; -                   -
        E(0, 0),   // 7A: -    -      ; -                   -
        E(0, 0),   // 7B: -    -      ; -                   -
        E(0, 0),   // 7C: -    -      ; -                   -
        E(0, 0),   // 7D: -    -      ; -                   -
        E(0, 0),   // 7E: -    -      ; -                   -
        E(0, 0),   // 7F: -    -      ; -                   -
        E(0, 0),   // 80: -    -      ; -                   -
        E(0, 0),   // 81: -    -      ; -                   -
        E(0, 0),   // 82: -    -      ; -                   -
        E(0, 0),   // 83: -    -      ; -                   -
        E(0, 0),   // 84: -    -      ; -                   -
        E(0, 0),   // 85: -    -      ; -                   -
        E(0, 0),   // 86: -    -      ; -                   -
        E(0, 0),   // 87: -    -      ; -                   -
        E(0, 0),   // 88: -    -      ; -                   -
        E(0, 0),   // 89: -    -      ; -                   -
        E(0, 0),   // 8A: -    -      ; -                   -
        E(0, 0),   // 8B: -    -      ; -                   -
        E(0, 0),   // 8C: -    -      ; -                   -
        E(0, 0),   // 8D: -    -      ; -                   -
        E(0, 0),   // 8E: -    -      ; -                   -
        E(0, 0),   // 8F: -    -      ; -                   -
        E(0, 0),   // 90: -    -      ; -                   -
        E(0, 0),   // 91: -    -      ; -                   -
        E(0, 0),   // 92: -    -      ; -                   -
        E(0, 0),   // 93: -    -      ; -                   -
        E(0, 0),   // 94: -    -      ; -                   -
        E(0, 0),   // 95: -    -      ; -                   -
        E(0, 0),   // 96: -    -      ; -                   -
        E(0, 0),   // 97: -    -      ; -                   -
        E(0, 0),   // 98: -    -      ; -                   -
        E(0, 0),   // 99: -    -      ; -                   -
        E(0, 0),   // 9A: -    -      ; -                   -
        E(0, 0),   // 9B: -    -      ; -                   -
        E(0, 0),   // 9C: -    -      ; -                   -
        E(0, 0),   // 9D: -    -      ; -                   -
        E(0, 0),   // 9E: -    -      ; -                   -
        E(0, 0),   // 9F: -    -      ; -                   -
        E(1, 1),   // A0: RLC  g      ; N                   -
        E(1, 1),   // A1: RRC  g      ; N                   -
        E(1, 1),   // A2: RL   g      ; N                   -
        E(1, 1),   // A3: RR   g      ; N                   -
        E(1, 1),   // A4: SLA  g      ; N                   -
        E(1, 1),   // A5: SRA  g      ; N                   -
        E(1, 1),   // A6: SLL  g      ; N                   -
        E(1, 1),   // A7: SRL  g      ; N                   -
        E(1, 1),   // A8: BIT  0,g    ; N                   -
        E(1, 1),   // A9: BIT  1,g    ; N                   -
        E(1, 1),   // AA: BIT  2,g    ; N                   -
        E(1, 1),   // AB: BIT  3,g    ; N                   -
        E(1, 1),   // AC: BIT  4,g    ; N                   -
        E(1, 1),   // AD: BIT  5,g    ; N                   -
        E(1, 1),   // AE: BIT  6,g    ; N                   -
        E(1, 1),   // AF: BIT  7,g    ; N                   -
        E(1, 1),   // B0: RES  0,g    ; N                   -
        E(1, 1),   // B1: RES  1,g    ; N                   -
        E(1, 1),   // B2: RES  2,g    ; N                   -
        E(1, 1),   // B3: RES  3,g    ; N                   -
        E(1, 1),   // B4: RES  4,g    ; N                   -
        E(1, 1),   // B5: RES  5,g    ; N                   -
        E(1, 1),   // B6: RES  6,g    ; N                   -
        E(1, 1),   // B7: RES  7,g    ; N                   -
        E(1, 1),   // B8: SET  0,g    ; N                   -
        E(1, 1),   // B9: SET  1,g    ; N                   -
        E(1, 1),   // BA: SET  2,g    ; N                   -
        E(1, 1),   // BB: SET  3,g    ; N                   -
        E(1, 1),   // BC: SET  4,g    ; N                   -
        E(1, 1),   // BD: SET  5,g    ; N                   -
        E(1, 1),   // BE: SET  6,g    ; N                   -
        E(1, 1),   // BF: SET  7,g    ; N                   -
        E(0, 0),   // C0: -    -      ; -                   -
        E(0, 0),   // C1: -    -      ; -                   -
        E(0, 0),   // C2: -    -      ; -                   -
        E(0, 0),   // C3: -    -      ; -                   -
        E(0, 0),   // C4: -    -      ; -                   -
        E(0, 0),   // C5: -    -      ; -                   -
        E(0, 0),   // C6: -    -      ; -                   -
        E(0, 0),   // C7: -    -      ; -                   -
        E(0, 0),   // C8: -    -      ; -                   -
        E(0, 0),   // C9: -    -      ; -                   -
        E(0, 0),   // CA: -    -      ; -                   -
        E(0, 0),   // CB: -    -      ; -                   -
        E(0, 0),   // CC: -    -      ; -                   -
        E(0, 0),   // CD: -    -      ; -                   -
        E(0, 0),   // CE: -    -      ; -                   -
        E(0, 0),   // CF: -    -      ; -                   -
        E(1, 40),  // D0: RET  F      ; n:R:r:d:P           N:d
        E(1, 40),  // D1: RET  LT     ; n:R:r:d:P           N:d
        E(1, 40),  // D2: RET  LE     ; n:R:r:d:P           N:d
        E(1, 40),  // D3: RET  ULE    ; n:R:r:d:P           N:d
        E(1, 40),  // D4: RET  OV     ; n:R:r:d:P           N:d
        E(1, 40),  // D5: RET  MI     ; n:R:r:d:P           N:d
        E(1, 40),  // D6: RET  Z      ; n:R:r:d:P           N:d
        E(1, 40),  // D7: RET  C      ; n:R:r:d:P           N:d
        E(1, 4),   // D8: RET  -      ; n:R:r:d:1           -
        E(1, 40),  // D9: RET  GE     ; n:R:r:d:P           N:d
        E(1, 40),  // DA: RET  GT     ; n:R:r:d:P           N:d
        E(1, 40),  // DB: RET  UGT    ; n:R:r:d:P           N:d
        E(1, 40),  // DC: RET  UGE    ; n:R:r:d:P           N:d
        E(1, 40),  // DD: RET  NOV    ; n:R:r:d:P           N:d
        E(1, 40),  // DE: RET  NZ     ; n:R:r:d:P           N:d
        E(1, 40),  // DF: RET  NC     ; n:R:r:d:P           N:d
        E(0, 0),   // E0: -    -      ; -                   -
        E(0, 0),   // E1: -    -      ; -                   -
        E(0, 0),   // E2: -    -      ; -                   -
        E(0, 0),   // E3: -    -      ; -                   -
        E(0, 0),   // E4: -    -      ; -                   -
        E(0, 0),   // E5: -    -      ; -                   -
        E(0, 0),   // E6: -    -      ; -                   -
        E(0, 0),   // E7: -    -      ; -                   -
        E(0, 0),   // E8: -    -      ; -                   -
        E(0, 0),   // E9: -    -      ; -                   -
        E(0, 0),   // EA: -    -      ; -                   -
        E(0, 0),   // EB: -    -      ; -                   -
        E(0, 0),   // EC: -    -      ; -                   -
        E(0, 0),   // ED: -    -      ; -                   -
        E(0, 0),   // EE: -    -      ; -                   -
        E(0, 0),   // EF: -    -      ; -                   -
        E(0, 0),   // F0: -    -      ; -                   -
        E(0, 0),   // F1: -    -      ; -                   -
        E(0, 0),   // F2: -    -      ; -                   -
        E(0, 0),   // F3: -    -      ; -                   -
        E(0, 0),   // F4: -    -      ; -                   -
        E(0, 0),   // F5: -    -      ; -                   -
        E(0, 0),   // F6: -    -      ; -                   -
        E(0, 0),   // F7: -    -      ; -                   -
        E(0, 0),   // F8: -    -      ; -                   -
        E(0, 0),   // F9: -    -      ; -                   -
        E(0, 0),   // FA: -    -      ; -                   -
        E(0, 0),   // FB: -    -      ; -                   -
        E(0, 0),   // FC: -    -      ; -                   -
        E(0, 0),   // FD: -    -      ; -                   -
        E(0, 0),   // FE: -    -      ; -                   -
        E(0, 0),   // FF: -    -      ; -                   -
};

constexpr uint8_t PAGE2[] = {
        E(0, 0),   // 00: -    -      ; -                   -
        E(0, 0),   // 01: -    -      ; -                   -
        E(0, 0),   // 02: -    -      ; -                   -
        E(0, 0),   // 03: -    -      ; -                   -
        E(0, 0),   // 04: -    -      ; -                   -
        E(0, 0),   // 05: -    -      ; -                   -
        E(0, 0),   // 06: -    -      ; -                   -
        E(0, 0),   // 07: -    -      ; -                   -
        E(0, 0),   // 08: -    -      ; -                   -
        E(0, 0),   // 09: -    -      ; -                   -
        E(0, 0),   // 0A: -    -      ; -                   -
        E(0, 0),   // 0B: -    -      ; -                   -
        E(0, 0),   // 0C: -    -      ; -                   -
        E(0, 0),   // 0D: -    -      ; -                   -
        E(0, 0),   // 0E: -    -      ; -                   -
        E(0, 0),   // 0F: -    -      ; -                   -
        E(1, 20),  // 10: RLD  (gg)   ; R:d:d:N:W           -
        E(1, 20),  // 11: RRD  (gg)   ; R:d:d:N:W           -
        E(1, 21),  // 12: MUL  HL,(gg); R:d:d:d:d:d:d:N     -
        E(1, 21),  // 13: DIV  HL,(gg); R:d:d:d:d:d:d:N     -
        E(1, 22),  // 14: ADD  IX,(gg); R:r:N               -
        E(1, 22),  // 15: ADD  IY,(gg); R:r:N               -
        E(1, 22),  // 16: ADD  SP,(gg); R:r:N               -
        E(0, 0),   // 17: -    -      ; -                   -
        E(1, 20),  // 18: TSET 0,(gg) ; R:N:d:d:W           -
        E(1, 20),  // 19: TSET 1,(gg) ; R:N:d:d:W           -
        E(1, 20),  // 1A: TSET 2,(gg) ; R:N:d:d:W           -
        E(1, 20),  // 1B: TSET 3,(gg) ; R:N:d:d:W           -
        E(1, 20),  // 1C: TSET 4,(gg) ; R:N:d:d:W           -
        E(1, 20),  // 1D: TSET 5,(gg) ; R:N:d:d:W           -
        E(1, 20),  // 1E: TSET 6,(gg) ; R:N:d:d:W           -
        E(1, 20),  // 1F: TSET 7,(gg) ; R:N:d:d:W           -
        E(0, 0),   // 20: -    -      ; -                   -
        E(0, 0),   // 21: -    -      ; -                   -
        E(0, 0),   // 22: -    -      ; -                   -
        E(0, 0),   // 23: -    -      ; -                   -
        E(0, 0),   // 24: -    -      ; -                   -
        E(0, 0),   // 25: -    -      ; -                   -
        E(0, 0),   // 26: -    -      ; -                   -
        E(0, 0),   // 27: -    -      ; -                   -
        E(1, 21),  // 28: LD   B,(gg) ; R:N                 -
        E(1, 21),  // 29: LD   C,(gg) ; R:N                 -
        E(1, 21),  // 2A: LD   D,(gg) ; R:N                 -
        E(1, 21),  // 2B: LD   E,(gg) ; R:N                 -
        E(1, 21),  // 2C: LD   H,(gg) ; R:N                 -
        E(1, 21),  // 2D: LD   L,(gg) ; R:N                 -
        E(1, 21),  // 2E: LD   A,(gg) ; R:N                 -
        E(0, 0),   // 2F: -    -      ; -                   -
        E(0, 0),   // 30: -    -      ; -                   -
        E(0, 0),   // 31: -    -      ; -                   -
        E(0, 0),   // 32: -    -      ; -                   -
        E(0, 0),   // 33: -    -      ; -                   -
        E(0, 0),   // 34: -    -      ; -                   -
        E(0, 0),   // 35: -    -      ; -                   -
        E(0, 0),   // 36: -    -      ; -                   -
        E(0, 0),   // 37: -    -      ; -                   -
        E(0, 0),   // 38: -    -      ; -                   -
        E(0, 0),   // 39: -    -      ; -                   -
        E(0, 0),   // 3A: -    -      ; -                   -
        E(0, 0),   // 3B: -    -      ; -                   -
        E(0, 0),   // 3C: -    -      ; -                   -
        E(0, 0),   // 3D: -    -      ; -                   -
        E(0, 0),   // 3E: -    -      ; -                   -
        E(0, 0),   // 3F: -    -      ; -                   -
        E(0, 0),   // 40: -    -      ; -                   -
        E(0, 0),   // 41: -    -      ; -                   -
        E(0, 0),   // 42: -    -      ; -                   -
        E(0, 0),   // 43: -    -      ; -                   -
        E(0, 0),   // 44: -    -      ; -                   -
        E(0, 0),   // 45: -    -      ; -                   -
        E(0, 0),   // 46: -    -      ; -                   -
        E(0, 0),   // 47: -    -      ; -                   -
        E(1, 22),  // 48: LD   BC,(gg); R:r:N               -
        E(1, 22),  // 49: LD   DE,(gg); R:r:N               -
        E(1, 22),  // 4A: LD   HL,(gg); R:r:N               -
        E(0, 0),   // 4B: -    -      ; -                   -
        E(1, 22),  // 4C: LD   IX,(gg); R:r:N               -
        E(1, 22),  // 4D: LD   IY,(gg); R:r:N               -
        E(1, 22),  // 4E: LD   SP,(gg); R:r:N               -
        E(0, 0),   // 4F: -    -      ; -                   -
        E(1, 23),  // 50: EX   (gg),BC; R:r:d:W:w:N         -
        E(1, 23),  // 51: EX   (gg),DE; R:r:d:W:w:N         -
        E(1, 23),  // 52: EX   (gg),HL; R:r:d:W:w:N         -
        E(0, 0),   // 53: -    -      ; -                   -
        E(1, 23),  // 54: EX   (gg),IX; R:r:d:W:w:N         -
        E(1, 23),  // 55: EX   (gg),IY; R:r:d:W:w:N         -
        E(1, 23),  // 56: EX   (gg),SP; R:r:d:W:w:N         -
        E(0, 0),   // 57: -    -      ; -                   -
        E(0, 0),   // 58: -    -      ; -                   -
        E(0, 0),   // 59: -    -      ; -                   -
        E(0, 0),   // 5A: -    -      ; -                   -
        E(0, 0),   // 5B: -    -      ; -                   -
        E(0, 0),   // 5C: -    -      ; -                   -
        E(0, 0),   // 5D: -    -      ; -                   -
        E(0, 0),   // 5E: -    -      ; -                   -
        E(0, 0),   // 5F: -    -      ; -                   -
        E(1, 21),  // 60: ADD  A,(gg) ; R:N                 -
        E(1, 21),  // 61: ADC  A,(gg) ; R:N                 -
        E(1, 21),  // 62: SUB  A,(gg) ; R:N                 -
        E(1, 21),  // 63: SBC  A,(gg) ; R:N                 -
        E(1, 21),  // 64: AND  A,(gg) ; R:N                 -
        E(1, 21),  // 65: XOR  A,(gg) ; R:N                 -
        E(1, 21),  // 66: OR   A,(gg) ; R:N                 -
        E(1, 21),  // 67: CP   A,(gg) ; R:N                 -
        E(0, 0),   // 68: -    -      ; -                   -
        E(0, 0),   // 69: -    -      ; -                   -
        E(0, 0),   // 6A: -    -      ; -                   -
        E(0, 0),   // 6B: -    -      ; -                   -
        E(0, 0),   // 6C: -    -      ; -                   -
        E(0, 0),   // 6D: -    -      ; -                   -
        E(0, 0),   // 6E: -    -      ; -                   -
        E(0, 0),   // 6F: -    -      ; -                   -
        E(1, 22),  // 70: ADD  HL,(gg); R:r:N               -
        E(1, 22),  // 71: ADC  HL,(gg); R:r:N               -
        E(1, 22),  // 72: SUB  HL,(gg); R:r:N               -
        E(1, 22),  // 73: SBC  HL,(gg); R:r:N               -
        E(1, 22),  // 74: AND  HL,(gg); R:r:N               -
        E(1, 22),  // 75: XOR  HL,(gg); R:r:N               -
        E(1, 22),  // 76: OR   HL,(gg); R:r:N               -
        E(1, 22),  // 77: CP   HL,(gg); R:r:N               -
        E(0, 0),   // 78: -    -      ; -                   -
        E(0, 0),   // 79: -    -      ; -                   -
        E(0, 0),   // 7A: -    -      ; -                   -
        E(0, 0),   // 7B: -    -      ; -                   -
        E(0, 0),   // 7C: -    -      ; -                   -
        E(0, 0),   // 7D: -    -      ; -                   -
        E(0, 0),   // 7E: -    -      ; -                   -
        E(0, 0),   // 7F: -    -      ; -                   -
        E(0, 0),   // 80: -    -      ; -                   -
        E(0, 0),   // 81: -    -      ; -                   -
        E(0, 0),   // 82: -    -      ; -                   -
        E(0, 0),   // 83: -    -      ; -                   -
        E(0, 0),   // 84: -    -      ; -                   -
        E(0, 0),   // 85: -    -      ; -                   -
        E(0, 0),   // 86: -    -      ; -                   -
        E(1, 20),  // 87: INC  (gg)   ; R:N:W               -
        E(0, 0),   // 88: -    -      ; -                   -
        E(0, 0),   // 89: -    -      ; -                   -
        E(0, 0),   // 8A: -    -      ; -                   -
        E(0, 0),   // 8B: -    -      ; -                   -
        E(0, 0),   // 8C: -    -      ; -                   -
        E(0, 0),   // 8D: -    -      ; -                   -
        E(0, 0),   // 8E: -    -      ; -                   -
        E(1, 20),  // 8F: DEC  (gg)   ; R:N:W               -
        E(0, 0),   // 90: -    -      ; -                   -
        E(0, 0),   // 91: -    -      ; -                   -
        E(0, 0),   // 92: -    -      ; -                   -
        E(0, 0),   // 93: -    -      ; -                   -
        E(0, 0),   // 94: -    -      ; -                   -
        E(0, 0),   // 95: -    -      ; -                   -
        E(0, 0),   // 96: -    -      ; -                   -
        E(1, 24),  // 97: INCW (gg)   ; R:r:N:W:w           -
        E(0, 0),   // 98: -    -      ; -                   -
        E(0, 0),   // 99: -    -      ; -                   -
        E(0, 0),   // 9A: -    -      ; -                   -
        E(0, 0),   // 9B: -    -      ; -                   -
        E(0, 0),   // 9C: -    -      ; -                   -
        E(0, 0),   // 9D: -    -      ; -                   -
        E(0, 0),   // 9E: -    -      ; -                   -
        E(1, 24),  // 9F: DECW (gg)   ; R:r:N:W:w           -
        E(1, 20),  // A0: RLC  (gg)   ; R:N:W               -
        E(1, 20),  // A1: RRC  (gg)   ; R:N:W               -
        E(1, 20),  // A2: RL   (gg)   ; R:N:W               -
        E(1, 20),  // A3: RR   (gg)   ; R:N:W               -
        E(1, 20),  // A4: SLA  (gg)   ; R:N:W               -
        E(1, 20),  // A5: SRA  (gg)   ; R:N:W               -
        E(1, 20),  // A6: SLL  (gg)   ; R:N:W               -
        E(1, 20),  // A7: SRL  (gg)   ; R:N:W               -
        E(1, 21),  // A8: BIT  0,(gg) ; R:N                 -
        E(1, 21),  // A9: BIT  1,(gg) ; R:N                 -
        E(1, 21),  // AA: BIT  2,(gg) ; R:N                 -
        E(1, 21),  // AB: BIT  3,(gg) ; R:N                 -
        E(1, 21),  // AC: BIT  4,(gg) ; R:N                 -
        E(1, 21),  // AD: BIT  5,(gg) ; R:N                 -
        E(1, 21),  // AE: BIT  6,(gg) ; R:N                 -
        E(1, 21),  // AF: BIT  7,(gg) ; R:N                 -
        E(1, 20),  // B0: RES  0,(gg) ; R:N:d:W             -
        E(1, 20),  // B1: RES  1,(gg) ; R:N:d:W             -
        E(1, 20),  // B2: RES  2,(gg) ; R:N:d:W             -
        E(1, 20),  // B3: RES  3,(gg) ; R:N:d:W             -
        E(1, 20),  // B4: RES  4,(gg) ; R:N:d:W             -
        E(1, 20),  // B5: RES  5,(gg) ; R:N:d:W             -
        E(1, 20),  // B6: RES  6,(gg) ; R:N:d:W             -
        E(1, 20),  // B7: RES  7,(gg) ; R:N:d:W             -
        E(1, 20),  // B8: SET  0,(gg) ; R:N:d:W             -
        E(1, 20),  // B9: SET  1,(gg) ; R:N:d:W             -
        E(1, 20),  // BA: SET  2,(gg) ; R:N:d:W             -
        E(1, 20),  // BB: SET  3,(gg) ; R:N:d:W             -
        E(1, 20),  // BC: SET  4,(gg) ; R:N:d:W             -
        E(1, 20),  // BD: SET  5,(gg) ; R:N:d:W             -
        E(1, 20),  // BE: SET  6,(gg) ; R:N:d:W             -
        E(1, 20),  // BF: SET  7,(gg) ; R:N:d:W             -
        E(0, 0),   // C0: -    -      ; -                   -
        E(0, 0),   // C1: -    -      ; -                   -
        E(0, 0),   // C2: -    -      ; -                   -
        E(0, 0),   // C3: -    -      ; -                   -
        E(0, 0),   // C4: -    -      ; -                   -
        E(0, 0),   // C5: -    -      ; -                   -
        E(0, 0),   // C6: -    -      ; -                   -
        E(0, 0),   // C7: -    -      ; -                   -
        E(0, 0),   // C8: -    -      ; -                   -
        E(0, 0),   // C9: -    -      ; -                   -
        E(0, 0),   // CA: -    -      ; -                   -
        E(0, 0),   // CB: -    -      ; -                   -
        E(0, 0),   // CC: -    -      ; -                   -
        E(0, 0),   // CD: -    -      ; -                   -
        E(0, 0),   // CE: -    -      ; -                   -
        E(0, 0),   // CF: -    -      ; -                   -
        E(0, 0),   // D0: -    -      ; -                   -
        E(0, 0),   // D1: -    -      ; -                   -
        E(0, 0),   // D2: -    -      ; -                   -
        E(0, 0),   // D3: -    -      ; -                   -
        E(0, 0),   // D4: -    -      ; -                   -
        E(0, 0),   // D5: -    -      ; -                   -
        E(0, 0),   // D6: -    -      ; -                   -
        E(0, 0),   // D7: -    -      ; -                   -
        E(0, 0),   // D8: -    -      ; -                   -
        E(0, 0),   // D9: -    -      ; -                   -
        E(0, 0),   // DA: -    -      ; -                   -
        E(0, 0),   // DB: -    -      ; -                   -
        E(0, 0),   // DC: -    -      ; -                   -
        E(0, 0),   // DD: -    -      ; -                   -
        E(0, 0),   // DE: -    -      ; -                   -
        E(0, 0),   // DF: -    -      ; -                   -
        E(0, 0),   // E0: -    -      ; -                   -
        E(0, 0),   // E1: -    -      ; -                   -
        E(0, 0),   // E2: -    -      ; -                   -
        E(0, 0),   // E3: -    -      ; -                   -
        E(0, 0),   // E4: -    -      ; -                   -
        E(0, 0),   // E5: -    -      ; -                   -
        E(0, 0),   // E6: -    -      ; -                   -
        E(0, 0),   // E7: -    -      ; -                   -
        E(0, 0),   // E8: -    -      ; -                   -
        E(0, 0),   // E9: -    -      ; -                   -
        E(0, 0),   // EA: -    -      ; -                   -
        E(0, 0),   // EB: -    -      ; -                   -
        E(0, 0),   // EC: -    -      ; -                   -
        E(0, 0),   // ED: -    -      ; -                   -
        E(0, 0),   // EE: -    -      ; -                   -
        E(0, 0),   // EF: -    -      ; -                   -
        E(0, 0),   // F0: -    -      ; -                   -
        E(0, 0),   // F1: -    -      ; -                   -
        E(0, 0),   // F2: -    -      ; -                   -
        E(0, 0),   // F3: -    -      ; -                   -
        E(0, 0),   // F4: -    -      ; -                   -
        E(0, 0),   // F5: -    -      ; -                   -
        E(0, 0),   // F6: -    -      ; -                   -
        E(0, 0),   // F7: -    -      ; -                   -
        E(0, 0),   // F8: -    -      ; -                   -
        E(0, 0),   // F9: -    -      ; -                   -
        E(0, 0),   // FA: -    -      ; -                   -
        E(0, 0),   // FB: -    -      ; -                   -
        E(0, 0),   // FC: -    -      ; -                   -
        E(0, 0),   // FD: -    -      ; -                   -
        E(0, 0),   // FE: -    -      ; -                   -
        E(0, 0),   // FF: -    -      ; -                   -
};

constexpr uint8_t PAGE3[] = {
        E(0, 0),   // 00: -    -      ; -                   -
        E(0, 0),   // 01: -    -      ; -                   -
        E(0, 0),   // 02: -    -      ; -                   -
        E(0, 0),   // 03: -    -      ; -                   -
        E(0, 0),   // 04: -    -      ; -                   -
        E(0, 0),   // 05: -    -      ; -                   -
        E(0, 0),   // 06: -    -      ; -                   -
        E(0, 0),   // 07: -    -      ; -                   -
        E(0, 0),   // 08: -    -      ; -                   -
        E(0, 0),   // 09: -    -      ; -                   -
        E(0, 0),   // 0A: -    -      ; -                   -
        E(0, 0),   // 0B: -    -      ; -                   -
        E(0, 0),   // 0C: -    -      ; -                   -
        E(0, 0),   // 0D: -    -      ; -                   -
        E(0, 0),   // 0E: -    -      ; -                   -
        E(0, 0),   // 0F: -    -      ; -                   -
        E(0, 0),   // 10: -    -      ; -                   -
        E(0, 0),   // 11: -    -      ; -                   -
        E(0, 0),   // 12: -    -      ; -                   -
        E(0, 0),   // 13: -    -      ; -                   -
        E(0, 0),   // 14: -    -      ; -                   -
        E(0, 0),   // 15: -    -      ; -                   -
        E(0, 0),   // 16: -    -      ; -                   -
        E(0, 0),   // 17: -    -      ; -                   -
        E(0, 0),   // 18: -    -      ; -                   -
        E(0, 0),   // 19: -    -      ; -                   -
        E(0, 0),   // 1A: -    -      ; -                   -
        E(0, 0),   // 1B: -    -      ; -                   -
        E(0, 0),   // 1C: -    -      ; -                   -
        E(0, 0),   // 1D: -    -      ; -                   -
        E(0, 0),   // 1E: -    -      ; -                   -
        E(0, 0),   // 1F: -    -      ; -                   -
        E(1, 25),  // 20: LD   (gg),B ; N:W                 -
        E(1, 25),  // 21: LD   (gg),C ; N:W                 -
        E(1, 25),  // 22: LD   (gg),D ; N:W                 -
        E(1, 25),  // 23: LD   (gg),E ; N:W                 -
        E(1, 25),  // 24: LD   (gg),H ; N:W                 -
        E(1, 25),  // 25: LD   (gg),L ; N:W                 -
        E(1, 25),  // 26: LD   (gg),A ; N:W                 -
        E(0, 0),   // 27: -    -      ; -                   -
        E(0, 0),   // 28: -    -      ; -                   -
        E(0, 0),   // 29: -    -      ; -                   -
        E(0, 0),   // 2A: -    -      ; -                   -
        E(0, 0),   // 2B: -    -      ; -                   -
        E(0, 0),   // 2C: -    -      ; -                   -
        E(0, 0),   // 2D: -    -      ; -                   -
        E(0, 0),   // 2E: -    -      ; -                   -
        E(0, 0),   // 2F: -    -      ; -                   -
        E(0, 0),   // 30: -    -      ; -                   -
        E(0, 0),   // 31: -    -      ; -                   -
        E(0, 0),   // 32: -    -      ; -                   -
        E(0, 0),   // 33: -    -      ; -                   -
        E(0, 0),   // 34: -    -      ; -                   -
        E(0, 0),   // 35: -    -      ; -                   -
        E(0, 0),   // 36: -    -      ; -                   -
        E(2, 26),  // 37: LD   (gg),n ; 2:W:N               -
        E(1, 1),   // 38: LD   BC,gg  ; N:d                 -
        E(1, 1),   // 39: LD   DE,gg  ; N:d                 -
        E(1, 1),   // 3A: LD   HL,gg  ; N:d                 -
        E(0, 0),   // 3B: -    -      ; -                   -
        E(1, 1),   // 3C: LD   IX,gg  ; N:d                 -
        E(1, 1),   // 3D: LD   IY,gg  ; N:d                 -
        E(1, 1),   // 3E: LD   SP,gg  ; N:d                 -
        E(3, 27),  // 3F: LDW  (gg),mn; 2:3:N:W:w           -
        E(1, 27),  // 40: LD   (gg),BC; N:W:w               -
        E(1, 27),  // 41: LD   (gg),DE; N:W:w               -
        E(1, 27),  // 42: LD   (gg),HL; N:W:w               -
        E(0, 0),   // 43: -    -      ; -                   -
        E(1, 27),  // 44: LD   (gg),IX; N:W:w               -
        E(1, 27),  // 45: LD   (gg),IY; N:W:w               -
        E(1, 27),  // 46: LD   (gg),SP; N:W:w               -
        E(0, 0),   // 47: -    -      ; -                   -
        E(0, 0),   // 48: -    -      ; -                   -
        E(0, 0),   // 49: -    -      ; -                   -
        E(0, 0),   // 4A: -    -      ; -                   -
        E(0, 0),   // 4B: -    -      ; -                   -
        E(0, 0),   // 4C: -    -      ; -                   -
        E(0, 0),   // 4D: -    -      ; -                   -
        E(0, 0),   // 4E: -    -      ; -                   -
        E(0, 0),   // 4F: -    -      ; -                   -
        E(0, 0),   // 50: -    -      ; -                   -
        E(0, 0),   // 51: -    -      ; -                   -
        E(0, 0),   // 52: -    -      ; -                   -
        E(0, 0),   // 53: -    -      ; -                   -
        E(0, 0),   // 54: -    -      ; -                   -
        E(0, 0),   // 55: -    -      ; -                   -
        E(0, 0),   // 56: -    -      ; -                   -
        E(0, 0),   // 57: -    -      ; -                   -
        E(0, 0),   // 58: -    -      ; -                   -
        E(0, 0),   // 59: -    -      ; -                   -
        E(0, 0),   // 5A: -    -      ; -                   -
        E(0, 0),   // 5B: -    -      ; -                   -
        E(0, 0),   // 5C: -    -      ; -                   -
        E(0, 0),   // 5D: -    -      ; -                   -
        E(0, 0),   // 5E: -    -      ; -                   -
        E(0, 0),   // 5F: -    -      ; -                   -
        E(0, 0),   // 60: -    -      ; -                   -
        E(0, 0),   // 61: -    -      ; -                   -
        E(0, 0),   // 62: -    -      ; -                   -
        E(0, 0),   // 63: -    -      ; -                   -
        E(0, 0),   // 64: -    -      ; -                   -
        E(0, 0),   // 65: -    -      ; -                   -
        E(0, 0),   // 66: -    -      ; -                   -
        E(0, 0),   // 67: -    -      ; -                   -
        E(2, 20),  // 68: ADD  (gg),n ; 2:R:N:W             -
        E(2, 20),  // 69: ADC  (gg),n ; 2:R:N:W             -
        E(2, 20),  // 6A: SUB  (gg),n ; 2:R:N:W             -
        E(2, 20),  // 6B: SBC  (gg),n ; 2:R:N:W             -
        E(2, 20),  // 6C: AND  (gg),n ; 2:R:N:W             -
        E(2, 20),  // 6D: XOR  (gg),n ; 2:R:N:W             -
        E(2, 20),  // 6E: OR   (gg),n ; 2:R:N:W             -
        E(2, 21),  // 6F: CP   (gg),n ; 2:R:N               -
        E(0, 0),   // 70: -    -      ; -                   -
        E(0, 0),   // 71: -    -      ; -                   -
        E(0, 0),   // 72: -    -      ; -                   -
        E(0, 0),   // 73: -    -      ; -                   -
        E(0, 0),   // 74: -    -      ; -                   -
        E(0, 0),   // 75: -    -      ; -                   -
        E(0, 0),   // 76: -    -      ; -                   -
        E(0, 0),   // 77: -    -      ; -                   -
        E(0, 0),   // 78: -    -      ; -                   -
        E(0, 0),   // 79: -    -      ; -                   -
        E(0, 0),   // 7A: -    -      ; -                   -
        E(0, 0),   // 7B: -    -      ; -                   -
        E(0, 0),   // 7C: -    -      ; -                   -
        E(0, 0),   // 7D: -    -      ; -                   -
        E(0, 0),   // 7E: -    -      ; -                   -
        E(0, 0),   // 7F: -    -      ; -                   -
        E(0, 0),   // 80: -    -      ; -                   -
        E(0, 0),   // 81: -    -      ; -                   -
        E(0, 0),   // 82: -    -      ; -                   -
        E(0, 0),   // 83: -    -      ; -                   -
        E(0, 0),   // 84: -    -      ; -                   -
        E(0, 0),   // 85: -    -      ; -                   -
        E(0, 0),   // 86: -    -      ; -                   -
        E(0, 0),   // 87: -    -      ; -                   -
        E(0, 0),   // 88: -    -      ; -                   -
        E(0, 0),   // 89: -    -      ; -                   -
        E(0, 0),   // 8A: -    -      ; -                   -
        E(0, 0),   // 8B: -    -      ; -                   -
        E(0, 0),   // 8C: -    -      ; -                   -
        E(0, 0),   // 8D: -    -      ; -                   -
        E(0, 0),   // 8E: -    -      ; -                   -
        E(0, 0),   // 8F: -    -      ; -                   -
        E(0, 0),   // 90: -    -      ; -                   -
        E(0, 0),   // 91: -    -      ; -                   -
        E(0, 0),   // 92: -    -      ; -                   -
        E(0, 0),   // 93: -    -      ; -                   -
        E(0, 0),   // 94: -    -      ; -                   -
        E(0, 0),   // 95: -    -      ; -                   -
        E(0, 0),   // 96: -    -      ; -                   -
        E(0, 0),   // 97: -    -      ; -                   -
        E(0, 0),   // 98: -    -      ; -                   -
        E(0, 0),   // 99: -    -      ; -                   -
        E(0, 0),   // 9A: -    -      ; -                   -
        E(0, 0),   // 9B: -    -      ; -                   -
        E(0, 0),   // 9C: -    -      ; -                   -
        E(0, 0),   // 9D: -    -      ; -                   -
        E(0, 0),   // 9E: -    -      ; -                   -
        E(0, 0),   // 9F: -    -      ; -                   -
        E(0, 0),   // A0: -    -      ; -                   -
        E(0, 0),   // A1: -    -      ; -                   -
        E(0, 0),   // A2: -    -      ; -                   -
        E(0, 0),   // A3: -    -      ; -                   -
        E(0, 0),   // A4: -    -      ; -                   -
        E(0, 0),   // A5: -    -      ; -                   -
        E(0, 0),   // A6: -    -      ; -                   -
        E(0, 0),   // A7: -    -      ; -                   -
        E(0, 0),   // A8: -    -      ; -                   -
        E(0, 0),   // A9: -    -      ; -                   -
        E(0, 0),   // AA: -    -      ; -                   -
        E(0, 0),   // AB: -    -      ; -                   -
        E(0, 0),   // AC: -    -      ; -                   -
        E(0, 0),   // AD: -    -      ; -                   -
        E(0, 0),   // AE: -    -      ; -                   -
        E(0, 0),   // AF: -    -      ; -                   -
        E(1, 42),  // B0: JP   F,gg   ; n:d:1               N:d
        E(1, 42),  // B1: JP   LT,gg  ; n:d:1               N:d
        E(1, 42),  // B2: JP   LE,gg  ; n:d:1               N:d
        E(1, 42),  // B3: JP   ULE,gg ; n:d:1               N:d
        E(1, 42),  // B4: JP   OV,gg  ; n:d:1               N:d
        E(1, 42),  // B5: JP   MI,gg  ; n:d:1               N:d
        E(1, 42),  // B6: JP   Z,gg   ; n:d:1               N:d
        E(1, 42),  // B7: JP   C,gg   ; n:d:1               N:d
        E(1, 28),  // B8: JP   gg     ; n:d:1               -
        E(1, 42),  // B9: JP   GE,gg  ; n:d:1               N:d
        E(1, 42),  // BA: JP   GT,gg  ; n:d:1               N:d
        E(1, 42),  // BB: JP   UGT,gg ; n:d:1               N:d
        E(1, 42),  // BC: JP   UGE,gg ; n:d:1               N:d
        E(1, 42),  // BD: JP   NOV,gg ; n:d:1               N:d
        E(1, 42),  // BE: JP   NZ,gg  ; n:d:1               N:d
        E(1, 42),  // BF: JP   NC,gg  ; n:d:1               N:d
        E(1, 44),  // C0: CALL F,gg   ; n:d:d:w:W:1         N:d
        E(1, 44),  // C1: CALL LT,gg  ; n:d:d:w:W:1         N:d
        E(1, 44),  // C2: CALL LE,gg  ; n:d:d:w:W:1         N:d
        E(1, 44),  // C3: CALL ULE,gg ; n:d:d:w:W:1         N:d
        E(1, 44),  // C4: CALL OV,gg  ; n:d:d:w:W:1         N:d
        E(1, 44),  // C5: CALL MI,gg  ; n:d:d:w:W:1         N:d
        E(1, 44),  // C6: CALL Z,gg   ; n:d:d:w:W:1         N:d
        E(1, 44),  // C7: CALL C,gg   ; n:d:d:w:W:1         N:d
        E(1, 29),  // C8: CALL gg     ; n:d:d:w:W:1         -
        E(1, 44),  // C9: CALL GE,gg  ; n:d:d:w:W:1         N:d
        E(1, 44),  // CA: CALL GT,gg  ; n:d:d:w:W:1         N:d
        E(1, 44),  // CB: CALL UGT,gg ; n:d:d:w:W:1         N:d
        E(1, 44),  // CC: CALL UGE,gg ; n:d:d:w:W:1         N:d
        E(1, 44),  // CD: CALL NOV,gg ; n:d:d:w:W:1         N:d
        E(1, 44),  // CE: CALL NZ,gg  ; n:d:d:w:W:1         N:d
        E(1, 44),  // CF: CALL NC,gg  ; n:d:d:w:W:1         N:d
        E(0, 0),   // D0: -    -      ; -                   -
        E(0, 0),   // D1: -    -      ; -                   -
        E(0, 0),   // D2: -    -      ; -                   -
        E(0, 0),   // D3: -    -      ; -                   -
        E(0, 0),   // D4: -    -      ; -                   -
        E(0, 0),   // D5: -    -      ; -                   -
        E(0, 0),   // D6: -    -      ; -                   -
        E(0, 0),   // D7: -    -      ; -                   -
        E(0, 0),   // D8: -    -      ; -                   -
        E(0, 0),   // D9: -    -      ; -                   -
        E(0, 0),   // DA: -    -      ; -                   -
        E(0, 0),   // DB: -    -      ; -                   -
        E(0, 0),   // DC: -    -      ; -                   -
        E(0, 0),   // DD: -    -      ; -                   -
        E(0, 0),   // DE: -    -      ; -                   -
        E(0, 0),   // DF: -    -      ; -                   -
        E(0, 0),   // E0: -    -      ; -                   -
        E(0, 0),   // E1: -    -      ; -                   -
        E(0, 0),   // E2: -    -      ; -                   -
        E(0, 0),   // E3: -    -      ; -                   -
        E(0, 0),   // E4: -    -      ; -                   -
        E(0, 0),   // E5: -    -      ; -                   -
        E(0, 0),   // E6: -    -      ; -                   -
        E(0, 0),   // E7: -    -      ; -                   -
        E(0, 0),   // E8: -    -      ; -                   -
        E(0, 0),   // E9: -    -      ; -                   -
        E(0, 0),   // EA: -    -      ; -                   -
        E(0, 0),   // EB: -    -      ; -                   -
        E(0, 0),   // EC: -    -      ; -                   -
        E(0, 0),   // ED: -    -      ; -                   -
        E(0, 0),   // EE: -    -      ; -                   -
        E(0, 0),   // EF: -    -      ; -                   -
        E(0, 0),   // F0: -    -      ; -                   -
        E(0, 0),   // F1: -    -      ; -                   -
        E(0, 0),   // F2: -    -      ; -                   -
        E(0, 0),   // F3: -    -      ; -                   -
        E(0, 0),   // F4: -    -      ; -                   -
        E(0, 0),   // F5: -    -      ; -                   -
        E(0, 0),   // F6: -    -      ; -                   -
        E(0, 0),   // F7: -    -      ; -                   -
        E(0, 0),   // F8: -    -      ; -                   -
        E(0, 0),   // F9: -    -      ; -                   -
        E(0, 0),   // FA: -    -      ; -                   -
        E(0, 0),   // FB: -    -      ; -                   -
        E(0, 0),   // FC: -    -      ; -                   -
        E(0, 0),   // FD: -    -      ; -                   -
        E(0, 0),   // FE: -    -      ; -                   -
        E(0, 0),   // FF: -    -      ; -                   -
};

const uint8_t *inst_page(uint8_t opc) {
    if (opc < 0xE0 || opc == 0xFF)
        return nullptr;
    if (opc < 0xE8)
        return PAGE2;
    if (opc < 0xF0)
        return PAGE3;
    if (opc < 0xF4)
        return PAGE2;
    if (opc < 0xF8)
        return PAGE3;
    return PAGE1;
}

}  // namespace

bool Inst::get(uint16_t addr) {
    const auto opc = Memory.read(addr);
    const auto e = PAGE0[opc];
    instLen = inst_len(e);
    sequence = inst_sequence(e);
    busCycles = bus_cycles(sequence);
    prefetch = prefetch_cycle(sequence);
    const auto *page = inst_page(opc);
    if (page == nullptr) {
        LOG_GET(cli.print(F("@@ get:")));
        if (instLen == 0)
            LOG_GET(cli.print(F(" UNKNOWN")));
        LOG_GET(cli.print(F(" addr=")));
        LOG_GET(cli.printHex(addr, 4));
        LOG_GET(cli.print(F(" opc=")));
        LOG_GET(cli.printHex(opc, 2));
        LOG_GET(cli.print(F(" instLen=")));
        LOG_GET(cli.printDec(instLen));
        LOG_GET(cli.print(F(" busCycles=")));
        LOG_GET(cli.printlnDec(busCycles));
        return instLen != 0;
    }
    const auto op2 = Memory.read(addr + instLen);
    const auto e2 = page[op2];
    const auto len = inst_len(e2);
    if (len == 0) {
        LOG_GET(cli.print(F("@@ get: UNKNOWN addr=")));
        LOG_GET(cli.printHex(addr, 4));
        LOG_GET(cli.print(F(" opc=")));
        LOG_GET(cli.printHex(opc, 2));
        LOG_GET(cli.print(':'));
        LOG_GET(cli.printHex(op2, 2));
        LOG_GET(cli.print(F(" instLen=")));
        LOG_GET(cli.printDec(instLen));
        LOG_GET(cli.print(F(" busCycles=")));
        LOG_GET(cli.printlnDec(busCycles));
        return false;
    }
    instLen += len;
    sequence = inst_sequence(e2);
    busCycles = bus_cycles(sequence);
    prefetch = prefetch_cycle(sequence);
    LOG_GET(cli.print(F("@@ get: addr=")));
    LOG_GET(cli.printHex(addr, 4));
    LOG_GET(cli.print(F(" opc=")));
    LOG_GET(cli.printHex(opc, 2));
    LOG_GET(cli.print(':'));
    LOG_GET(cli.printHex(op2, 2));
    LOG_GET(cli.print(F(" instLen=")));
    LOG_GET(cli.printDec(instLen));
    LOG_GET(cli.print(F(" busCycles=")));
    LOG_GET(cli.printlnDec(busCycles));
    return true;
}

bool Inst::match(const Signals &s, const Signals &end, const Signals *pre) {
    if (instLen == 0)
        return false;
    if (pre) {
        LOG_MATCH(cli.print(F("@@ match: prefetch=")));
        LOG_MATCH(pre->print());
        LOG_MATCH(cli.print(F("           signals=")));
        LOG_MATCH(s.print());
    } else {
        LOG_MATCH(cli.print(F("@@ match:  signals=")));
        LOG_MATCH(s.print());
    }
    const auto cycles = s.diff(end);
    const auto instAddr = pre ? pre->addr : s.addr;
    const auto instOff = pre ? 1 : 0;
    if (cycles < instLen - instOff)
        return false;
    for (auto i = 1; i < instLen; ++i) {
        const auto &f = s.next(i - instOff);
        if (f.read() && f.addr == instAddr + i)
            continue;
        LOG_MATCH(cli.print(F("@@ ")));
        LOG_MATCH(cli.printDec(i + 1, -2));
        if (f.read()) {
            LOG_MATCH(cli.print(F(" addr=")));
            LOG_MATCH(cli.printlnHex(f.addr));
        } else {
            LOG_MATCH(cli.println(F(" not read")));
        }
        return false;
    }
    const auto *seq = BUS_SEQUENCE[sequence];
    if (busMatch(seq, instAddr, s.next(instLen - instOff), end))
        return true;
    if (seq[strlen(seq) - 1] == '@') {
        // check for false condition
        seq = BUS_SEQUENCE[++sequence];
        busCycles = bus_cycles(sequence);
        prefetch = prefetch_cycle(sequence);
        return busMatch(seq, instAddr, s.next(instLen - instOff), end);
    }
    return false;
}

bool Inst::busMatch(const char *seq, uint16_t instAddr, const Signals &s,
        const Signals &end) const {
    LOG_MATCH(cli.print(F("@@ busMatch: sequence=")));
    LOG_MATCH(cli.print(seq));
    LOG_MATCH(cli.print(F(" instAddr=")));
    LOG_MATCH(cli.printHex(instAddr, 4));
    LOG_MATCH(cli.print(F(" busCycles=")));
    LOG_MATCH(cli.printlnDec(busCycles));
    const Signals *r = nullptr;
    const Signals *w = nullptr;
    const Signals *x = nullptr;
    uint16_t p = 0;
    for (auto i = 0; i < busCycles; ++i) {
        const auto &b = s.next(i);
        LOG_MATCH(cli.print(F("  seq=")));
        LOG_MATCH(cli.print(seq[i]));
        LOG_MATCH(cli.print(F(" bus=")));
        LOG_MATCH(b.print());
        switch (seq[i]) {
        case '0':
            if (b.read() && b.addr == instAddr)
                continue;
            LOG_MATCH(cli.println(F("@@ 0")));
            return false;
        case '1':
            if (b.read() && b.addr != instAddr + instLen)
                continue;
            LOG_MATCH(cli.println(F("@@ 1")));
            return false;
        case 'N':
        case 'n':
            if (b.read() && b.addr == instAddr + instLen)
                continue;
            LOG_MATCH(cli.print(F("@@ ")));
            LOG_MATCH(cli.println(seq[i]));
            return false;
        case 'B':
            if (b.read()) {
                const int8_t delta = Memory.read(instAddr + 1);
                LOG_MATCH(cli.print(F("@@ B delta=")));
                LOG_MATCH(cli.printHex(delta & 0xFF, 2));
                LOG_MATCH(cli.print(F(" target=")));
                LOG_MATCH(cli.printlnHex(instAddr + 2 + delta));
                if (b.addr == instAddr + 2 + delta)
                    continue;
            };
            LOG_MATCH(cli.println(F("@@ B")));
            return false;
        case 'P':
            if (b.read() && b.addr == p)
                continue;
            LOG_MATCH(cli.print(F("@@ P poped=")));
            LOG_MATCH(cli.printlnHex(p, 4));
            return false;
        case 'R':
            if (b.read()) {
                r = &b;
                p = b.data;
                continue;
            }
            LOG_MATCH(cli.println(F("@@ R")));
            return false;
        case 'E':
            if (b.read() && b.addr >= 0xFF00) {
                r = &b;
                continue;
            }
            LOG_MATCH(cli.println(F("@@ E")));
            return false;
        case 'r':  // R->r
        case 'e':  // E->e
            if (b.read() && b.addr == r->addr + 1) {
                p |= (static_cast<uint16_t>(b.data) << 8);
                continue;
            }
            LOG_MATCH(cli.print(F("@@ ")));
            LOG_MATCH(cli.println(seq[i]));
            return false;
        case 'S':  // R->r->S
            if (b.read() && b.addr == r->addr + 2)
                continue;
            LOG_MATCH(cli.println(F("@@ S")));
            return false;
        case 's':  // R->r->S->s
            if (b.read() && b.addr == r->addr + 3)
                continue;
            LOG_MATCH(cli.println(F("@@ s")));
            return false;
        case 'W':  // W->w or w->W
            if (b.write() && (w == nullptr || b.addr == w->addr - 1)) {
                if (w == nullptr)
                    w = &b;
                continue;
            }
            LOG_MATCH(cli.println(F("@@ W")));
            return false;
        case 'F':  // F->f
            if (b.write() && b.addr >= 0xFF00) {
                w = &b;
                continue;
            }
            LOG_MATCH(cli.println(F("@@ F")));
            return false;
        case 'w':  // W->w or w->W or x->X->w
            if (b.write() && (w == nullptr || b.addr == w->addr + 1 ||
                                     (x && b.addr == x->addr - 2))) {
                if (w == nullptr)
                    w = &b;
                continue;
            }
            LOG_MATCH(cli.println(F("@@ w")));
            return false;
        case 'f':  // F->f
            if (b.write() && b.addr == w->addr + 1)
                continue;
            LOG_MATCH(cli.println(F("@@ f")));
            return false;
        case 'X':  // x->X
            if (b.write() && b.addr == x->addr - 1)
                continue;
            LOG_MATCH(cli.println(F("@@ X")));
            return false;
        case 'x':  // x->X
            if (b.write()) {
                x = &b;
                continue;
            }
            LOG_MATCH(cli.println(F("@@ x")));
            return false;
        case '@':
            return true;
        default:
            break;
        }
    }
    LOG_MATCH(cli.println(F("@@ busMatch: MATCH")));
    return true;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
