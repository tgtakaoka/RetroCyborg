#include <stdint.h>
#include <Arduino.h>

#include "commands.h"
#include "hex.h"
#include "input.h"

class Input Input;

static void printChars(const __FlashStringHelper *chars, int8_t n) {
  while (n-- > 0)
    Serial.print(chars);
}

void Input::backspace(int8_t n) {
  printChars(F("\b \b"), n);
}

void Input::HexBuffer::reset(uint8_t digits) {
  _len = 0;
  _digits = digits;
  _value = 0;
}

void Input::HexBuffer::set(uint8_t digits, uint16_t value) {
  _len = _digits = digits;
  _value = value;
  if (digits == 4)
    printHex2(value >> 8);
  printHex2(value);
}

Input::State Input::HexBuffer::append(char c) {
  if (isHexadecimalDigit(c) && _len < _digits) {
    if (isLowerCase(c)) c &= ~0x20;
    Serial.print(c);
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
    Serial.print(c);
    return c == ' ' ? NEXT : FINISH;
  }
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
      Serial.println(F(" cancel"));
      _mode = CHAR_COMMAND;
      break;
    case CONTINUE:
      break;
  }
}

void Input::loop() {
  if (Serial.available()) {
    const char c = Serial.read();
    switch (_mode) {
      case CHAR_COMMAND:
        Commands.exec(c);
        break;
      case READ_UINT:
        processHexNumbers(c);
        break;
      case READ_CHAR:
        _mode = CHAR_COMMAND;
        _handler(FINISH, c, _index);
        break;
    }
  }

  Commands.loop();
}
