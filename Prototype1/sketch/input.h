#ifndef __readnum_h__
#define __readnum_h__

#include <stdint.h>

enum InputMode {
  CHAR_COMMAND,
  HEX_NUMBER,
};

enum InputMode readHexNumber(char command, void (*handler)(uint16_t));
enum InputMode processHexNumber();

#endif
