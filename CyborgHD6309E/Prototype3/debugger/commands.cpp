/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
/*
  The Controller can accept commands represented by one letter.

  R - reset MPU.
  p - print MPU hardware signal status.
  i - inject instruction.
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

#include <Arduino.h>
#include <string.h>
#include <SD.h>
#include <asm_mc6809.h>
#include <dis_mc6809.h>
#include <dis_memory.h>
#include <symbol_table.h>

#include "commands.h"
#include "libcli.h"
#include "pins.h"
#include "regs.h"

#define VERSION F("* CyborgHD6309 Prototype3 2.0.0")
#define USAGE F("R:eset r:egs =:setReg d:ump D:iasm m:emory i:nst A:sm s/S:tep c:ont G:o h/H:alt p:ins F:iles L:oad")

class Commands Commands;

static uint16_t last_addr;
#define DUMP_ADDR   0
#define DUMP_LENGTH 1
#define ASM_ADDR    0
#define DIS_ADDR    0
#define DIS_LENGTH  1
#define INST_DATA(i) ((uintptr_t)(i))
#define REG_NAME    0

static uint8_t buffer[16];
#define MEMORY_ADDR static_cast<uintptr_t>(sizeof(buffer) + 10)
#define MEMORY_DATA(i) ((uintptr_t)(i))

static void execInst2(uint8_t inst, uint8_t opr, bool show = false) {
  const uint8_t insn[] = { inst, opr };
  Pins.execInst(insn, sizeof(insn), show);
}

static void execInst3(uint8_t inst, uint16_t opr, bool show = false) {
  const uint8_t insn[] = { inst, (uint8_t)(opr >> 8), (uint8_t)opr };
  Pins.execInst(insn, sizeof(insn), show);
}

static bool handleInstruction(Cli::State state, uint16_t value, uintptr_t extra) {
  uint16_t index = extra;
  if (state == Cli::State::CLI_DELETE) {
    if (index > 0) {
      index--;
      Cli.backspace();
      Cli.readUint8(handleInstruction, INST_DATA(index), buffer[index]);
    }
    return false;
  }

  buffer[index++] = value;
  if (state == Cli::State::CLI_SPACE) {
    if (index < sizeof(buffer))
      return Cli.readUint8(handleInstruction, INST_DATA(index));
    Cli.println();
  }
  Pins.execInst(buffer, index, true /* show */);
  return true;
}

static void memoryDump(uint16_t addr, uint16_t len) {
  Regs.save();
  for (uint16_t i = 0; i < len; i++, addr++) {
    execInst3(0xB6, addr); // LDA $addr
    if (i % 16 == 0) {
      if (i) Cli.println();
      Cli.printUint16(addr);
      Cli.print(':');
    }
    Cli.printUint8(Pins.dbus());
    Cli.print(' ');
  }
  Cli.println();
  Regs.restore();
}

static bool handleDump(Cli::State state, uint16_t value, uintptr_t extra) {
  if (state == Cli::State::CLI_DELETE) {
    if (extra == DUMP_LENGTH) {
      Cli.backspace();
      Cli.readUint16(handleDump, DUMP_ADDR, last_addr);
    }
    return false;
  }
  if (extra == DUMP_ADDR) {
    last_addr = value;
    if (state == Cli::State::CLI_SPACE)
      return Cli.readUint16(handleDump, DUMP_LENGTH);
    value = 16;
  }
  memoryDump(last_addr, value);
  last_addr += value;
  return true;
}


