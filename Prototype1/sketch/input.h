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

  void processUint8();

  Uint8Handler _handler;
  uint8_t _len;
  uint8_t _values[16];

  class Uint8Buffer {
  public:
    void reset();
    enum State {
      CONTINUE,
      NEXT,
      FINISH,
      DELETE,
      CANCEL,
    };
    State append(char c);
    void set(uint8_t value);
    uint8_t value() { return _value; }
    static const uint8_t _max_len = 2;

  private:
    uint8_t _len;
    uint8_t _value;
  } _buffer;
};

extern Input Input;

#endif
