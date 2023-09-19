#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <stdint.h>

#include "config.h"

struct Signals {
    void getAddr();
    bool read() const { return rw != 0; }
    bool write() const { return rw == 0; }
    void getData();
    void outData();
    void clear();
    Signals &inject(uint8_t data);
    void capture();
    void print() const;
    Signals &debug(char c);

    uint16_t addr;
    uint8_t data;
    uint8_t rw;
#if Z8_DATA_MEMORY == 1
    uint8_t dm;
#endif

    bool readRam() const {
        return _inject == false;
    }
    bool writeRam() const {
        return _capture == false;
    }

    static void printCycles();
    static void disassembleCycles();
    static Signals &currCycle();
    static void resetCycles();
    static void resetTo(const Signals &);
    static void nextCycle();

private:
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
