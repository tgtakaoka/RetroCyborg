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
   C - continuously run.
   h - halt MPU.
   ? - print version.
*/

#include <Arduino.h>
#include <string.h>
#include <SD.h>

#include "commands.h"
#include "hex.h"
#include "input.h"
#include "pins.h"
#include "regs.h"

#include "asm_hd6309.h"
#include "dis_hd6309.h"
#include "memory.h"
#include "symbol_table.h"

#define VERSION F("* Cyborg09 Prototype3 1.6")
#define USAGE F("R:eset r:egs =:setReg d:ump D:iasm m:emory i:nst A:asm s/S:tep c/C:ont h/H:alt p:ins F:iles L:oad")

class Commands Commands;

static uint16_t addr;
static uint8_t buffer[16];
#define NO_INDEX (sizeof(buffer) + 10)

static void execInst2(uint8_t inst, uint8_t opr, bool show = false) {
  const uint8_t insn[] = { inst, opr };
  Pins.execInst(insn, sizeof(insn), show);
}

static void execInst3(uint8_t inst, uint16_t opr, bool show = false) {
  const uint8_t insn[] = { inst, opr >> 8, opr };
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
      Input.readUint16(handleDumpMemory, 0, addr);
    }
    return;
  }
  if (index == 0) {
    addr = value;
    if (state == Input::State::NEXT) {
      Input.readUint16(handleDumpMemory, 1);
      return;
    }
    value = 16;
  }
  dumpMemory(addr, value);
}


