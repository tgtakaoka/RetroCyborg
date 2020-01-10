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
#include <asm_hd6309.h>
#include <dis_hd6309.h>
#include <dis_memory.h>
#include <symbol_table.h>

#include "commands.h"
#include "hex.h"
#include "input.h"
#include "pins.h"
#include "regs.h"

#define VERSION F("* Cyborg09 Prototype3 1.9")
#define USAGE F("R:eset r:egs =:setReg d:ump D:iasm m:emory i:nst A:sm s/S:tep c:ont G:o h/H:alt p:ins F:iles L:oad")

class Commands Commands;

static uint16_t last_addr;
static uint8_t buffer[16];
#define NO_INDEX (sizeof(buffer) + 10)

static void execInst2(uint8_t inst, uint8_t opr, bool show = false) {
  const uint8_t insn[] = { inst, opr };
  Pins.execInst(insn, sizeof(insn), show);
}

static void execInst3(uint8_t inst, uint16_t opr, bool show = false) {
  const uint8_t insn[] = { inst, (uint8_t)(opr >> 8), (uint8_t)opr };
  Pins.execInst(insn, sizeof(insn), show);
}

static void handleInstruction(Input::State state, uint16_t value, uint8_t index) {
  if (state == Input::State::DELETE) {
    if (index-- > 0) {
      Input::backspace();
      Input.readUint8(handleInstruction, index, buffer[index]);
    }
    return;
  }

  buffer[index++] = value;
  if (state == Input::State::NEXT) {
    if (index < sizeof(buffer)) {
      Input.readUint8(handleInstruction, index);
      return;
    }
    Serial.println();
  }
  Pins.execInst(buffer, index, true /* show */);
}

static void dumpMemory(uint16_t addr, uint16_t len) {
  Regs.save();
  for (uint16_t i = 0; i < len; i++, addr++) {
    execInst3(0xB6, addr); // LDA $addr
    if (i % 16 == 0) {
      if (i) Serial.println();
      printHex16(addr, ':');
    }
    printHex8(Pins.dbus(), ' ');
  }
  Serial.println();
  Regs.restore();
}

static void handleDumpMemory(Input::State state, uint16_t value, uint8_t index) {
  if (state == Input::State::DELETE) {
    if (index == 1) {
      Input::backspace();
      Input.readUint16(handleDumpMemory, 0, last_addr);
    }
    return;
  }
  if (index == 0) {
    last_addr = value;
    if (state == Input::State::NEXT) {
      Input.readUint16(handleDumpMemory, 1);
      return;
    }
    value = 16;
  }
  dumpMemory(last_addr, value);
  last_addr += value;
}


class Hd6309Memory : public DisMemory<target::uintptr_t> {
  public:
    Hd6309Memory() : DisMemory(0) {}
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
  Serial.print(text);
  for (int i = strlen(text); i < width; i++) {
    Serial.print(' ');
  }
}

static void print(const Insn& insn) {
  printHex16(insn.address(), ':');
  for (int i = 0; i < insn.insnLen(); i++) {
    printHex8(insn.bytes()[i], ' ');
  }
  for (int i = insn.insnLen(); i < 5; i++) {
    Serial.print(F("   "));
  }
}

static uint16_t disassemble(uint16_t addr, uint16_t max) {
  DisHd6309 dis;
  class Hd6309Memory memory;
  memory.setAddress(addr);
  uint16_t len = 0;
  while (len < max) {
    char operands[20];
    Insn insn;
    dis.decode(memory, insn, operands, nullptr);
    len += insn.insnLen();
    print(insn);
    if (dis.getError()) {
      Serial.print(F("Error: "));
      Serial.println(dis.getError(), DEC);
      continue;
    }
    print(insn.name(), 6);
    print(operands, 12);
    Serial.println();
  }
  return addr + len;
}

static void handleDisassemble(Input::State state , uint16_t value, uint8_t index) {
  if (state == Input::State::DELETE) {
    if (index == 1) {
      Input::backspace();
      Input.readUint16(handleDisassemble, 0, last_addr);
    }
    return;
  }
  if (index == 0) {
    last_addr = value;
    if (state == Input::State::NEXT) {
      Input.readUint16(handleDisassemble, 1);
      return;
    }
    value = 16;
  }
  last_addr = disassemble(last_addr, value);
}

static void memoryWrite(uint16_t addr, const uint8_t values[], const uint8_t len) {
  Regs.save();
  for (uint8_t i = 0; i < len; i++, addr++) {
    execInst2(0x86, values[i]); // LDA #values[i]
    execInst3(0xB7, addr);      // STA $addr
  }
  Regs.restore();
}

static void handleMemoryWrite(Input::State state, uint16_t value, uint8_t index) {
  if (index == NO_INDEX) {
    last_addr = value;
    Input.readUint8(handleMemoryWrite, 0);
    return;
  }
  if (state == Input::State::DELETE) {
    if (index-- == 0) {
      Input::backspace();
      Input.readUint16(handleMemoryWrite, NO_INDEX, last_addr);
    } else {
      Input::backspace();
      Input.readUint8(handleMemoryWrite, index, buffer[index]);
    }
    return;
  }
  buffer[index++] = value;
  if (state == Input::State::NEXT) {
    if (index < sizeof(buffer)) {
      Input.readUint8(handleMemoryWrite, index);
      return;
    }
    Serial.println();
  }
  memoryWrite(last_addr, buffer, index);
  dumpMemory(last_addr, index);
  last_addr += index;
}

