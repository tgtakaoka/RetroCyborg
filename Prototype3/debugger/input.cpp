/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */

#include "commands.h"
#include "console.h"
#include "input.h"

#include <stdint.h>
#include <Arduino.h>

class Input Input;

void Input::backspace(int8_t n) {
  while (n-- > 0)
    Console.print(F("\b \b"));
  Console.flush();
}

void Input::HexBuffer::reset(uint8_t digits) {
  _len = 0;
  _digits = digits;
  _value = 0;
}

void Input::HexBuffer::set(uint8_t digits, uint16_t value) {
  _len = _digits = digits;
  _value = value;
  if (digits == 4) {
    Console.hex16(value);
  } else {
    Console.hex8(static_cast<uint8_t>(value));
  }
  Console.flush();
}

Input::State Input::HexBuffer::append(char c) {
  if (isHexadecimalDigit(c) && _len < _digits) {
    if (isLowerCase(c)) c &= ~0x20;
    Console.print(c);
    Console.flush();
    _len++;
    _value *= 16;
    _value += isDigit(c) ? c - '0' : c - 'A' + 10;
    return CONTINUE;
  }
  if (c == '\b' || c == '\x7f') {
    if (_len == 0) return DELETE;
    backspace();
    _value /= 16;
    _len--;
    return CONTINUE;
  }
  if (_len > 0 && isSpace(c)) {
    backspace(_len);
    set(_digits, _value);
    Console.print(c);
    Console.flush();
    if (c == ' ') return NEXT;
    Console.println();
    return FINISH;
  }
  // TODO: Handle cancel nicely.
  // if (_len == 0 && isSpace(c)) return CANCEL;
  if (c == '\x1b') return CANCEL;
  return CONTINUE;
}

void Input::readUint8(InputHandler handler, uint8_t index) {
  readUint(handler, index, -2);
}

void Input::readUint16(InputHandler handler, uint8_t index) {
  readUint(handler, index, -4);
}

void Input::readUint8(InputHandler handler, uint8_t index, uint8_t value) {
  readUint(handler, index, 2, value);
}

void Input::readUint16(InputHandler handler, uint8_t index, uint16_t value) {
  readUint(handler, index, 4, value);
}

void Input::readUint(InputHandler handler, uint8_t index, int8_t digits, uint16_t value) {
  if (digits < 0) {
    _buffer.reset(-digits);
  } else {
    backspace(digits);
    _buffer.set(digits, value);
  }
  _handler = handler;
  _index = index;
  _mode = READ_UINT;
}

void Input::readChar(InputHandler handler, uint8_t index) {
  _handler = handler;
  _index = index;
  _mode = READ_CHAR;
}

void Input::readLine(LineHandler handler) {
  _lineHandler = handler;
  _lineLen = 0;
  *_lineBuffer = 0;
  _mode = READ_LINE;
}

void Input::processHexNumbers(char c) {
  const State state = _buffer.append(c);
  switch (state) {
  case NEXT:
  case FINISH:
    _mode = CHAR_COMMAND;
    _handler(state, _buffer.value(), _index);
    break;
  case DELETE:
    _handler(state, 0, _index);
    break;
  case CANCEL:
    Console.println(F(" cancel"));
    _mode = CHAR_COMMAND;
    break;
  case CONTINUE:
    break;
  }
}

static void trimLineBuffer(char *line) {
  char *top = line;
  while (isSpace(*top)) top++;
  char *end = top + strlen(top) - 1;
  while (end >= top && isSpace(*end)) *end-- = 0;
  while ((*line++ = *top++) != 0)
    ;
}

void Input::processReadLine(char c) {
  if (c == '\r' || c == '\n') {
    _mode = CHAR_COMMAND;
    Console.println();
    trimLineBuffer(_lineBuffer);
    _lineHandler(FINISH, _lineBuffer);
  } else if (c == '\b' || c == '\x7f') {
    if (_lineLen > 0) {
      backspace();
      _lineBuffer[--_lineLen] = 0;
    }
  } else if (c == '\x1b') {
    Console.println(F(" cancel"));
    _mode = CHAR_COMMAND;
    _lineHandler(CANCEL, _lineBuffer);
  } else if (_lineLen < sizeof(_lineBuffer) - 1) {
    Console.print(c);
    Console.flush();
    _lineBuffer[_lineLen++] = c;
    _lineBuffer[_lineLen] = 0;
  }
}

void Input::begin() {
  Commands.begin();
}

void Input::loop() {
  if (Console.available()) {
    const char c = Console.read();
    switch (_mode) {
    case CHAR_COMMAND:
      if (Commands.exec(c)) {
        if (!Commands.isRunning() && _mode == CHAR_COMMAND) {
          Console.print(F("> ")); // prompt
          Console.flush();
        }
      }
      break;
    case READ_UINT:
      processHexNumbers(c);
      break;
    case READ_CHAR:
      _mode = CHAR_COMMAND;
      _handler(FINISH, c, _index);
      break;
    case READ_LINE:
      processReadLine(c);
      break;
    }
  }

  Commands.loop();
}
