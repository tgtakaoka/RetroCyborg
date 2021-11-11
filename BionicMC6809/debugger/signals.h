#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <stdint.h>

#define DEBUG_SIGNALS

struct Signals {
    Signals &get();
    Signals &readAddr();
    Signals &readData();
    Signals &clear();
    static Signals &inject(uint8_t data);
    static Signals &capture();
    void print() const;
    Signals &debug(char c);

    uint16_t addr;
    uint8_t data;
    uint8_t rw;
    uint8_t ba;
    uint8_t bs;
    uint8_t halt;

    bool readRam() const { return _inject == false; }
    bool writeRam() const { return _capture == false; }

    static void printCycles(const Signals *end = nullptr);
    static Signals &currCycle();
    static Signals &resetCycles();
    static Signals &nextCycle();
    static void flushWrites(const Signals *end);

private:
    bool _inject;
    bool _capture;

#ifdef DEBUG_SIGNALS
    char _debug;
#endif

    static constexpr uint8_t MAX_CYCLES = 40;
    static uint8_t _cycles;
    static Signals _signals[MAX_CYCLES + 1];
};
#endif /* __SIGNALS_H__ */

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
