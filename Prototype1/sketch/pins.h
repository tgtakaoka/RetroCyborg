#ifndef __pins_h__
#define __pins_h__

#include <stdint.h>

class Pins {

public:
  void begin();
  // |value|: LOW or HIGH.
  void reset(uint8_t value);
  void halt(uint8_t value);
  // |dir|: OUTPUT or INPUT_PULLUP.
  // return: false when |RW| and |dir| conflicts.
  bool setDataBusDirection(uint8_t dir);
  // |data|: 8 bit data.
  void setDataBus(uint8_t data);
  // return: 8 bit data.
  uint8_t getDataBus();
  void printStatus();
};

extern Pins Pins;

#endif /* __pins_h__ */
