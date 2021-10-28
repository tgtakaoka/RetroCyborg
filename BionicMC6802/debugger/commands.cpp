/*
  The Controller can accept commands represented by one letter.

  R - reset MPU.
  d - dump memory. addr [length]
  m - write memory. addr byte...
  s - step one instruction.
  S - step one instruction with printing bus cycles.
  r - print MPU registers.
  = - set MPU register. register value
  c - continuously run with printing register.
  G - Go, continuously run MPU.
  h - halt MPU.
  H - halt MPU with printing bus cycles.
  D - disassemble
  A - assemble
  F - list files in SD card
  L - load S-record file.
  ? - print version.
*/

#include "commands.h"

#include <SD.h>
#include <asm_mc6800.h>
#include <dis_mc6800.h>
#include <libcli.h>
#include <string.h>

#include "config.h"
#include "pins.h"
#include "regs.h"

using namespace libasm;

typedef libcli::Cli::State State;
extern libcli::Cli &cli;

#define USAGE                                                          \
    "R:eset r:egs =:setReg d:ump D:is m:emory A:sm s/S:tep c:ont G:o " \
    "h/H:alt F:iles L:oad I:o"

class Commands Commands;

static void commandHandler(char c, uintptr_t extra) {
    (void)extra;
    Commands.exec(c);
}

static void printPrompt() {
    cli.print("> ");
    cli.readLetter(commandHandler, 0);
}

static uint16_t last_addr;
#define DUMP_ADDR 0
#define DUMP_LENGTH 1
#define DIS_ADDR 0
#define DIS_LENGTH 1

static uint8_t mem_buffer[16];
#define MEMORY_ADDR static_cast<uintptr_t>(sizeof(mem_buffer) + 10)
#define MEMORY_DATA(i) ((uintptr_t)(i))

static char str_buffer[40];

extern uint8_t RAM[0x10000];

static void writeMemory(uint16_t addr, uint8_t data) {
    RAM[addr] = data;
}

static uint8_t readMemory(uint16_t addr) {
    return RAM[addr];
}

static void memoryDump(uint16_t addr, uint16_t len) {
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
}

static void handleDump(uint32_t value, uintptr_t extra, State state) {
    if (state == State::CLI_CANCEL)
        goto cancel;
    if (state == State::CLI_DELETE) {
        if (extra == DUMP_LENGTH) {
            cli.backspace();
            cli.readHex(handleDump, DUMP_ADDR, UINT16_MAX, last_addr);
        }
        return;
    }
    if (extra == DUMP_ADDR) {
        last_addr = value;
        if (state == State::CLI_SPACE) {
            cli.readHex(handleDump, DUMP_LENGTH, UINT16_MAX);
            return;
        }
        value = 16;
    }
    cli.println();
    memoryDump(last_addr, value);
    last_addr += value;
cancel:
    printPrompt();
}

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

static uint16_t disassemble(uint16_t addr, uint16_t numInsn) {
    mc6800::DisMc6800 dis6800;
    Disassembler &disassembler = dis6800;
    disassembler.setCpu(Regs.cpu());
    disassembler.setUppercase(true);
    uint16_t num = 0;
    while (num < numInsn) {
        ArrayMemory memory(addr, &RAM[addr], disassembler.config().codeMax());
        char operands[20];
        Insn insn(addr);
        disassembler.decode(memory, insn, operands, sizeof(operands));
        addr += insn.length();
        num++;
        print(insn);
        if (disassembler.getError()) {
            cli.print(F("Error: "));
            cli.println(disassembler.errorText(disassembler.getError()));
            continue;
        }
        print(insn.name(), 6);
        print(operands, 12);
        cli.println();
    }
    return addr;
}

