#include <ctype.h>
#include <string.h>

#include "regs.h"

#include <asm_tlcs90.h>
#include <dis_tlcs90.h>
#include <libcli.h>

#include "config.h"
#include "digital_fast.h"
#include "i8251.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli cli;
extern I8251 Usart;

static constexpr bool debug_cycles = false;

struct Regs Regs;
struct Memory Memory;

const char *Regs::cpu() const {
    return "TLC90";
}

const char *Regs::cpuName() const {
    return "TMP90C802";
}

namespace {

libasm::tlcs90::AsmTlcs90 assembler;
libasm::tlcs90::DisTlcs90 disassembler;

char bit1(uint8_t v, char name) {
    return v ? name : '_';
}

void outFlags(char *p, uint8_t f) {
    p[0] = bit1(f & 0x80, 'S');  // Sign
    p[1] = bit1(f & 0x40, 'Z');  // Zero
    p[2] = bit1(f & 0x20, 'I');  // Interrupt enable
    p[3] = bit1(f & 0x10, 'H');  // Half carry
    p[4] = bit1(f & 0x08, 'X');  // Expansion carry
    p[5] = bit1(f & 0x04, 'V');  // Parity/Overflow
    p[6] = bit1(f & 0x02, 'N');  // Addition/Subtraction
    p[7] = bit1(f & 0x01, 'C');  // Carry
}

}  // namespace

void Regs::print() const {
    // clang-format off
    static char main[] = {
        'P', 'C', '=', 0, 0, 0, 0, ' ',      // PC=3
        'S', 'P', '=', 0, 0, 0, 0, ' ',      // SP=11
        ' ',
        'B', 'C', '=', 0, 0, 0, 0, ' ',      // BC=20
        'D', 'E', '=', 0, 0, 0, 0, ' ',      // DE=28
        'H', 'L', '=', 0, 0, 0, 0, ' ',      // HL=36
        'A', '=', 0, 0, ' ',                 // A=43
        'F', '=', 0, 0, 0, 0, 0, 0, 0, 0,    // F=48
        0,
    };
    static char alt[] = {
        'I', 'X', '=', 0, 0, 0, 0, ' ',      // IX=3
        'I', 'Y', '=', 0, 0, 0, 0, ' ',      // IY=11
        '(',
        'B', 'C', '=', 0, 0, 0, 0, ' ',      // BC=20
        'D', 'E', '=', 0, 0, 0, 0, ' ',      // DE=28
        'H', 'L', '=', 0, 0, 0, 0, ' ',      // HL=36
        'A', '=', 0, 0, ' ',                 // A=43
        'F', '=', 0, 0, 0, 0, 0, 0, 0, 0,    // F=48
        ')', 0,
    };
    // clang-format on
    outHex16(main + 3, _pc);
    outHex16(main + 11, _sp);
    outHex16(main + 20, _main.bc());
    outHex16(main + 28, _main.de());
    outHex16(main + 36, _main.hl());
    outHex8(main + 43, _main.a);
    outFlags(main + 48, _main.f);
    cli.println(main);
    Pins.idle();
    outHex16(alt + 3, _ix);
    outHex16(alt + 11, _iy);
    outHex16(alt + 20, _alt.bc());
    outHex16(alt + 28, _alt.de());
    outHex16(alt + 36, _alt.hl());
    outHex8(alt + 43, _alt.a);
    outFlags(alt + 48, _alt.f);
    cli.println(alt);
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

void setle16(uint8_t *p, uint8_t vh, uint8_t vl) {
    p[0] = vl;
    p[1] = vh;
}

void setle16(uint8_t *p, uint16_t v) {
    setle16(p, hi(v), lo(v));
}

}  // namespace

void Regs::reset(bool show) {
    if (show)
        Signals::resetCycles();

    static constexpr uint8_t CONFIG[] = {
            /* Set normal-wait at P3CR */
            0x37, 0xC7, 0x40,  // LD (P3CR), 40H   ; 1:2:d:3:W:N
            /* Disable watchdog timer */
            0x37, 0xD2, 0x00,  // LD (WDMOD), ~WDTE; 2:d:3:W:N
            0x37, 0xD3, 0xB1,  // LD (WDCR), 0B1H  ; 2:d:2:W:N
            /* Jump to reset */
            0x3E, 0x00, 0x10,  // LD SP,1000H      ; 2:3:N
            0x1A, 0x00, 0x00,  // JP (0000H)       ; 2:3:1
    };
    Pins.execInst(CONFIG, sizeof(CONFIG));

    if (show)
        Signals::printCycles();
}

