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

extern libcli::Cli cli;
using libcli::State;

class Commands Commands;

static void commandHandler(char c, uintptr_t extra) {
    (void)extra;
    Commands.exec(c);
}

static void printPrompt() {
    cli.print(F("> "));
    cli.readLetter(commandHandler, 0);
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

static void handleInstruction(uint32_t value, uintptr_t context, State state) {
    const char c = context >> 8;
    uint16_t index = context & 0xFF;
    if (state == State::CLI_DELETE) {
        if (index > 0) {
            index--;
            cli.backspace();
            cli.readHex(handleInstruction, INST_DATA(c, index), UINT8_MAX,
                    buffer[index]);
        }
        return;
    }

    buffer[index++] = value;
    if (state == State::CLI_SPACE) {
        if (index < sizeof(buffer)) {
            cli.readHex(handleInstruction, INST_DATA(c, index), UINT8_MAX);
            return;
        }
        cli.println();
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
                cli.println();
            cli.printHex(addr, 4);
            cli.print(':');
        }
        cli.printHex(data, 2);
        cli.print(' ');
    }
    cli.println();
    Regs.restore();
}

static void handleDump(uint32_t value, uintptr_t context, State state) {
    if (state == State::CLI_DELETE) {
        if (context == DUMP_LENGTH) {
            cli.backspace();
            cli.readHex(handleDump, DUMP_ADDR, UINT16_MAX, last_addr);
        }
        return;
    }
    if (context == DUMP_ADDR) {
        last_addr = value;
        if (state == State::CLI_SPACE) {
            cli.readHex(handleDump, DUMP_LENGTH, UINT16_MAX);
            return;
        }
        value = 16;
    }
    memoryDump(last_addr, value);
    last_addr += value;
    printPrompt();
}

struct Mc6809Memory : DisMemory {
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
    cli.print(text);
    for (int i = strlen(text); i < width; i++) {
        cli.print(' ');
    }
}