static void handleDisassemble(uint32_t value, uintptr_t extra, State state) {
    if (state == State::CLI_CANCEL)
        goto cancel;
    if (state == State::CLI_DELETE) {
        if (extra == DIS_LENGTH) {
            cli.backspace();
            cli.readHex(handleDisassemble, DIS_ADDR, UINT16_MAX, last_addr);
        }
        return;
    }
    if (extra == DIS_ADDR) {
        last_addr = value;
        if (state == State::CLI_SPACE) {
            cli.readDec(handleDisassemble, DIS_LENGTH, UINT16_MAX);
            return;
        }
        value = 10;
        ;
    }
    cli.println();
    last_addr = disassemble(last_addr, value);
cancel:
    printPrompt();
}

static void memoryWrite(
        uint16_t addr, const uint8_t values[], const uint8_t len) {
    for (uint8_t i = 0; i < len; i++, addr++) {
        writeMemory(addr, values[i]);
    }
}

static void handleMemory(uint32_t value, uintptr_t extra, State state) {
    if (state != State::CLI_CANCEL) {
        if (extra == MEMORY_ADDR) {
            last_addr = value;
            cli.readHex(handleMemory, MEMORY_DATA(0), UINT8_MAX);
            return;
        }
        uint16_t index = extra;
        if (state == State::CLI_DELETE) {
            cli.backspace();
            if (index == 0) {
                cli.readHex(handleMemory, MEMORY_ADDR, UINT16_MAX, last_addr);
            } else {
                index--;
                cli.readHex(handleMemory, MEMORY_DATA(index), UINT8_MAX,
                        mem_buffer[index]);
            }
            return;
        }
        mem_buffer[index++] = value;
        if (state == State::CLI_SPACE) {
            if (index < sizeof(mem_buffer)) {
                cli.readHex(handleMemory, MEMORY_DATA(index), UINT8_MAX);
                return;
            }
        }
        cli.println();
        memoryWrite(last_addr, mem_buffer, index);
        memoryDump(last_addr, index);
        last_addr += index;
    }
    printPrompt();
}

static void handleAssembleLine(char *line, uintptr_t extra, State state) {
    (void)extra;
    if (state == State::CLI_CANCEL || *line == 0) {
        cli.println(F("end"));
        printPrompt();
        return;
    }
    cli.println();
    mc6800::AsmMc6800 as6800;
    Assembler &assembler(as6800);
    assembler.setCpu(Regs.cpu());
    Insn insn(last_addr);
    if (assembler.encode(line, insn)) {
        cli.print(F("Error: "));
        cli.println(assembler.errorText(assembler.getError()));
    } else {
        print(insn);
        cli.println();
        memoryWrite(insn.address(), insn.bytes(), insn.length());
        last_addr += insn.length();
    }
    cli.printHex(last_addr, 4);
    cli.print('?');
    cli.readLine(handleAssembleLine, 0, str_buffer, sizeof(str_buffer));
}

static void handleAssembler(uint32_t value, uintptr_t extra, State state) {
    if (state == State::CLI_CANCEL) {
        printPrompt();
        return;
    }
    last_addr = value;
    cli.println();
    cli.printHex(last_addr, 4);
    cli.print('?');
    cli.readLine(handleAssembleLine, 0, str_buffer, sizeof(str_buffer));
}

