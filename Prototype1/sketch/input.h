#ifndef __readnum_h__
#define __readnum_h__

#include <stdint.h>

class Input {

public:
  enum State {
    CONTINUE,
    NEXT,
    FINISH,
    DELETE,
    CANCEL,
  };
  typedef void (*UintHandler)(State state, uint16_t values, uint8_t index);

  void loop();
  void readUint8(UintHandler handler, uint8_t index);
  void readUint16(UintHandler handler, uint8_t index);
  void readUint8(UintHandler handler, uint8_t index, uint8_t value);
  void readUint16(UintHandler handler, uint8_t index, uint16_t value);
  static void backspace(int8_t n = 1);

private:
  enum Mode {
    CHAR_COMMAND,
    HEX_NUMBERS,
  } _mode = CHAR_COMMAND;

  void readUint(UintHandler, uint8_t index, int8_t digits, uint16_t value = 0);
  void processHexNumbers();

  UintHandler _handler;
  uint8_t _index;

  class HexBuffer {
  public:
    void reset(uint8_t digits);
    void set(uint8_t digits, uint16_t value);
    State append(char c);
    uint16_t value() { return _value; }

  private:
    uint8_t _len;
    uint8_t _digits;
    uint16_t _value;
  } _buffer;
};

extern Input Input;

#endif
