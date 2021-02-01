/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
/*
  The Controller can accept commands represented by one letter.

  R - reset MPU.
  p - print MPU hardware signal status.
  i - inject instruction.
  I - inject instruction with printing signal status.
  d - dump memory. addr [length]
  m - write memory. addr byte...
  s - step one instruction.
  S - step one instruction with printing signal status.
  r - print MPU registers.
  = - set MPU register. register value
  c - continuously run with printing register.
  G - go, continuously run MPU.
  h - halt MPU.
  D - disassemble
  A - assemble
  F - list files in SD card
  L - load S-record file.
  ? - print version.
*/

#include "commands.h"

#include <Arduino.h>
#include <SD.h>
#include <asm_mc6809.h>
#include <dis_mc6809.h>
#include <dis_memory.h>
#include <libcli.h>
#include <string.h>

#include "config.h"
#include "pins.h"
#include "regs.h"

using namespace libasm;
using namespace libasm::mc6809;

#define USAGE                                                                  \
    F("R:eset r:egs =:setReg d:ump D:iasm m:emory i/I:nst A:sm s/S:tep c:ont " \
      "G:o h/H:alt p:ins F:iles L:oad")

using libcli::Cli;
extern class Cli Cli;

class Commands Commands;

static void commandHandler(char c, uintptr_t extra) {
    (void)extra;
    Commands.exec(c);
}

static void printPrompt() {
    Cli.print(F("> "));
    Cli.readLetter(commandHandler, 0);
}

static uint16_t last_addr;
#define DUMP_ADDR 0
#define DUMP_LENGTH 1
#define ASM_ADDR 0
#define DIS_ADDR 0
#define DIS_LENGTH 1
#define INST_DATA(c, i) ((uintptr_t)((c << 8) | i))

static uint8_t buffer[16];
#define MEMORY_ADDR static_cast<uintptr_t>(sizeof(buffer) + 10)
#define MEMORY_DATA(i) ((uintptr_t)(i))

static void writeMemory(uint16_t addr, uint8_t data) {
    const uint8_t lda[] = {0x86, data};
    const uint8_t sta[] = {0xB7, (uint8_t)(addr >> 8), (uint8_t)addr};
    Pins.execInst(lda, sizeof(lda));
    Pins.execInst(sta, sizeof(sta));
}

static uint8_t readMemory(uint16_t addr) {
    const uint8_t sta[] = {0xB6, (uint8_t)(addr >> 8), (uint8_t)addr};
    uint8_t data;
    Pins.captureReads(sta, sizeof(sta), &data, 1);
    return data;
}

static void handleInstruction(
        uint32_t value, uintptr_t extra, Cli::State state) {
    const char c = extra >> 8;
    uint16_t index = extra & 0xFF;
    if (state == Cli::State::CLI_DELETE) {
        if (index > 0) {
            index--;
            Cli.backspace();
            Cli.readHex8(handleInstruction, INST_DATA(c, index), buffer[index]);
        }
        return;
    }

    buffer[index++] = value;
    if (state == Cli::State::CLI_SPACE) {
        if (index < sizeof(buffer)) {
            Cli.readHex8(handleInstruction, INST_DATA(c, index));
            return;
        }
        Cli.println();
    }
    Pins.execInst(buffer, index, c == 'I');
    printPrompt();
}

static void memoryDump(uint16_t addr, uint16_t len) {
    Regs.save();
    for (uint16_t i = 0; i < len; i++, addr++) {
        const uint8_t data = readMemory(addr);
        if (i % 16 == 0) {
            if (i)
                Cli.println();
            Cli.printHex16(addr);
            Cli.print(':');
        }
        Cli.printHex8(data);
        Cli.print(' ');
    }
    Cli.println();
    Regs.restore();
}

