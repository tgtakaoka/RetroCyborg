#ifndef __REGS_H__
#define __REGS_H__

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
  void get();
  void save();
  void restore();
};

extern Regs Regs;

#endif
