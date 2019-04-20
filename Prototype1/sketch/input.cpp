#include <stdint.h>
#include <Arduino.h>

#include "commands.h"
#include "input.h"

class Input Input;

#define MSG_BS     F("\b \b")
#define MSG_CANCEL F(" cancel")

static uint8_t toNumber(char c) {
  return isDigit(c) ? c - '0' : c - 'A' + 10;
}

static char toChar(uint8_t value) {
  value &= 0xf;
  return value < 10 ? value + '0' : value - 10 + 'A';
}

static void backspace(int8_t n = 1) {
  while (n-- > 0)
    Serial.print(MSG_BS);
}

void Input::HexBuffer::reset() {
  _len = 0;
  _value = 0;
}

void Input::HexBuffer::set(uint8_t value) {
  _value = value;
  _len = _max_len;
  Serial.print(toChar(value >> 4));
  Serial.print(toChar(value));
}

Input::HexBuffer::State Input::HexBuffer::append(char c) {
  if (isHexadecimalDigit(c) && _len < _max_len) {
    if (isLowerCase(c)) c &= ~0x20;
    Serial.print(c);
    _len++;
    _value *= 16;
    _value += toNumber(c);
    return CONTINUE;
  }
  if (c == '\b' || c == '\x7f') {
    if (_len == 0) return DELETE;
    _value /= 16;
    _len--;
    backspace();
  }
  if (_len > 0 && isSpace(c)) {
    backspace(_len);
    set(_value);
    if (c == ' ') {
      Serial.print(' ');
      return NEXT;
    }
    Serial.println();
    return FINISH;
  }
  if (c == '\x1b') return CANCEL;
  return CONTINUE;
}

void Input::readHex(char command, HexHandler handler) {
  _command = command;
  _hexHandler = handler;
  Serial.print(command);
  Serial.print('?');
  _buffer.reset();
  _mode = HEX_NUMBERS;
  _len = 0;
}

void Input::processHexNumbers() {
  Input::HexBuffer::State state = _buffer.append(Serial.read());
  switch (state) {
  case HexBuffer::State::CONTINUE:
    break;
  case HexBuffer::State::NEXT:
  case HexBuffer::State::FINISH:
    _values[_len++] = _buffer.hexValue();
    if (state == HexBuffer::State::FINISH || _len == sizeof(_values)) {
      _hexHandler(_command, _values, _len);
      _mode = CHAR_COMMAND;
    } else {
      _buffer.reset();
    }
    break;
  case HexBuffer::State::DELETE:
    if (_len > 0) {
      backspace(HexBuffer::_max_len + 1);
      _buffer.set(_values[--_len]);
    }
    break;
  case HexBuffer::State::CANCEL:
    Serial.println(MSG_CANCEL);
    _mode = CHAR_COMMAND;
  }
}

void Input::loop() {
  switch (_mode) {
  case CHAR_COMMAND:
    Commands.loop();
    break;
  case HEX_NUMBERS:
    processHexNumbers();
    break;
  }
}