static void print(const Insn &insn) {
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

static uint16_t disassemble(uint16_t addr, uint16_t max) {
    DisMc6809 dis6809;
    Disassembler &disassembler(dis6809);
    disassembler.setCpu(Regs.cpu());
    disassembler.setUppercase(true);
    Mc6809Memory memory;
    memory.setAddress(addr);
    uint16_t len = 0;
    while (len < max) {
        char operands[20];
        Insn insn{addr};
        disassembler.decode(memory, insn, operands, sizeof(buffer), nullptr);
        len += insn.length();
        print(insn);
        if (disassembler.getError()) {
            cli.print(F("Error: "));
            cli.println(disassembler.getError());
            continue;
        }
        print(insn.name(), 6);
        print(operands, 12);
        cli.println();
    }
    return addr + len;
}

static void handleDisassemble(uint32_t value, uintptr_t context, State state) {
    if (state == State::CLI_DELETE) {
        if (context == DIS_LENGTH) {
            cli.backspace();
            cli.readHex(handleDisassemble, DIS_ADDR, UINT16_MAX, last_addr);
        }
        return;
    }
    if (context == DIS_ADDR) {
        last_addr = value;
        if (state == State::CLI_SPACE) {
            cli.readHex(handleDisassemble, DIS_LENGTH, UINT16_MAX);
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

static void handleMemory(uint32_t value, uintptr_t context, State state) {
    if (context == MEMORY_ADDR) {
        last_addr = value;
        cli.readHex(handleMemory, MEMORY_DATA(0), UINT8_MAX);
        return;
    }
    uint16_t index = context;
    if (state == State::CLI_DELETE) {
        cli.backspace();
        if (index == 0) {
            cli.readHex(handleMemory, MEMORY_ADDR, UINT16_MAX, last_addr);
        } else {
            index--;
            cli.readHex(
                    handleMemory, MEMORY_DATA(index), UINT8_MAX, buffer[index]);
        }
        return;
    }
    buffer[index++] = value;
    if (state == State::CLI_SPACE) {
        if (index < sizeof(buffer)) {
            cli.readHex(handleMemory, MEMORY_DATA(index), UINT8_MAX);
            return;
        }
        cli.println();
    }
    memoryWrite(last_addr, buffer, index);
    memoryDump(last_addr, index);
    last_addr += index;
    printPrompt();
}

static char line[128];

static void handleAssembleLine(char *line, uintptr_t context, State state) {
    (void)state;
    (void)context;
    if (*line == 0) {
        cli.println(F("end"));
        printPrompt();
        return;
    }
    AsmMc6809 as6809;
    Assembler &assembler(as6809);
    assembler.setCpu(Regs.cpu());
    Insn insn{last_addr};
    if (assembler.encode(line, insn, nullptr)) {
        cli.print(F("Error: "));
        cli.println(assembler.getError());
    } else {
        print(insn);
        cli.println();
        memoryWrite(insn.address(), insn.bytes(), insn.length());
        last_addr += insn.length();
    }
    cli.printHex(last_addr, 4);
    cli.print('?');
    cli.readLine(handleAssembleLine, 0, line, sizeof(line));
}

static void handleAssembler(uint32_t value, uintptr_t context, State state) {
    if (context == ASM_ADDR) {
        last_addr = value;
        if (state == State::CLI_NEWLINE) {
            cli.printHex(last_addr, 4);
            cli.print('?');
            cli.readLine(handleAssembleLine, 0, line, sizeof(line));
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
            cli.print(entry.name());
            cli.print('\t');
            cli.println(entry.size());
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
    cli.printHex(addr, 4);
    cli.print(':');
    cli.printHex(num, 2);
    cli.print(' ');
    return num;
}

static void handleLoadFile(char *line, uintptr_t context, State state) {
    (void)context;
    if (state != State::CLI_NEWLINE)
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
    cli.println();
    cli.print(size);
    cli.println(F(" bytes loaded"));
    printPrompt();
}

static void handleRegisterValue(uint32_t, uintptr_t, State);

static void handleSetRegister(char value, uintptr_t context) {
    (void)context;
    const char c = value;
    if (c == 'p' || c == 's' || c == 'u' || c == 'y' || c == 'x' || c == 'd' ||
            (Regs.is6309() && (c == 'w' || c == 'v'))) {
        cli.print(c);
        cli.print('?');
        cli.readHex(handleRegisterValue, (uintptr_t)c, UINT16_MAX);
        return;
    }
    if (c == 'D' || c == 'a' || c == 'b' || c == 'c' ||
            (Regs.is6309() && (c == 'e' || c == 'f'))) {
        cli.print(c);
        cli.print('?');
        cli.readHex(handleRegisterValue, (uintptr_t)c, UINT8_MAX);
        return;
    }
    cli.print(F("?Reg: pc s u x y a b d"));
    if (Regs.is6309())
        cli.print(F(" w e f v"));
    cli.println(F(" Dp cc"));
    printPrompt();
}

static void handleRegisterValue(
        uint32_t value, uintptr_t context, State state) {
    if (state == State::CLI_DELETE) {
        cli.backspace(2);
        cli.readLetter(handleSetRegister, 0);
        return;
    }
    if (state == State::CLI_SPACE)
        cli.println();
    const char reg = context;
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
        cli.print(F("pins:"));
        Pins.print();
        break;
    case 'R':
        cli.println(F("RESET"));
        _target = HALT;
        Pins.reset(true);
        Regs.get(true);
        disassemble(Regs.pc, 1);
        break;
    case 'i':
    case 'I':
        cli.print(F("inst? "));
        cli.readHex(handleInstruction, INST_DATA(c, 0), UINT8_MAX);
        return;
    case 'd':
        cli.print(F("dump? "));
        cli.readHex(handleDump, DUMP_ADDR, UINT16_MAX);
        return;
    case 'D':
        cli.print(F("Dis? "));
        cli.readHex(handleDisassemble, DIS_ADDR, UINT16_MAX);
        return;
    case 'A':
        cli.print(F("Asm? "));
        cli.readHex(handleAssembler, ASM_ADDR, UINT16_MAX);
        return;
    case 'm':
        cli.print(F("mem? "));
        cli.readHex(handleMemory, MEMORY_ADDR, UINT16_MAX);
        return;
    case 's':
    case 'S':
        if (_target == HALT) {
            if (c == 's')
                cli.print(F("step: "));
            else
                cli.println(F("Step"));
            Pins.step(c == 'S');
        } else {
            _target = HALT;
            Pins.halt(c == 'S');
        }
        Regs.get(true);
        disassemble(Regs.pc, 1);
        break;
    case 'r':
        cli.print(F("regs: "));
        Regs.get(true);
        break;
    case '=':
        cli.print(F("set reg? "));
        cli.readLetter(handleSetRegister, 0);
        return;
    case 'c':
        _target = STEP;
        return;
    case 'G':
        if (_target != RUN) {
            cli.println(F("GO"));
            _target = RUN;
            Pins.run();
        }
        return;
    case 'h':
    case 'H':
        if (_target != HALT) {
            _target = HALT;
            Pins.halt(c == 'H');
            cli.println(F("HALT"));
            Regs.get(true);
            disassemble(Regs.pc, 1);
        }
        break;
    case 'F':
        cli.println(F("Files"));
        handleFileListing();
        break;
    case 'L':
        cli.print(F("Load? "));
        cli.readLine(handleLoadFile, 0, line, sizeof(line));
        return;
    case '?':
        cli.println(VERSION_TEXT);
        cli.println(USAGE);
        break;
    case '\r':
        cli.println();
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
