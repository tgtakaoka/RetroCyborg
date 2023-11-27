#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <stdint.h>

#include "config.h"

struct Signals {
    Signals &getAddr();
    bool read() const { return _rd == 0; }
    bool write() const { return _wr == 0; }
    void getData();
    void outData();
    void clear();
    Signals &inject(uint8_t data);
    void capture();
    void print() const;
    Signals &debug(char c);

    uint16_t addr;
    uint8_t data;

    bool readRam() const {
        return _inject == false;
    }
    bool writeRam() const {
        return _capture == false;
    }

    static void printCycles();
    static void disassembleCycles(const Signals &end);
    static Signals &currCycle();
    static void resetCycles();
    static void nextCycle();
    const Signals &prev(uint8_t backward = 1) const;
    const Signals &next(uint8_t forward = 1) const;
    uint8_t diff(const Signals &s) const;

private:
    uint8_t _rd;
    uint8_t _wr;
    bool _inject;
    bool _capture;
    char _debug;

    static constexpr uint8_t MAX_CYCLES = 64;
    static uint8_t _put;
    static uint8_t _get;
    static uint8_t _cycles;
    static Signals _signals[MAX_CYCLES];
};
#endif /* __SIGNALS_H__ */

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