static void handlerAssembleLine(Input::State state, char *line) {
  if (state == Input::State::CANCEL || *line == 0) {
    Serial.println(F("end"));
    return;
  }
  AsmHd6309 assembler;
  Insn insn;
  if (assembler.encode(line, insn, last_addr, nullptr)) {
    Serial.print(F("Error: "));
    Serial.println(assembler.getError(), DEC);
  } else {
    print(insn);
    Serial.println();
    memoryWrite(insn.address(), insn.bytes(), insn.insnLen());
    last_addr += insn.insnLen();
  }
  printHex16(last_addr, '?');
  Input.readLine(handlerAssembleLine);
}

static void handlerAssembler(Input::State state, uint16_t value, uint8_t index) {
  if (index == 0) {
    last_addr = value;
    if (state == Input::State::FINISH) {
      printHex16(last_addr, '?');
      Input.readLine(handlerAssembleLine);
    }
  }
}

static void handlerFileListing() {
  SD.begin();
  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
    if (!entry.isDirectory()) {
      Serial.print(entry.name());
      Serial.print('\t');
      Serial.println(entry.size(), DEC);
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
  printHex16(addr, ':');
  printHex8(num);
  Serial.print(' ');
  return num;
}

static void handlerLoadFile(Input::State state, char *line) {
  if (state != Input::State::FINISH) return;
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
  Serial.println();
  Serial.print(size, DEC);
  Serial.println(F(" bytes loaded"));
}

void handleSetRegister(Input::State state, uint16_t value, uint8_t index) {
  if (index == 0) {
    const char c = value & ~0x20;
    if (c == 'P' || c == 'S' || c == 'U' || c == 'Y' || c == 'X') {
      Serial.print(c);
      Serial.print('?');
      Input.readUint16(handleSetRegister, c);
      return;
    }
    if (c == 'D' || c == 'A' || c == 'B' || c == 'C') {
      Serial.print(c);
      Serial.print('?');
      Input.readUint8(handleSetRegister, c);
      return;
    }
    Serial.println();
    return;
  }
  if (state == Input::State::DELETE) {
    Input::backspace(2);
    Input.readChar(handleSetRegister, 0);
    return;
  }
  if (state == Input::State::NEXT)
    Serial.println();
  if (index == 'P') Regs.pc = value;
  else if (index == 'S') Regs.s = value;
  else if (index == 'U') Regs.u = value;
  else if (index == 'Y') Regs.y = value;
  else if (index == 'X') Regs.x = value;
  else if (index == 'D') Regs.dp = value;
  else if (index == 'B') Regs.b = value;
  else if (index == 'A') Regs.a = value;
  else if (index == 'C') Regs.cc = value;
  Regs.restore();
  Regs.print();
}

bool Commands::exec(char c) {
  switch (c) {
  case 'p':
    Serial.print(F("pins:"));
    Pins.print();
    break;
  case 'R':
    Serial.println(F("RESET"));
    _target = HALT;
    Pins.reset(true);
    Regs.get(true);
    break;
  case 'i':
    Serial.print(F("inst? "));
    Input.readUint8(handleInstruction, 0);
    break;
  case 'd':
    Serial.print(F("dump? "));
    Input.readUint16(handleDumpMemory, 0);
    break;
  case 'D':
    Serial.print(F("Dis? "));
    Input.readUint16(handleDisassemble, 0);
    break;
  case 'A':
    Serial.print(F("Asm? "));
    Input.readUint16(handlerAssembler, 0);
    break;
  case 'm':
    Serial.print(F("mem? "));
    Input.readUint16(handleMemoryWrite, NO_INDEX);
    break;
  case 's':
  case 'S':
    if (_target == HALT) {
      if (c == 's') Serial.print(F("step: "));
      else Serial.println(F("Step"));
      Pins.step(c == 'S');
    } else {
      _target = HALT;
      Pins.halt(c == 'S');
    }
    Regs.get(true);
    disassemble(Regs.pc, 1);
    break;
  case 'r':
    Serial.print(F("regs: "));
    Regs.get(true);
    break;
  case '=':
    Serial.print(F("set reg? "));
    Input.readChar(handleSetRegister, 0);
    break;
  case 'c':
    _target = STEP;
    return false;
  case 'G':
    if (_target != RUN) {
      Serial.println(F("GO"));
      _target = RUN;
      Pins.run();
      break;
    }
    return false;
  case 'h':
  case 'H':
    if (_target != HALT) {
      _target = HALT;
      Pins.halt(c == 'H');
      Serial.println(F("HALT"));
      Regs.get(true);
      disassemble(Regs.pc, 1);
      break;
    }
    return false;
  case 'F':
    Serial.println(F("Files"));
    handlerFileListing();
    break;
  case 'L':
    Serial.print(F("Load? "));
    Input.readLine(handlerLoadFile);
    break;
  case '?':
    Serial.println(VERSION);
    Serial.println(USAGE);
    break;
  default:
    return false;
  }
  return true;
}

static void haltMpu() {
  Commands.exec('h');
  Serial.print(F("> ")); // Initial prompt.
}

void Commands::begin() {
  Pins.attachUserSwitch(haltMpu);
  exec('?');
  Serial.print(F("> ")); // Initial prompt.
}

void Commands::loop() {
  if (_target == STEP) {
    Pins.step();
    Regs.get(true);
  }
}
