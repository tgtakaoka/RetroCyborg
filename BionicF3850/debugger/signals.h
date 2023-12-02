#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <stdint.h>

#include "config.h"

struct Signals {
    Signals &getRomc();
    Signals &getData();
    void outData();
    void clear();
    bool read() const { return _read; }
    bool write() const { return _write; }
    bool fetch() const { return _fetch; }
    void markFetch() { _fetch = true; }
    void markRead(uint16_t addr);
    void markWrite(uint16_t addr);
    void markIoRead(uint8_t addr);
    void markIoWrite(uint8_t addr);
    Signals &inject(uint8_t data);
    void capture();
    Signals &debug(char c);
    void print() const;

    uint16_t addr;
    uint8_t data;
    uint8_t romc;

    bool readRam() const { return _inject == false; }
    bool writeRam() const { return _capture == false; }

    static void printCycles();
    static void disassembleCycles();
    static Signals &currCycle();
    static void resetTo(const Signals &to);
    static void resetCycles();
    static void nextCycle();
    const Signals &prev(uint8_t backward = 1) const;
    const Signals &next(uint8_t forward = 1) const;
    uint8_t diff(const Signals &s) const;

private:
    bool _read;
    bool _write;
    bool _fetch;
    bool _io;
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
