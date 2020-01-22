/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __pins_h__
#define __pins_h__

#include <stdint.h>

class Pins {
public:
  void begin();
  /** Return true when address bus points I/O area. */
  bool isIoAddr() const;
  bool isAck() const;
  bool isStep() const;
  void setE();
  void clrE();
  void setQ();
  void clrQ();
  void nop() const;
  void assertInt();
  void negateInt();
};

extern class Pins Pins;

#endif
