#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <stdint.h>

#include "config.h"

struct Signals {
    Signals &getAddr();
    bool read() const { return _rw == 0; }
    bool write() const { return _rw != 0; }
    bool fetch() const { return _fetch; }
    bool ioAccess() const;
    void getData();
    void outData();
    void clear();
    void markFetch() { _fetch = true; }
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

    static void disassembleCycles();
    static void printCycles();
    static Signals &currCycle();
    static void resetCycles();
    static void resetTo(const Signals &);
    static void nextCycle();

private:
    uint8_t _rw;
    uint8_t _mio;
    bool _fetch;
    bool _inject;
    bool _capture;
    char _debug;

    static constexpr uint8_t MAX_CYCLES = 92;
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
