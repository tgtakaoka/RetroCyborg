/*
  The Controller can accept commands represented by one letter.

   R - reset 6309.
   p - print 6309 hardware signal status.
   i - execute one instruction, input hex numbers.
   d - dump memory. AH AL [LH] LL
   m - write memory. AH AL D0 [D1...]
   s - step one instruction.
   r - print 6309 registers.
   ? - print version.
*/

#include <Arduino.h>

#include "commands.h"
#include "hex.h"
#include "input.h"
#include "pins.h"
#include "regs.h"

#define VERSION F("* Cyborg09 Prototype1 0.7")

class Commands Commands;

static uint8_t buffer[16];

static void execInst2(uint8_t inst, uint8_t opr) {
  const uint8_t insn[] = { inst, opr };
  Pins.execInst(insn, sizeof(insn));
}

static void execInst3(uint8_t inst, uint16_t opr) {
  const uint8_t insn[] = { inst, opr>>8, opr };
  Pins.execInst(insn, sizeof(insn));
}

static void handleInstruction(Input::State state, uint16_t value, uint8_t index) {
  switch (state) {
  case Input::State::NEXT:
  case Input::State::FINISH:
    buffer[index++] = value;
    if (state == Input::State::NEXT && index < sizeof(buffer)) {
      Input.readUint8(handleInstruction, index);
    } else {
      Pins.execInst(buffer, index, true /* show */);
    }
    break;
  case Input::State::DELETE:
    if (index-- > 0) {
      Input::backspace();
      Input.readUint8(handleInstruction, index, buffer[index]);
    }
  }
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
  switch (state) {
  case Input::State::NEXT:
  case Input::State::FINISH:
    if (index == 0) {
      if (state == Input::State::FINISH) {
        dumpMemory(value, 16);
      } else {
        addr = value;
        Input.readUint16(handleDumpMemory, 1);
      }
    } else {
      dumpMemory(addr, value);
    }
    break;
  case Input::State::DELETE:
    if (index == 1) {
      Input::backspace();
      Input.readUint16(handleDumpMemory, 0, addr);
    }
  }
}

static void memoryWrite(uint16_t addr, const uint8_t values[], const uint8_t len) {
  Regs.save();
  execInst3(0x8E, addr); // LDX #addr
  for (uint8_t i = 0; i < len; i++) {
    execInst2(0x86, values[i]); // LDA #values[i]
    execInst3(0xA7, 0x80);      // STA ,X+
  }
  Regs.restore();
}

static void handleMemoryWrite(Input::State state, uint16_t value, uint8_t index) {
  static uint16_t addr;
  switch (state) {
  case Input::State::NEXT:
    if (index == sizeof(buffer)) {
      addr = value;
      Input.readUint8(handleMemoryWrite, 0);
      break;
    }
    // fall-through
  case Input::State::FINISH:
    buffer[index++] = value;
    if (index < sizeof(buffer) && state == Input::State::NEXT) {
      Input.readUint8(handleMemoryWrite, index);
    } else {
      if (state == Input::State::NEXT) Serial.println();
      memoryWrite(addr, buffer, index);
      dumpMemory(addr, index);
    }
    break;
  case Input::State::DELETE:
    if (index == 0) {
      Input::backspace();
      Input.readUint16(handleMemoryWrite, sizeof(buffer), addr);
    } else if (index < sizeof(buffer)) {
      index--;
      Input::backspace();
      Input.readUint8(handleMemoryWrite, index, buffer[index]);
    }
  }
}

void Commands::loop() {
  const char c = Serial.read();
  if (c == -1) return;
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
    Input.readUint16(handleMemoryWrite, sizeof(buffer));
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
  if (c == '?') Serial.println(VERSION);
}
