/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __REGS_H__
#define __REGS_H__

#include <stdint.h>

union Regs {
    struct {
        uint16_t pc;
        union {
            uint16_t bc;
            struct {
                uint8_t c;
                uint8_t b;
            };
        };
        union {
            uint16_t de;
            struct {
                uint8_t e;
                uint8_t d;
            };
        };
        union {
            uint16_t hl;
            struct {
                uint8_t l;
                uint8_t h;
            };
        };
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
