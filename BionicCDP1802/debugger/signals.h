#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <stdint.h>

struct Signals {
    enum Status : uint8_t {
        SC_FETCH = 0,
        SC_EXEC = 1,
        SC_DMA = 2,
        SC_INT = 3,
    };

    void getStatus();
    bool fetchInsn() const { return sc == SC_FETCH; }
    void getAddr1();
    void getAddr2();
    void getDirection();
    void getData();
    void clear();
    static Signals &inject(uint8_t data);
    static void capture();
    void print() const;
    Signals &debug(char c);
    bool insnFetch() const { return sc == 0; }

    Status sc;
    uint16_t addr;
    uint8_t data;
    uint8_t n;
    uint8_t mrd;
    uint8_t mwr;

    bool readRam() const { return _inject == false; }
    bool writeRam() const { return _capture == false; }

    static void disassembleCycles();
    static void printCycles();
    static Signals &currCycle();
    static void resetCycles();
    static void nextCycle();

private:
    bool _inject;
    bool _capture;
    char _debug;

    static constexpr uint8_t MAX_CYCLES = 128;
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