static void handleDump(uint32_t value, uintptr_t extra, Cli::State state) {
    if (state == Cli::State::CLI_DELETE) {
        if (extra == DUMP_LENGTH) {
            Cli.backspace();
            Cli.readHex16(handleDump, DUMP_ADDR, last_addr);
        }
        return;
    }
    if (extra == DUMP_ADDR) {
        last_addr = value;
        if (state == Cli::State::CLI_SPACE) {
            Cli.readHex16(handleDump, DUMP_LENGTH);
            return;
        }
        value = 16;
    }
    memoryDump(last_addr, value);
    last_addr += value;
    printPrompt();
}

class Mc6809Memory : public DisMemory {
public:
    Mc6809Memory() : DisMemory(0) {}
    bool hasNext() const override { return true; }
    void setAddress(uint16_t addr) { resetAddress(addr); }

protected:
    uint8_t nextByte() {
        Regs.save();
        const uint8_t data = readMemory(address());
        Regs.restore();
        return data;
    }
};

static void print(const char *text, int width) {
    Cli.print(text);
    for (int i = strlen(text); i < width; i++) {
        Cli.print(' ');
    }
}

static void print(const Insn &insn) {
    Cli.printHex16(insn.address());
    Cli.print(':');
    for (int i = 0; i < insn.length(); i++) {
        Cli.printHex8(insn.bytes()[i]);
        Cli.print(' ');
    }
    for (int i = insn.length(); i < 5; i++) {
        Cli.print(F("   "));
    }
}

static uint16_t disassemble(uint16_t addr, uint16_t max) {
    DisMc6809 dis6809;
    Disassembler &disassembler(dis6809);
    disassembler.setCpu(Regs.cpu());
    disassembler.setUppercase(true);
    class Mc6809Memory memory;
    memory.setAddress(addr);
    uint16_t len = 0;
    while (len < max) {
        char operands[20];
        Insn insn;
        disassembler.decode(memory, insn, operands, nullptr);
        len += insn.length();
        print(insn);
        if (disassembler.getError()) {
            Cli.print(F("Error: "));
            Cli.println(disassembler.getError());
            continue;
        }
        print(insn.name(), 6);
        print(operands, 12);
        Cli.println();
    }
    return addr + len;
}

static void handleDisassemble(
        uint32_t value, uintptr_t extra, Cli::State state) {
    if (state == Cli::State::CLI_DELETE) {
        if (extra == DIS_LENGTH) {
            Cli.backspace();
            Cli.readHex16(handleDisassemble, DIS_ADDR, last_addr);
        }
        return;
    }
    if (extra == DIS_ADDR) {
        last_addr = value;
        if (state == Cli::State::CLI_SPACE) {
            Cli.readHex16(handleDisassemble, DIS_LENGTH);
            return;
        }
        value = 16;
    }
    last_addr = disassemble(last_addr, value);
    printPrompt();
}

static void memoryWrite(
        uint16_t addr, const uint8_t values[], const uint8_t len) {
    Regs.save();
    for (uint8_t i = 0; i < len; i++, addr++) {
        writeMemory(addr, values[i]);
    }
    Regs.restore();
}

static void handleMemory(uint32_t value, uintptr_t extra, Cli::State state) {
    if (extra == MEMORY_ADDR) {
        last_addr = value;
        Cli.readHex8(handleMemory, MEMORY_DATA(0));
        return;
    }
    uint16_t index = extra;
    if (state == Cli::State::CLI_DELETE) {
        Cli.backspace();
        if (index == 0) {
            Cli.readHex16(handleMemory, MEMORY_ADDR, last_addr);
        } else {
            index--;
            Cli.readHex8(handleMemory, MEMORY_DATA(index), buffer[index]);
        }
        return;
    }
    buffer[index++] = value;
    if (state == Cli::State::CLI_SPACE) {
        if (index < sizeof(buffer)) {
            Cli.readHex8(handleMemory, MEMORY_DATA(index));
            return;
        }
        Cli.println();
    }
    memoryWrite(last_addr, buffer, index);
    memoryDump(last_addr, index);
    last_addr += index;
    printPrompt();
}