class Hd6309Memory : public Memory {
  public:
    Hd6309Memory() : Memory(0) {}
    bool hasNext() const override {
      return true;
    }
    void setAddress(target::uintptr_t addr) {
      _address = addr;
    }
  protected:
    target::byte_t nextByte() {
      Regs.save();
      execInst3(0xB6, _address); // LDA $_address
      target::byte_t data = Pins.dbus();
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

static void disassemble(uint16_t addr, uint16_t len) {
  DisHd6309 dis(HD6309);
  class Hd6309Memory memory;
  memory.setAddress(addr);
  while (len-- != 0) {
    char operands[20];
    char comments[20];
    Insn insn;
    dis.decode(memory, insn, operands, comments, nullptr);
    print(insn);
    if (dis.getError()) {
      Serial.print(F("Error: "));
      Serial.println(dis.getError(), DEC);
      continue;
    }
    print(insn.name(), 6);
    print(operands, 12);
    if (*comments) {
      Serial.print(';');
      Serial.print(comments);
    }
    Serial.println();
  }
}

static void handleDisassemble(Input::State state , uint16_t value, uint8_t index) {
  if (state == Input::State::DELETE) {
    if (index == 1) {
      Input::backspace();
      Input.readUint16(handleDisassemble, 0, addr);
    }
    return;
  }
  if (index == 0) {
    addr = value;
    if (state == Input::State::NEXT) {
      Input.readUint16(handleDisassemble, 1);
      return;
    }
    value = 16;
  }
  disassemble(addr, value);
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
    addr = value;
    Input.readUint8(handleMemoryWrite, 0);
    return;
  }
  if (state == Input::State::DELETE) {
    if (index-- == 0) {
      Input::backspace();
      Input.readUint16(handleMemoryWrite, NO_INDEX, addr);
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
  memoryWrite(addr, buffer, index);
  dumpMemory(addr, index);
}

static void handlerAssembleLine(Input::State state, const String& line) {
  if (state == Input::State::CANCEL || line.length() == 0) {
    Serial.println(F("end"));
    return;
  }
  AsmHd6309 assembler(HD6309);
  Insn insn;
  if (assembler.encode(line.c_str(), insn, addr, nullptr)) {
    Serial.print(F("Error: "));
    Serial.println(assembler.getError(), DEC);
  } else {
    print(insn);
    Serial.println();
    memoryWrite(insn.address(), insn.bytes(), insn.insnLen());
    addr += insn.insnLen();
  }
  printHex16(addr, '?');
  Input.readLine(handlerAssembleLine);
}

static void handlerAssembler(Input::State state, uint16_t value, uint8_t index) {
  if (index == 0) {
    addr = value;
    if (state == Input::State::FINISH) {
      printHex16(addr, '?');
      Input.readLine(handlerAssembleLine);
    }
  }
}

static void handlerFileListing() {
  File root = SD.open("/");
  while (true) {
    File entry = root.openNextFile();
    if (!entry) break;
    if (entry.isDirectory()) continue;
    Serial.print(entry.name());
    Serial.print('\t');
    Serial.println(entry.size(), DEC);
    entry.close();
  }
  root.close();
}

static uint8_t toInt(const char c) {
  return isDigit(c) ? c - '0' : c - 'A' + 10;
}

static uint8_t toInt8Hex(const String& text) {
    return (toInt(text[0]) << 4) | toInt(text[1]);
}

static uint16_t toInt16Hex(const String& text) {
  return ((uint16_t)toInt8Hex(text) << 8) | toInt8Hex(text.substring(2));
}

static void loadS19Record(const String& line) {
  int len = line.length();
  if (line.startsWith("S1") && len > 8) {
    const int num = toInt8Hex(line.substring(2)) - 3;
    const uint16_t addr = toInt16Hex(line.substring(4));
    uint8_t buffer[num];
    for (int i = 0; i < num; i++) {
      buffer[i] = toInt8Hex(line.substring(i * 2 + 8));
    }
    memoryWrite(addr, buffer, num);
    printHex16(addr, ':');
    printHex8(num, '\r');
  }
}

static void handlerLoadFile(Input::State state, const String& line) {
  if (state != Input::State::FINISH) return;
  File file = SD.open(line.c_str());
  String s19;
  while (file.available() > 0) {
    const char c = file.read();
    if (c == '\n') {
      loadS19Record(s19);
      s19 = "";
    } else if (c != '\r') {
      s19 += c;
    }
  }
  file.close();
  Serial.println();
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

void Commands::exec(char c) {
  if (c == 'p') Pins.print();
  if (c == 'R') {
    _target = HALT;
    Pins.reset();
    Serial.println(F("RESET"));
    Regs.get(true);
  }
  if (c == 'i') {
    Serial.print(F("i?"));
    Input.readUint8(handleInstruction, 0);
  }
  if (c == 'd') {
    Serial.print(F("d?"));
    Input.readUint16(handleDumpMemory, 0);
  }
  if (c == 'D') {
    Serial.printf(F("D?"));
    Input.readUint16(handleDisassemble, 0);
  }
  if (c == 'A') {
    Serial.printf(F("A?"));
    Input.readUint16(handlerAssembler, 0);
  }
  if (c == 'm') {
    Serial.print(F("m?"));
    Input.readUint16(handleMemoryWrite, NO_INDEX);
  }
  if (c == 's' || c == 'S') {
    if (_target == HALT) {
      Pins.step(c == 'S');
    } else {
      _target = HALT;
      Pins.halt(c == 'S');
    }
    Regs.get(true);
    disassemble(Regs.pc, 1);
  }
  if (c == 'r') Regs.get(true);
  if (c == '=') {
    Serial.print(c);
    Input.readChar(handleSetRegister, 0);
  }
  if (c == 'c') {
    _target = STEP;
  }
  if (c == 'C' && _target != RUN) {
    _target = RUN;
    Pins.run();
    Serial.println(F("RUN"));
  }
  if ((c == 'h' || c == 'H') && _target != HALT) {
    _target = HALT;
    Pins.halt(c == 'H');
    Serial.println(F("HALT"));
    Regs.get(true);
    disassemble(Regs.pc, 1);
  }
  if (c == 'F') {
    handlerFileListing();
  }
  if (c == 'L') {
    Serial.print(F("L?"));
    Input.readLine(handlerLoadFile);
  }
  if (c == '?') {
    Serial.println(VERSION);
    Serial.println(USAGE);
  }
}

void Commands::loop() {
  if (_target == STEP) {
    Pins.step();
    Regs.get(true);
  }
}