void Regs::interrupt(const Signals &signals, bool nmi) {
    // Machine context were pushed in the following order; PCH, PCL, A, F
    _main.a = signals.next(2).data;
    _main.f = signals.next(3).data;
    if (nmi) {
        // NMI
        _sp = signals.addr + 1;
        _pc = uint16(signals.data, signals.next().data);
    } else {
        // SWI
        _sp = signals.next(3).addr;
        _pc = Signals::currCycle().addr;
        _main.f &= ~0x20;  //  clear F.I
    }
    saveRegs();
}

void Regs::breakPoint(const Signals &signals) {
    interrupt(signals, true);
    _pc -= 1;  // back to break point
}

void Regs::save(bool show) {
    if (debug_cycles)
        cli.println(F("@@ save"));
    if (show)
        Signals::resetCycles();
    static constexpr uint8_t SAVE_SP[] = {
            0xEB, 0x00, 0x00, 0x46,  // LD (0000H),SP ; 1:2:3:4:N:W:W
            0x56,                    // PUSH AF;      ; N:d:W:W
            0x00,                    // NOP           ; N
    };
    uint8_t buffer[4];
    Pins.captureWrites(SAVE_SP, sizeof(SAVE_SP), &_pc, buffer, sizeof(buffer));
    _sp = le16(buffer + 0);
    _main.a = buffer[2];
    _main.f = buffer[3];
    saveRegs();
    if (show)
        Signals::printCycles();
}

void Regs::saveRegs() {
    static constexpr uint8_t SAVE_ALL[] = {
            0x09,  // EX AF,AF' ; N
            0x56,  // PUSH AF   ; N:d:W:W
            0x09,  // EX AF,AF' ; N
            0x50,  // PUSH BC   ; N:d:W:W
            0x51,  // PUSH DE   ; N:d:W:W
            0x52,  // PUSH HL   ; N:d:W:W
            0x54,  // PUSH IX   ; N:d:W:W
            0x55,  // PUSH IY   ; N:d:W:W
            0x0A,  // EXX       ; N
            0x50,  // PUSH BC   ; N:d:W:W
            0x51,  // PUSH DE   ; N:d:W:W
            0x52,  // PUSH HL   ; N:d:W:W
            0x0A,  // EXX       ; N
    };
    uint8_t buffer[2 * 9];
    Pins.captureWrites(
            SAVE_ALL, sizeof(SAVE_ALL), nullptr, buffer, sizeof(buffer));
    _alt.a = buffer[0];
    _alt.f = buffer[1];
    _main.b = buffer[2];
    _main.c = buffer[3];
    _main.d = buffer[4];
    _main.e = buffer[5];
    _main.h = buffer[6];
    _main.l = buffer[7];
    _ix = be16(buffer + 8);
    _iy = be16(buffer + 10);
    _alt.b = buffer[12];
    _alt.c = buffer[13];
    _alt.d = buffer[14];
    _alt.e = buffer[15];
    _alt.h = buffer[16];
    _alt.l = buffer[17];
}

void Regs::restore(bool show) {
    if (debug_cycles)
        cli.println(F("@@ restore"));
    if (show)
        Signals::resetCycles();

    static uint8_t LD_ALL[] = {
            0x09,           // EX AF,AF' ; N
            0x5E, 0, 0, 0,  // POP AF    ; n:R:R:d:N
            0x09,           // EX AF,AF' ; N
            0x5E, 0, 0, 0,  // POP AF    ; n:R:R:d:N
            0x0A,           // EXX       ; N
            0x38, 0, 0,     // LD BC,mn  ; 2:3:N
            0x39, 0, 0,     // LD DE,mn  ; 2:3:N
            0x3A, 0, 0,     // LD HL,nm  ; 2:3:N
            0x0A,           // EXX       ; N
            0x38, 0, 0,     // LD BC,mn  ; 2:3:N
            0x39, 0, 0,     // LD DE,mn  ; 2:3:N
            0x3A, 0, 0,     // LD HL,nm  ; 2:3:N
            0x3C, 0, 0,     // LD IX,nm  ; 2:3:N
            0x3D, 0, 0,     // LD IY,nm  ; 2:3:N
            0x3E, 0, 0,     // LD SP,nm  ; 2:3:N
            0x1A, 0, 0,     // JP nm     ; 2:3:d:1
    };
    LD_ALL[3] = _alt.f;
    LD_ALL[4] = _alt.a;
    LD_ALL[8] = _main.f;
    LD_ALL[9] = _main.a;
    LD_ALL[12] = _alt.e;
    LD_ALL[13] = _alt.d;
    LD_ALL[15] = _alt.c;
    LD_ALL[16] = _alt.b;
    LD_ALL[18] = _alt.l;
    LD_ALL[19] = _alt.h;
    LD_ALL[22] = _main.c;
    LD_ALL[23] = _main.b;
    LD_ALL[25] = _main.e;
    LD_ALL[26] = _main.d;
    LD_ALL[28] = _main.l;
    LD_ALL[29] = _main.h;
    setle16(LD_ALL + 31, _ix);
    setle16(LD_ALL + 34, _iy);
    setle16(LD_ALL + 37, _sp);
    setle16(LD_ALL + 40, _pc);
    Pins.execInst(LD_ALL, sizeof(LD_ALL));

    if (show)
        Signals::printCycles();
}