static void handleAssembleLine(char *line, uintptr_t extra, Cli::State state) {
    (void)state;
    (void)extra;
    if (*line == 0) {
        Cli.println(F("end"));
        printPrompt();
        return;
    }
    AsmMc6809 as6809;
    Assembler &assembler(as6809);
    assembler.setCpu(Regs.cpu());
    Insn insn;
    if (assembler.encode(line, insn, last_addr, nullptr)) {
        Cli.print(F("Error: "));
        Cli.println(assembler.getError());
    } else {
        print(insn);
        Cli.println();
        memoryWrite(insn.address(), insn.bytes(), insn.length());
        last_addr += insn.length();
    }
    Cli.printHex16(last_addr);
    Cli.print('?');
    Cli.readString(handleAssembleLine, 0);
}

static void handleAssembler(uint32_t value, uintptr_t extra, Cli::State state) {
    if (extra == ASM_ADDR) {
        last_addr = value;
        if (state == Cli::State::CLI_NEWLINE) {
            Cli.printHex16(last_addr);
            Cli.print('?');
            Cli.readString(handleAssembleLine, 0);
        }
    }
}

static void handleFileListing() {
    SD.begin(Pins.sdCardChipSelectPin());
    File root = SD.open("/");
    while (true) {
        File entry = root.openNextFile();
        if (!entry)
            break;
        if (!entry.isDirectory()) {
            Cli.print(entry.name());
            Cli.print('\t');
            Cli.println(entry.size());
        }
        entry.close();
    }
    root.close();
}

static uint8_t toInt(const char c) {
    return isDigit(c) ? c - '0' : c - 'A' + 10;
}

static uint8_t toInt8Hex(const char *text) {
    return (toInt(text[0]) << 4) | toInt(text[1]);
}

static uint16_t toInt16Hex(const char *text) {
    return ((uint16_t)toInt8Hex(text) << 8) | toInt8Hex(text + 2);
}

static int loadS19Record(const char *line) {
    int len = strlen(line);
    if (len == 0 || line[0] != 'S' || line[1] != '1')
        return 0;
    const int num = toInt8Hex(line + 2) - 3;
    const uint16_t addr = toInt16Hex(line + 4);
    uint8_t buffer[num];
    for (int i = 0; i < num; i++) {
        buffer[i] = toInt8Hex(line + i * 2 + 8);
    }
    memoryWrite(addr, buffer, num);
    Cli.printHex16(addr);
    Cli.print(':');
    Cli.printHex8(num);
    Cli.print(' ');
    return num;
}

static void handleLoadFile(char *line, uintptr_t extra, Cli::State state) {
    (void)extra;
    if (state != Cli::State::CLI_NEWLINE)
        return;
    uint16_t size = 0;
    SD.begin(Pins.sdCardChipSelectPin());
    File file = SD.open(line);
    if (file) {
        char s19[80];
        char *p = s19;
        while (file.available() > 0) {
            const char c = file.read();
            if (c == '\n') {
                *p = 0;
                size += loadS19Record(s19);
                p = s19;
            } else if (c != '\r' && p < s19 + sizeof(s19) - 1) {
                *p++ = c;
            }
        }
        file.close();
    }
    Cli.println();
    Cli.print(size);
    Cli.println(F(" bytes loaded"));
    printPrompt();
}

static void handleRegisterValue(uint32_t, uintptr_t, Cli::State);

static void handleSetRegister(char value, uintptr_t extra) {
    (void)extra;
    const char c = value;
    if (c == 'p' || c == 's' || c == 'u' || c == 'y' || c == 'x' || c == 'd' ||
            (Regs.is6309() && (c == 'w' || c == 'v'))) {
        Cli.print(c);
        Cli.print('?');
        Cli.readHex16(handleRegisterValue, (uintptr_t)c);
        return;
    }
    if (c == 'D' || c == 'a' || c == 'b' || c == 'c' ||
            (Regs.is6309() && (c == 'e' || c == 'f'))) {
        Cli.print(c);
        Cli.print('?');
        Cli.readHex8(handleRegisterValue, (uintptr_t)c);
        return;
    }
    Cli.print(F("?Reg: pc s u x y a b d"));
    if (Regs.is6309())
        Cli.print(F(" w e f v"));
    Cli.println(F(" Dp cc"));
    printPrompt();
}