static void handleFileListing() {
    SD.begin(BUILTIN_SDCARD);
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

static void handleLoadFile(char *line, uintptr_t extra, State state) {
    (void)extra;
    if (state != State::CLI_CANCEL && *line) {
        cli.println();
        SD.begin(BUILTIN_SDCARD);
        File file = SD.open(line);
        if (!file) {
            cli.print(line);
            cli.println(F(" not found"));
        } else {
            uint16_t size = 0;
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
            cli.println();
            cli.print(size);
            cli.println(F(" bytes loaded"));
        }
    }
    printPrompt();
}

static void handleRegisterValue(uint32_t, uintptr_t, State);

static void handleSetRegister(char reg, uintptr_t extra) {
    (void)extra;
    if (Regs.validUint16Reg(reg)) {
        cli.print('?');
        cli.readHex(handleRegisterValue, (uintptr_t)reg, UINT16_MAX);
        return;
    }
    if (Regs.validUint8Reg(reg)) {
        cli.print('?');
        cli.readHex(handleRegisterValue, (uintptr_t)reg, UINT8_MAX);
        return;
    }
    Regs.printRegList();
    printPrompt();
}

static void handleRegisterValue(uint32_t value, uintptr_t reg, State state) {
    if (Regs.setRegValue(reg, value, state)) {
        printPrompt();
    } else {
        cli.readLetter(handleSetRegister, 0);
    }
}

static void handleIo(char *line, uintptr_t extra, State state);

static void printIoDevice(State state) {
    cli.println();
    uint16_t baseAddr;
    if (Pins.getIoDevice(baseAddr) == Pins::SerialDevice::DEV_ACIA) {
        cli.print(F("ACIA (MC6850) at $"));
    }
    cli.printHex(baseAddr, 4);
    cli.println();
}

static void handleAciaAddr(uint32_t val, uintptr_t extra, State state) {
    if (state == State::CLI_CANCEL)
        goto cancel;
    if (state == State::CLI_DELETE) {
        cli.backspace();
        strcpy_P(str_buffer, PSTR("ACIA"));
        cli.readWord(handleIo, 0, str_buffer, sizeof(str_buffer), true);
        return;
    }
    Pins.setIoDevice(Pins::SerialDevice::DEV_ACIA, val);
    printIoDevice(state);
cancel:
    printPrompt();
}

static void handleIo(char *line, uintptr_t extra, State state) {
    if (state == State::CLI_CANCEL)
        goto cancel;
    if (state == State::CLI_DELETE) {
        cli.backspace();
        cli.readWord(handleIo, 0, str_buffer, sizeof(str_buffer));
        return;
    }
    if (state == State::CLI_SPACE &&
            strcasecmp_P(str_buffer, PSTR("acia")) == 0) {
        cli.readHex(handleAciaAddr, 0, UINT16_MAX);
        return;
    }
    if (strcasecmp_P(str_buffer, PSTR("acia")) == 0) {
        Pins.setIoDevice(Pins::SerialDevice::DEV_ACIA, Pins.ioBaseAddress());
    }
    printIoDevice(state);
cancel:
    printPrompt();
}

void Commands::exec(char c) {
    switch (c) {
    case 'R':
        cli.println("RESET");
        Pins.reset(true);
        Regs.print();
        disassemble(Regs.pc, 1);
        break;
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
        cli.readHex(handleAssembler, 0, UINT16_MAX);
        return;
    case 'm':
        cli.print(F("mem? "));
        cli.readHex(handleMemory, MEMORY_ADDR, UINT16_MAX);
        return;
    case 'r':
        cli.print(F("regs: "));
        Regs.print();
        disassemble(Regs.pc, 1);
        break;
    case '=':
        cli.print(F("set reg? "));
        cli.readLetter(handleSetRegister, 0);
        return;
    case 'S':
    case 's':
        if (_target != HALT) {
            halt(c == 'S');
            return;
        }
        cli.println("STEP");
        Pins.step(c == 'S');
        Regs.print();
        disassemble(Regs.pc, 1);
        break;
    case 'H':
    case 'h':
        if (_target != HALT) {
            halt(c == 'H');
            return;
        }
        break;
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
    case 'F':
        cli.println(F("Files"));
        handleFileListing();
        break;
    case 'L':
        cli.print(F("Load? "));
        cli.readLine(handleLoadFile, 0, str_buffer, sizeof(str_buffer));
        return;
    case 'I':
        cli.print(F("Io? "));
        cli.readWord(handleIo, 0, str_buffer, sizeof(str_buffer));
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

void Commands::halt(bool show) {
    _target = HALT;
    Pins.halt(show);
    Regs.print();
    disassemble(Regs.pc, 1);
    printPrompt();
}

void Commands::begin() {
    printPrompt();
    _target = HALT;
}

void Commands::loop() {
    if (_target == STEP) {
        Pins.step();
        Regs.print();
    } else {
        Pins.loop();
    }
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
