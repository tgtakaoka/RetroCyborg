/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __regs_h__
#define __regs_h__

#include <stdint.h>

union Regs {
  struct {
    uint16_t pc;
    uint16_t bc;
    uint16_t de;
    uint16_t hl;
    union {
      uint16_t af;
      struct {
        uint8_t f;
        uint8_t a;
      };
    };
    uint16_t ix;
    uint16_t iy;
    uint16_t sp;
  };

  void print() const;
  void get(bool show = false);
  void save();
  void restore();
};

extern Regs Regs;

#endif