class Mc6809Memory : public DisMemory<target::uintptr_t> {
public:
  Mc6809Memory() : DisMemory(0) {}
  bool hasNext() const override {
    return true;
  }
  void setAddress(target::uintptr_t addr) {
    _address = addr;
  }
protected:
  uint8_t nextByte() {
    Regs.save();
    execInst3(0xB6, _address); // LDA $_address
    uint8_t data = Pins.dbus();
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

static void print(const Insn& insn) {
  Cli.printUint16(insn.address());
  Cli.print(':');
  for (int i = 0; i < insn.insnLen(); i++) {
    Cli.printUint8(insn.bytes()[i]);
    Cli.print(' ');
  }
  for (int i = insn.insnLen(); i < 5; i++) {
    Cli.print(F("   "));
  }
}

static uint16_t disassemble(uint16_t addr, uint16_t max) {
  DisMc6809 dis6809;
  Disassembler<uint16_t> &disassembler(dis6809);
  disassembler.setCpu("6309");
  class Mc6809Memory memory;
  memory.setAddress(addr);
  uint16_t len = 0;
  while (len < max) {
    char operands[20];
    Insn insn;
    disassembler.decode(memory, insn, operands, nullptr, true);
    len += insn.insnLen();
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

static bool handleDisassemble(Cli::State state , uint16_t value, uintptr_t extra) {
  if (state == Cli::State::CLI_DELETE) {
    if (extra == DIS_LENGTH) {
      Cli.backspace();
      Cli.readUint16(handleDisassemble, DIS_ADDR, last_addr);
    }
    return false;
  }
  if (extra == DIS_ADDR) {
    last_addr = value;
    if (state == Cli::State::CLI_SPACE)
      return Cli.readUint16(handleDisassemble, DIS_LENGTH);
    value = 16;
  }
  last_addr = disassemble(last_addr, value);
  return true;
}

static void memoryWrite(uint16_t addr, const uint8_t values[], const uint8_t len) {
  Regs.save();
  for (uint8_t i = 0; i < len; i++, addr++) {
    execInst2(0x86, values[i]); // LDA #values[i]
    execInst3(0xB7, addr);      // STA $addr
  }
  Regs.restore();
}

static bool handleMemory(Cli::State state, uint16_t value, uintptr_t extra) {
  if (extra == MEMORY_ADDR) {
    last_addr = value;
    return Cli.readUint8(handleMemory, MEMORY_DATA(0));
  }
  uint16_t index = extra;
  if (state == Cli::State::CLI_DELETE) {
    Cli.backspace();
    if (index == 0)
      return Cli.readUint16(handleMemory, MEMORY_ADDR, last_addr);
    index--;
    return Cli.readUint8(handleMemory, MEMORY_DATA(index), buffer[index]);
  }
  buffer[index++] = value;
  if (state == Cli::State::CLI_SPACE) {
    if (index < sizeof(buffer))
      return Cli.readUint8(handleMemory, MEMORY_DATA(index));
    Cli.println();
  }
  memoryWrite(last_addr, buffer, index);
  memoryDump(last_addr, index);
  last_addr += index;
  return true;
}

static bool handleAssembleLine(Cli::State state, char *line, uintptr_t extra) {
  if (*line == 0) {
    Cli.println(F("end"));
    return true;
  }
  AsmMc6809 as6809;
  Assembler<uint16_t> &assembler(as6809);
  assembler.setCpu("6309");
  Insn insn;
  if (assembler.encode(line, insn, last_addr, nullptr)) {
    Cli.print(F("Error: "));
    Cli.println(assembler.getError());
  } else {
    print(insn);
    Cli.println();
    memoryWrite(insn.address(), insn.bytes(), insn.insnLen());
    last_addr += insn.insnLen();
  }
  Cli.printUint16(last_addr);
  Cli.print('?');
  return Cli.readLine(handleAssembleLine, 0);
}

static bool handleAssembler(Cli::State state, uint16_t value, uintptr_t extra) {
  if (extra == ASM_ADDR) {
    last_addr = value;
    if (state == Cli::State::CLI_NEWLINE) {
      Cli.printUint16(last_addr);
      Cli.print('?');
      Cli.readLine(handleAssembleLine, 0);
    }
  }
  return false;
}

static void handleFileListing() {
  SD.begin();
  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
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
  Cli.printUint16(addr);
  Cli.print(':');
  Cli.printUint8(num);
  Cli.print(' ');
  return num;
}

static bool handleLoadFile(Cli::State state, char *line, uint16_t extra) {
  if (state != Cli::State::CLI_NEWLINE) return false;
  uint16_t size = 0;
  SD.begin();
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
      } else if (c != '\r' && p < s19 + sizeof(s19) -1) {
        *p++ = c;
      }
    }
    file.close();
  }
  Cli.println();
  Cli.print(size);
  Cli.println(F(" bytes loaded"));
  return true;
}

static bool handleSetRegister(Cli::State state, uint16_t value, uintptr_t extra) {
  if (extra == REG_NAME) {
    const char c = value & ~0x20;
    if (c == 'P' || c == 'S' || c == 'U' || c == 'Y' || c == 'X') {
      Cli.print(c);
      Cli.print('?');
      return Cli.readUint16(handleSetRegister, (uintptr_t)c);
    }
    if (c == 'D' || c == 'A' || c == 'B' || c == 'C') {
      Cli.print(c);
      Cli.print('?');
      return Cli.readUint8(handleSetRegister, (uintptr_t)c);
    }
    Cli.println();
    return true;
  }
  if (state == Cli::State::CLI_DELETE) {
    Cli.backspace(2);
    return Cli.readLetter(handleSetRegister, REG_NAME);
  }
  if (state == Cli::State::CLI_SPACE)
    Cli.println();
  if (extra == 'P') Regs.pc = value;
  else if (extra == 'S') Regs.s = value;
  else if (extra == 'U') Regs.u = value;
  else if (extra == 'Y') Regs.y = value;
  else if (extra == 'X') Regs.x = value;
  else if (extra == 'D') Regs.dp = value;
  else if (extra == 'B') Regs.b = value;
  else if (extra == 'A') Regs.a = value;
  else if (extra == 'C') Regs.cc = value;
  Regs.restore();
  Regs.print();
  return true;
}

static bool commandHandler(char);

bool Commands::exec(char c) {
  switch (c) {
  case 'p':
    Cli.print(F("pins:"));
    Pins.print();
    return true;
  case 'R':
    Cli.println(F("RESET"));
    _target = HALT;
    Pins.reset(true);
    Regs.get(true);
    disassemble(Regs.pc, 1);
    return true;
  case 'i':
    Cli.print(F("inst? "));
    return Cli.readUint8(handleInstruction, INST_DATA(0));
  case 'd':
    Cli.print(F("dump? "));
    return Cli.readUint16(handleDump, DUMP_ADDR);
  case 'D':
    Cli.print(F("Dis? "));
    return Cli.readUint16(handleDisassemble, DIS_ADDR);
  case 'A':
    Cli.print(F("Asm? "));
    return Cli.readUint16(handleAssembler, ASM_ADDR);
  case 'm':
    Cli.print(F("mem? "));
    return Cli.readUint16(handleMemory, MEMORY_ADDR);
  case 's':
  case 'S':
    if (_target == HALT) {
      if (c == 's') Cli.print(F("step: "));
      else Cli.println(F("Step"));
      Pins.step(c == 'S');
    } else {
      _target = HALT;
      Pins.halt(c == 'S');
    }
    Regs.get(true);
    disassemble(Regs.pc, 1);
    return true;
  case 'r':
    Cli.print(F("regs: "));
    Regs.get(true);
    return true;
  case '=':
    Cli.print(F("set reg? "));
    return Cli.readLetter(handleSetRegister, REG_NAME);
  case 'c':
    _target = STEP;
    return false;
  case 'G':
    if (_target != RUN) {
      Cli.println(F("GO"));
      _target = RUN;
      Pins.run();
    }
    return false;
  case 'h':
  case 'H':
    if (_target != HALT) {
      _target = HALT;
      Pins.halt(c == 'H');
      Cli.println(F("HALT"));
      Regs.get(true);
      disassemble(Regs.pc, 1);
    }
    return true;
  case 'F':
    Cli.println(F("Files"));
    handleFileListing();
    return true;
  case 'L':
    Cli.print(F("Load? "));
    return Cli.readLine(handleLoadFile, 0);
  case '?':
    Cli.println(VERSION);
    Cli.println(USAGE);
    return true;
  default:
    return false;
  }
}

static bool commandHandler(char c) {
  return Commands.exec(c);
}

static void printPrompt(Stream &console) {
  console.print(F("> "));
}

void Commands::begin() {
  Cli.setPrompt(printPrompt);
  Cli.readCommand(commandHandler);
  _target = HALT;
}

void Commands::loop() {
  if (_target == STEP) {
    Pins.step();
    Regs.get(true);
  }
}
