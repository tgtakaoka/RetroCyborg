#include "regs.h"

#include <libcli.h>

#include <asm_mc6805.h>
#include <dis_mc6805.h>
#include "mc6850.h"
#include "pins.h"
#include "string_util.h"

extern libcli::Cli &cli;
extern Mc6850 Acia;

libasm::mc6805::AsmMc6805 asm6805;
libasm::mc6805::DisMc6805 dis6805;
libasm::Assembler &assembler(asm6805);
libasm::Disassembler &disassembler(dis6805);

struct Regs Regs;
struct Memory Memory;

static constexpr uint8_t cycles_table[256] = {
        5,   // 00:3 BRSET BTB
        5,   // 01:3 BRCLR BTB
        5,   // 02:3 BRSET BTB
        5,   // 03:3 BRCLR BTB
        5,   // 04:3 BRSET BTB
        5,   // 05:3 BRCLR BTB
        5,   // 06:3 BRSET BTB
        5,   // 07:3 BRCLR BTB
        5,   // 08:3 BRSET BTB
        5,   // 09:3 BRCLR BTB
        5,   // 0A:3 BRSET BTB
        5,   // 0B:3 BRCLR BTB
        5,   // 0C:3 BRSET BTB
        5,   // 0D:3 BRCLR BTB
        5,   // 0E:3 BRSET BTB
        5,   // 0F:3 BRCLR BTB
        5,   // 10:2 BSET BSC
        5,   // 11:2 BCLR BSC
        5,   // 12:2 BSET BSC
        5,   // 13:2 BCLR BSC
        5,   // 14:2 BSET BSC
        5,   // 15:2 BCLR BSC
        5,   // 16:2 BSET BSC
        5,   // 17:2 BCLR BSC
        5,   // 18:2 BSET BSC
        5,   // 19:2 BCLR BSC
        5,   // 1A:2 BSET BSC
        5,   // 1B:2 BCLR BSC
        5,   // 1C:2 BSET BSC
        5,   // 1D:2 BCLR BSC
        5,   // 1E:2 BSET BSC
        5,   // 1F:2 BCLR BSC
        3,   // 20:2 BRA  REL
        3,   // 21:2 BRN  REL
        3,   // 22:2 BHI  REL
        3,   // 23:2 BLS  REL
        3,   // 24:2 BCC  REL
        3,   // 25:2 BCS  REL
        3,   // 26:2 BNE  REL
        3,   // 27:2 BEQ  REL
        3,   // 28:2 BHCC REL
        3,   // 29:2 BHCS REL
        3,   // 2A:2 BPL  REL
        3,   // 2B:2 BMI  REL
        3,   // 2C:2 BMC  REL
        3,   // 2D:2 BMS  REL
        3,   // 2E:2 BIL  REL
        3,   // 2F:2 BIH  REL
        5,   // 30:2 NEG  DIR
        0,   // 31:0 -    -
        0,   // 32:0 -    -
        5,   // 33:2 COM  DIR
        5,   // 34:2 LSR  DIR
        0,   // 35:0 -    -
        5,   // 36:2 ROR  DIR
        5,   // 37:2 ASR  DIR
        5,   // 38:2 LSL  DIR
        5,   // 39:2 ROL  DIR
        5,   // 3A:2 DEC  DIR
        0,   // 3B:0 -    -
        5,   // 3C:2 INC  DIR
        4,   // 3D:2 TST  DIR
        0,   // 3E:0 -    -
        5,   // 3F:2 CLR  DIR
        3,   // 40:1 NEGA INH
        0,   // 41:0 -    -
        0,   // 42:0 -    -
        3,   // 43:1 COMA INH
        3,   // 44:1 LSRA INH
        0,   // 45:0 -    -
        3,   // 46:1 RORA INH
        3,   // 47:1 ASRA INH
        3,   // 48:1 LSLA INH
        3,   // 49:1 ROLA INH
        3,   // 4A:1 DECA INH
        0,   // 4B:0 -    -
        3,   // 4C:1 INCA INH
        3,   // 4D:1 TSTA INH
        0,   // 4E:0 -    -
        3,   // 4F:1 CLRA INH
        3,   // 50:1 NEGX INH
        0,   // 51:0 -    -
        0,   // 52:0 -    -
        3,   // 53:1 COMX INH
        3,   // 54:1 LSRX INH
        0,   // 55:0 -    -
        3,   // 56:1 RORX INH
        3,   // 57:1 ASRX INH
        3,   // 58:1 LSLX INH
        3,   // 59:1 ROLX INH
        3,   // 5X:1 DECX INH
        0,   // 5X:0 -    -
        3,   // 5C:1 INCX INH
        3,   // 5D:1 TSTX INH
        0,   // 5E:0 -    -
        3,   // 5F:1 CLRX INH
        6,   // 60:2 NEG  IX1
        0,   // 61:0 -    -
        0,   // 62:0 -    -
        6,   // 63:2 COM  IX1
        6,   // 64:2 LSR  IX1
        0,   // 65:0 -    -
        6,   // 66:2 ROR  IX1
        6,   // 67:2 ASR  IX1
        6,   // 68:2 LSL  IX1
        6,   // 69:2 ROL  IX1
        6,   // 6A:2 DEC  IX1
        0,   // 6B:0 -    -
        6,   // 6C:2 INC  IX1
        5,   // 6D:2 TST  IX1
        0,   // 6E:0 -    -
        6,   // 6F:2 CLR  IX1
        5,   // 70:1 NEG  IX
        0,   // 71:0 -    -
        0,   // 72:0 -    -
        5,   // 73:1 COM  IX
        5,   // 74:1 LSR  IX
        0,   // 75:0 -    -
        5,   // 76:1 ROR  IX
        5,   // 77:1 ASR  IX
        5,   // 78:1 LSL  IX
        5,   // 79:1 ROL  IX
        5,   // 7A:1 DEC  IX
        0,   // 7B:0 -    -
        5,   // 7C:1 INC  IX
        4,   // 7D:1 TST  IX
        0,   // 7E:0 -    -
        5,   // 7F:1 CLR  IX
        9,   // 80:1 RTI  INH
        6,   // 81:1 RTS  INH
        0,   // 82:0 -    -
        10,  // 83:1 SWI  ING
        0,   // 84:0 -    -
        0,   // 85:0 -    -
        0,   // 86:0 -    -
        0,   // 87:0 -    -
        0,   // 88:0 -    -
        0,   // 89:0 -    -
        0,   // 8A:0 -    -
        0,   // 8B:0 -    -
        0,   // 8C:0 -    -
        0,   // 8D:0 -    -
        2,   // 8E:1 STOP INH
        2,   // 8F:1 WAI  INH
        0,   // 90:0 -    -
        0,   // 91:0 -    -
        0,   // 92:0 -    -
        0,   // 93:0 -    -
        0,   // 94:0 -    -
        0,   // 95:0 -    -
        0,   // 96:0 -    -
        2,   // 97:1 TAX  INH
        2,   // 98:1 CLC  INH
        2,   // 99:1 SEC  INH
        2,   // 9A:1 CLI  INH
        2,   // 9B:1 SEI  INH
        2,   // 9C:1 RSP  INH
        2,   // 9D:1 NOP  INH
        0,   // 9E:0 -    -
        2,   // 9F:1 TXA  INH
        2,   // A0:2 SUB  IMM
        2,   // A1:2 CMP  IMM
        2,   // A2:2 SBC  IMM
        2,   // A3:2 CPX  IMM
        2,   // A4:2 AND  IMM
        2,   // A5:2 BIT  IMM
        2,   // A6:2 LDA  IMM
        0,   // A7:0 -    -
        2,   // A8:2 EOR  IMM
        2,   // A9:2 ADC  IMM
        2,   // AA:2 ORA  IMM
        2,   // AB:2 ADD  IMM
        0,   // AC:0 -    -
        6,   // AD:2 BSR  REL
        2,   // AE:2 LDX  IMM
        0,   // AF:0 -    -
        3,   // B0:2 SUB  DIR
        3,   // B1:2 CMP  DIR
        3,   // B2:2 SBC  DIR
        3,   // B3:2 CPX  DIR
        3,   // B4:2 AND  DIR
        3,   // B5:2 BIT  DIR
        3,   // B6:2 LDA  DIR
        4,   // B7:2 STA  DIR
        3,   // B8:2 EOR  DIR
        3,   // B9:2 ADC  DIR
        3,   // BA:2 ORA  DIR
        3,   // BB:2 ADD  DIR
        5,   // BC:2 JMP  DIR
        5,   // BD:2 JSR  DIR
        3,   // BE:2 LDX  DIR
        4,   // BF:2 STX  DIR
        4,   // C0:3 SUB  EXT
        4,   // C1:3 CMP  EXT
        4,   // C2:3 SBC  EXT
        4,   // C3:3 CPX  EXT
        4,   // C4:3 AND  EXT
        4,   // C5:3 BIT  EXT
        4,   // C6:3 LDA  EXT
        5,   // C7:3 STA  EXT
        4,   // C8:3 EOR  EXT
        4,   // C9:3 ADC  EXT
        4,   // CA:3 ORA  EXT
        4,   // CB:3 ADD  EXT
        3,   // CC:3 JMP  EXT
        6,   // CD:3 JSR  EXT
        4,   // CE:3 LDX  EXT
        5,   // CF:3 STX  EXT
        5,   // D0:3 SUB  IX2
        5,   // D1:3 CMP  IX2
        5,   // D2:3 SBC  IX2
        5,   // D3:3 CPX  IX2
        5,   // D4:3 AND  IX2
        5,   // D5:3 BIT  IX2
        5,   // D6:3 LDA  IX2
        6,   // D7:3 STA  IX2
        5,   // D8:3 EOR  IX2
        5,   // D9:3 ADC  IX2
        5,   // DA:3 ORA  IX2
        5,   // DB:3 ADD  IX2
        4,   // DC:3 JMP  IX2
        7,   // DD:3 JSR  IX2
        5,   // DE:3 LDX  IX2
        6,   // DF:3 STX  IX2
        4,   // E0:2 SUB  IX1
        4,   // E1:2 CMP  IX1
        4,   // E2:2 SBC  IX1
        4,   // E3:2 CPX  IX1
        4,   // E4:2 AND  IX1
        4,   // E5:2 BIT  IX1
        4,   // E6:2 LDA  IX1
        5,   // E7:2 STA  IX1
        4,   // E8:2 EOR  IX1
        4,   // E9:2 ADC  IX1
        4,   // EA:2 ORA  IX1
        4,   // EB:2 ADD  IX1
        3,   // EC:2 JMP  IX1
        6,   // ED:2 JSR  IX1
        4,   // EE:2 LDX  IX1
        5,   // EF:2 STX  IX1
        3,   // F0:1 SUB  IX
        3,   // F1:1 CMP  IX
        3,   // F2:1 SBC  IX
        3,   // F3:1 CPX  IX
        3,   // F4:1 AND  IX
        3,   // F5:1 BIT  IX
        3,   // F6:1 LDA  IX
        4,   // F7:1 STA  IX
        3,   // F8:1 EOR  IX
        3,   // F9:1 ADC  IX
        3,   // FA:1 ORA  IX
        3,   // FB:1 ADD  IX
        2,   // FC:1 JMP  IX
        5,   // FD:1 JSR  IX
        3,   // FE:1 LDX  IX
        4,   // FF:1 STX  IX
};

