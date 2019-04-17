#ifndef __readnum_h__
#define __readnum_h__

#include <stdint.h>

class Input {

public:
  void loop();
  typedef void (*HexHandler)(uint8_t values[], uint8_t num);
  void readHex(char command, HexHandler handler);

private:
  enum Mode {
    CHAR_COMMAND,
    HEX_NUMBERS,
  } _mode = CHAR_COMMAND;

  void processHexNumbers();

  HexHandler _hexHandler;
  uint8_t _len;
  uint8_t _values[8];

  class HexBuffer {
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
    uint8_t hexValue() { return _value; }
    static const uint8_t _max_len = 2;

  private:
    uint8_t _len;
    uint8_t _value;
  } _buffer;
};

extern Input Input;

#endif
