/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __regs_h__
#define __regs_h__

#include <stdint.h>

union Regs {
  struct {
    uint16_t pc;
    uint16_t u;
    uint16_t y;
    uint16_t x;
    uint8_t dp;
    uint8_t b;
    uint8_t a;
    uint8_t cc;
    uint16_t s;
  };
  uint8_t bytes[14];

  void print() const;
  void get(bool show = false);
  void save();
  void restore();
};

extern Regs Regs;

#endif