static void handleRegisterValue(
        uint32_t value, uintptr_t extra, Cli::State state) {
    if (state == Cli::State::CLI_DELETE) {
        Cli.backspace(2);
        Cli.readLetter(handleSetRegister, 0);
        return;
    }
    if (state == Cli::State::CLI_SPACE)
        Cli.println();
    const char reg = extra;
    switch (reg) {
    case 'p':
        Regs.pc = value;
        break;
    case 's':
        Regs.s = value;
        break;
    case 'u':
        Regs.u = value;
        break;
    case 'y':
        Regs.y = value;
        break;
    case 'x':
        Regs.x = value;
        break;
    case 'd':
        Regs.d = value;
        break;
    case 'w':
        Regs.w = value;
        break;
    case 'v':
        Regs.v = value;
        break;
    case 'a':
        Regs.a = value;
        break;
    case 'b':
        Regs.b = value;
        break;
    case 'e':
        Regs.e = value;
        break;
    case 'f':
        Regs.f = value;
        break;
    case 'D':
        Regs.dp = value;
        break;
    case 'c':
        Regs.cc = value;
        break;
    }
    Regs.restore();
    Regs.print();
    printPrompt();
}

void Commands::exec(char c) {
    switch (c) {
    case 'p':
        Cli.print(F("pins:"));
        Pins.print();
        break;
    case 'R':
        Cli.println(F("RESET"));
        _target = HALT;
        Pins.reset(true);
        Regs.get(true);
        disassemble(Regs.pc, 1);
        break;
    case 'i':
    case 'I':
        Cli.print(F("inst? "));
        Cli.readHex8(handleInstruction, INST_DATA(c, 0));
        return;
    case 'd':
        Cli.print(F("dump? "));
        Cli.readHex16(handleDump, DUMP_ADDR);
        return;
    case 'D':
        Cli.print(F("Dis? "));
        Cli.readHex16(handleDisassemble, DIS_ADDR);
        return;
    case 'A':
        Cli.print(F("Asm? "));
        Cli.readHex16(handleAssembler, ASM_ADDR);
        return;
    case 'm':
        Cli.print(F("mem? "));
        Cli.readHex16(handleMemory, MEMORY_ADDR);
        return;
    case 's':
    case 'S':
        if (_target == HALT) {
            if (c == 's')
                Cli.print(F("step: "));
            else
                Cli.println(F("Step"));
            Pins.step(c == 'S');
        } else {
            _target = HALT;
            Pins.halt(c == 'S');
        }
        Regs.get(true);
        disassemble(Regs.pc, 1);
        break;
    case 'r':
        Cli.print(F("regs: "));
        Regs.get(true);
        break;
    case '=':
        Cli.print(F("set reg? "));
        Cli.readLetter(handleSetRegister, 0);
        return;
    case 'c':
        _target = STEP;
        return;
    case 'G':
        if (_target != RUN) {
            Cli.println(F("GO"));
            _target = RUN;
            Pins.run();
        }
        return;
    case 'h':
    case 'H':
        if (_target != HALT) {
            _target = HALT;
            Pins.halt(c == 'H');
            Cli.println(F("HALT"));
            Regs.get(true);
            disassemble(Regs.pc, 1);
        }
        break;
    case 'F':
        Cli.println(F("Files"));
        handleFileListing();
        break;
    case 'L':
        Cli.print(F("Load? "));
        Cli.readString(handleLoadFile, 0);
        return;
    case '?':
        Cli.println(VERSION_TEXT);
        Cli.println(USAGE);
        break;
    case '\r':
        Cli.println();
    case '\n':
        break;
    default:
        return;
    }
    printPrompt();
}

void Commands::begin() {
    printPrompt();
    _target = HALT;
}

void Commands::loop() {
    if (_target == STEP) {
        Pins.step();
        Regs.get(true);
    }
}
