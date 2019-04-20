#ifndef __pins_h__
#define __pins_h__

#include <stdint.h>

class Pins {

public:
  struct Status {
    uint16_t cycle;
    unsigned reset:1;
    unsigned halt:1;
    unsigned ba:1;
    unsigned bs:1;
    unsigned lic:1;
    unsigned avma:1;
    unsigned busy:1;
    unsigned rw:1;
    uint8_t dbus;
  };

  void begin();
  void getStatus(struct Status *);
  // |value|: LOW or HIGH.
  void reset(uint8_t value);
  void halt(uint8_t value);
  bool isLic() const;
  // |dir|: OUTPUT or INPUT_PULLUP.
  // return: false when |RW| and |dir| conflicts.
  bool setDataBusDirection(uint8_t dir);
  // |data|: 8 bit data.
  void setDataBus(uint8_t data);
  // return: 8 bit data.
  uint8_t getDataBus() const;
  void printStatus() const;
};

extern Pins Pins;

#endif /* __pins_h__ */