void Regs::printRegList() const {
    cli.println(F("?Reg: PC SP IX IY BC DE HL A F B C D E H L"));
}

char Regs::validUint8Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("A")) == 0)
        return 'a';
    if (strcasecmp_P(word, PSTR("F")) == 0)
        return 'f';
    if (strcasecmp_P(word, PSTR("B")) == 0)
        return 'b';
    if (strcasecmp_P(word, PSTR("C")) == 0)
        return 'c';
    if (strcasecmp_P(word, PSTR("D")) == 0)
        return 'd';
    if (strcasecmp_P(word, PSTR("E")) == 0)
        return 'e';
    if (strcasecmp_P(word, PSTR("H")) == 0)
        return 'h';
    if (strcasecmp_P(word, PSTR("L")) == 0)
        return 'l';
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("PC")) == 0)
        return 'P';
    if (strcasecmp_P(word, PSTR("SP")) == 0)
        return 'S';
    if (strcasecmp_P(word, PSTR("IX")) == 0)
        return 'X';
    if (strcasecmp_P(word, PSTR("IY")) == 0)
        return 'Y';
    if (strcasecmp_P(word, PSTR("BC")) == 0)
        return 'B';
    if (strcasecmp_P(word, PSTR("DE")) == 0)
        return 'D';
    if (strcasecmp_P(word, PSTR("HL")) == 0)
        return 'H';
    return 0;
}

void Regs::setRegValue(char reg, uint32_t value) {
    switch (reg) {
    case 'P':
        _pc = value;
        break;
    case 'S':
        _sp = value;
        break;
    case 'X':
        _ix = value;
        break;
    case 'Y':
        _iy = value;
        break;
    case 'B':
        _main.setbc(value);
        break;
    case 'D':
        _main.setde(value);
        break;
    case 'H':
        _main.sethl(value);
        break;
    case 'a':
        _main.a = value;
        break;
    case 'f':
        _main.f = value;
        break;
    case 'b':
        _main.b = value;
        break;
    case 'c':
        _main.c = value;
        break;
    case 'd':
        _main.d = value;
        break;
    case 'e':
        _main.e = value;
        break;
    case 'h':
        _main.h = value;
        break;
    case 'l':
        _main.l = value;
        break;
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

bool Memory::is_internal(uint16_t addr) {
    return addr >= 0xFEC0 && addr < 0xFFF0;
}

void Memory::write(uint16_t addr, const uint8_t *data, uint8_t len) {
    for (auto i = 0; i < len; i++) {
        write(addr++, *data++);
    }
}

uint8_t Memory::read(uint16_t addr, const char *space) const {
    if (Usart.isSelected(addr))
        return Usart.read(addr);
    return is_internal(addr) ? internal_read(addr) : raw_read(addr);
}

void Memory::write(uint16_t addr, uint8_t data, const char *space) {
    if (Usart.isSelected(addr))
        return Usart.write(data, addr);
    return is_internal(addr) ? internal_write(addr, data)
                             : raw_write(addr, data);
}

uint8_t Memory::raw_read(uint16_t addr) const {
    return memory[addr];
}

void Memory::raw_write(uint16_t addr, uint8_t data) {
    memory[addr] = data;
}

uint8_t Memory::internal_read(uint16_t addr) const {
    static uint8_t LD[] = {
            0xE3, 0, 0, 0x2E,  // LD A,(mn) ; 2:3:4:R:N
            0xEB, 0, 0, 0x26,  // LD (mn),A ; 2:3:4:N:W
            0x00,              // NOP       ; N
    };
    setle16(LD + 1, addr);
    Pins.captureWrites(LD, sizeof(LD), nullptr, &LD[2], 1);
    return LD[2];
}

void Memory::internal_write(uint16_t addr, uint8_t data) const {
    static uint8_t LD[] = {
            0x36, 0,           // LD A,n    ; 2:N
            0xEB, 0, 0, 0x26,  // LD (mn),A ; 2:3:4:N:W
            0x00,              // NOP       ; N
    };
    LD[1] = data;
    setle16(LD + 3, addr);
    Pins.execInst(LD, sizeof(LD));
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