uint8_t Regs::cycles(uint8_t insn) const {
    return cycles_table[insn];
}

const char *Regs::cpu() const {
    return "MC146805";
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
        'P', 'C', '=', 0, 0, 0, 0, ' ', // PC=3
        'S', 'P', '=', 0, 0, 0, 0, ' ', // SP=11
        'X', '=', 0, 0, ' ',            // X=18
        'A', '=', 0, 0, ' ',            // A=23
        'C', 'C', '=', 0, 0, 0, 0, 0,   // CC=29
        0,                              // EOS
    };
    // clang-format on
    outHex16(buffer + 3, pc);
    outHex16(buffer + 11, sp);
    outHex8(buffer + 18, x);
    outHex8(buffer + 23, a);
    char *p = buffer + 29;
    *p++ = bit1(cc & 0x10, 'H');
    *p++ = bit1(cc & 0x08, 'I');
    *p++ = bit1(cc & 0x04, 'N');
    *p++ = bit1(cc & 0x02, 'Z');
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
    static const uint8_t SWI[2] = {0x83, 0xFF};  // SWI

    uint8_t bytes[8];
    if (show)
        Signals::resetCycles();
    Pins.captureWrites(SWI, sizeof(SWI), &sp, bytes, 5);
    // Capturing writes to stack in little endian order.
    pc = le16(bytes) - 1;  //  offset SWI instruction.
    // Injecting PC as SWI vector (with irrelevant read after).
    bytes[5] = hi(pc);
    bytes[6] = lo(pc);
    bytes[7] = 0;
    Pins.execInst(bytes + 5, 3);
    if (show)
        Signals::printCycles();

    x = bytes[2];
    a = bytes[3];
    cc = bytes[4];
}

