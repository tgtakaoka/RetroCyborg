#include <stdint.h>
#include <Arduino.h>

#include "input.h"

static void (*handleHexNumber)(uint16_t value);
static uint16_t hexNumber;

static enum InputMode readHexNumber(char command, void (*handler)(uint16_t)) {
  handleHexNumber = handler;
  Serial.print(command);
  Serial.print('?');
  hexNumber = 0;
  return HEX_NUMBER;
}

enum InputMode processHexNumber() {
  char c = Serial.read();
  if (isDigit(c)) {
    Serial.print(c);
    hexNumber *= 16;
    hexNumber += (c - '0');
  } else if (isHexadecimalDigit(c)) {
    Serial.print((char)(c & ~0x20));
    hexNumber *= 16;
    hexNumber += (c - (isUpperCase(c) ? 'A' : 'a')) + 10;
  }
  if (c == '\b' || c == 0x7f) {
    Serial.print("\b \b");
    hexNumber /= 16;
  }
  if (c == 0x1b) {
    Serial.println(" cancel");
    return CHAR_COMMAND;
  }
  if (c == '\r' || c == '\n') {
    Serial.println("");
    handleHexNumber(hexNumber);
    return CHAR_COMMAND;
  }

  return HEX_NUMBER;
}
