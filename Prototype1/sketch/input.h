#ifndef __readnum_h__
#define __readnum_h__

#include <stdint.h>

class Input {

public:
  typedef void (*Uint8Handler)(uint8_t values[], uint8_t len);

  void loop();
  void readUint8(char command, Uint8Handler handler);

private:
  enum Mode {
    CHAR_COMMAND,
    HEX_NUMBERS,
  } _mode = CHAR_COMMAND;

  void processHexNumbers();

  Uint8Handler _handler;
  uint8_t _len;
  uint8_t _values[18];

  class HexBuffer {
  public:
    void reset(uint8_t digits);
    enum State {
      CONTINUE,
      NEXT,
      FINISH,
      DELETE,
      CANCEL,
    };
    State append(char c);
    void set(uint16_t value);
    uint16_t value() { return _value; }
    uint8_t digits() { return _digits; }

  private:
    uint8_t _len;
    uint8_t _digits;
    uint16_t _value;
  } _buffer;
};

extern Input Input;

#endif
