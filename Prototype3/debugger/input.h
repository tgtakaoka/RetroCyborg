/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __input_h__
#define __input_h__

#include <stdint.h>

class Input {

public:
  enum State {
    NEXT,
    FINISH,
    DELETE,
    CANCEL,
    CONTINUE,
  };
  // |state| can be NEXT, FINISH, or DELETE.
  typedef void (*InputHandler)(State state, uint16_t value, uint8_t index);
  typedef void (*LineHandler)(State state, char *line);

  void begin();
  void loop();
  void readUint8(InputHandler handler, uint8_t index);
  void readUint16(InputHandler handler, uint8_t index);
  void readUint8(InputHandler handler, uint8_t index, uint8_t value);
  void readUint16(InputHandler handler, uint8_t index, uint16_t value);
  void readChar(InputHandler handler, uint8_t index);
  void readLine(LineHandler hadler);
  static void backspace(int8_t n = 1);

private:
  enum Mode {
    CHAR_COMMAND,
    READ_UINT,
    READ_CHAR,
    READ_LINE,
  } _mode = CHAR_COMMAND;

  void readUint(InputHandler, uint8_t index, int8_t digits, uint16_t value = 0);
  void processHexNumbers(char c);
  void processReadLine(char c);

  InputHandler _handler;
  uint8_t _index;
  LineHandler _lineHandler;

  class HexBuffer {
  public:
    void reset(uint8_t digits);
    void set(uint8_t digits, uint16_t value);
    State append(char c);
    uint16_t value() {
      return _value;
    }

  private:
    uint8_t _len;
    uint8_t _digits;
    uint16_t _value;
  } _buffer;
  uint8_t _lineLen;
  char _lineBuffer[80];
};

extern Input Input;

#endif
