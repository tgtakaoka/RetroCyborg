/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __REGS_H__
#define __REGS_H__

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
