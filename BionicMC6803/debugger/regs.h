#ifndef __REGS_H__
#define __REGS_H__

#include <stdint.h>

#include <libcli.h>
#include "signals.h"

struct Regs {
    uint16_t sp;
    uint16_t pc;
    uint16_t x;
    uint8_t a;
    uint8_t b;
    uint8_t cc;
    uint16_t getD() const { return (static_cast<uint16_t>(a) << 8) | b; }
    void setD(uint16_t d) {
        b = d;
        a = d >> 8;
    }

    void print() const;
    void save(bool show = false);
    void restore(bool show = false);
    void set(const Signals *stack);
    uint8_t cycles(uint8_t insn) const;
    const char *cpu() const;

    void printRegList() const;
    bool validUint8Reg(char reg) const;
    bool validUint16Reg(char reg) const;
    typedef libcli::Cli::State State;
    bool setRegValue(char reg, uint32_t value, State state);
};

extern Regs Regs;

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
