/*
  The Controller can accept commands represented by one letter.

   R - reset 6309.
   p - print 6309 hardware signal status.
   i - execute one instruction, input hex numbers.
   d - dump memory. addr [length]
   m - write memory. addr byte...
   s - step one instruction.
   r - print 6309 registers.
   = - set 6309 register.
   ? - print version.
*/

#include <Arduino.h>

#include "commands.h"
#include "hex.h"
#include "input.h"
#include "pins.h"
#include "regs.h"

#define VERSION F("* Cyborg09 Prototype1 0.8")
#define USAGE F("R:eset p:in i:nst d:ump m:emory s:tep r:eg =r:set")

class Commands Commands;

static uint8_t buffer[16];
#define NO_INDEX (sizeof(buffer) + 10)

static void execInst2(uint8_t inst, uint8_t opr, bool show = false) {
  const uint8_t insn[] = { inst, opr };
  Pins.execInst(insn, sizeof(insn), show);
}

static void execInst3(uint8_t inst, uint16_t opr, bool show = false) {
  const uint8_t insn[] = { inst, opr>>8, opr };
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
  execInst3(0x8E, addr); // LDX #addr
  for (uint16_t i = 0; i < len; i++) {
    execInst2(0xA6, 0x80); // LDA ,X+
    if (i % 16 == 0) {
      if (i) Serial.println();
      printHex4(addr + i, ':');
    }
    printHex2(Pins.dbus(), ' ');
  }
  Serial.println();
  Regs.restore();
}

static void handleDumpMemory(Input::State state, uint16_t value, uint8_t index) {
  static uint16_t addr;
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

static void memoryWrite(uint16_t addr, const uint8_t values[], const uint8_t len) {
  Regs.save();
  execInst3(0x8E, addr); // LDX #addr
  for (uint8_t i = 0; i < len; i++) {
    execInst2(0x86, values[i]); // LDA #values[i]
    execInst2(0xA7, 0x80);      // STA ,X+
  }
  Regs.restore();
}

static void handleMemoryWrite(Input::State state, uint16_t value, uint8_t index) {
  static uint16_t addr;
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
  if (c == 'R') Pins.reset();
  if (c == 'i') {
    Serial.print(F("i?"));
    Input.readUint8(handleInstruction, 0);
  }
  if (c == 'd') {
    Serial.print(F("d?"));
    Input.readUint16(handleDumpMemory, 0);
  }
  if (c == 'm') {
    Serial.print(F("m?"));
    Input.readUint16(handleMemoryWrite, NO_INDEX);
  }
  if (c == 's') {
    Pins.step();
    Regs.get();
    Regs.print();
    dumpMemory(Regs.pc, 6);
  }
  if (c == 'r') {
    Regs.get();
    Regs.print();
  }
  if (c == '=') {
    Serial.print(c);
    Input.readChar(handleSetRegister, 0);
  }
  if (c == '?') {
    Serial.println(VERSION);
    Serial.println(USAGE);
  }
}
