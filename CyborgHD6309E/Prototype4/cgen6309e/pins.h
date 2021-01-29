/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __PINS_H__
#define __PINS_H__

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
    void nop() const { asm volatile("nop"); }
    void assertInt();
    void negateInt();
};

extern class Pins Pins;

#endif
