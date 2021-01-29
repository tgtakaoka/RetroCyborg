/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
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
        union {
            uint16_t d;
            struct {
                uint8_t b;
                uint8_t a;
            };
        };
        uint8_t cc;
        uint16_t s;
        union {
            uint16_t w;
            struct {
                uint8_t f;
                uint8_t e;
            };
        };
        uint16_t v;
    };
    uint8_t bytes[18];

    void print() const;
    void get(bool show = false);
    void save();
    void restore();
    bool is6309() const;
    const char *cpu() const;
};

extern Regs Regs;

#endif