void Regs::restore(bool show) {
    // Store registers into stack on internal RAM.
    sp -= 4;
    if (show)
        Signals::resetCycles();
    Memory.internal_write(sp++, cc);
    Memory.internal_write(sp++, a);
    Memory.internal_write(sp++, x);
    Memory.internal_write(sp++, hi(pc));
    Memory.internal_write(sp, lo(pc));
    // Restore registers
    static const uint8_t rti[9] = {0x80};
    Pins.execInst(rti, sizeof(rti));
    if (show)
        Signals::printCycles();
}

void Regs::printRegList() const {
    cli.println(F("?Reg: PC X A CC"));
}

char Regs::validUint8Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("A")) == 0)
        return 'a';
    if (strcasecmp_P(word, PSTR("X")) == 0)
        return 'x';
    if (strcasecmp_P(word, PSTR("CC")) == 0)
        return 'c';
    return 0;
}

char Regs::validUint16Reg(const char *word) const {
    if (strcasecmp_P(word, PSTR("PC")) == 0)
        return 'p';
    return 0;
}

void Regs::setRegValue(char reg, uint32_t value) {
    switch (reg) {
    case 'p':
        pc = value;
        break;
    case 'x':
        x = value;
        break;
    case 'a':
        a = value;
        break;
    case 'c':
        cc = value;
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

bool Memory::is_internal(uint16_t addr) {
    if (addr < 0x0A)  // Internal Peripherals
        return true;
    if (addr < 0x10)  // External Memory Space
        return false;
    if (addr < 0x80)  // Internal RAM
        return true;
    return false;  // External Memory Space
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

uint8_t Memory::internal_read(uint8_t addr) const {
    static uint8_t LDA_STA[6] = {
            0xB6, 0, 0, 0xB7, 0x10, 0};  // LDA dir[addr], STA $10
    LDA_STA[1] = addr;
    Pins.captureWrites(LDA_STA, sizeof(LDA_STA), nullptr, &LDA_STA[2], 1);
    return LDA_STA[2];
}

void Memory::internal_write(uint8_t addr, uint8_t data) const {
    static uint8_t LDA_STA[6] = {
            0xA6, 0, 0xB7, 0, 0, 0};  // LDA #val, STA dir[addr]
    LDA_STA[1] = data;
    LDA_STA[3] = addr;
    Pins.execInst(LDA_STA, sizeof(LDA_STA));
}

uint8_t Memory::raw_read(uint16_t addr) const {
    return addr < memory_size ? memory[addr] : 0;
}

void Memory::raw_write(uint16_t addr, uint8_t data) {
    if (addr < memory_size)
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
