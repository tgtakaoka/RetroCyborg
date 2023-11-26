#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <stdint.h>

struct Signals {
    void getAddress();
    void getDirection();
    void getData();
    bool fetchInsn() const { return iom == 0 && s == S_FETCH; }
    void clear();
    Signals &inject(uint8_t data);
    void capture();
    void print() const;
    Signals &debug(char c);
    Signals &setAddress(uint16_t addr);

    uint16_t addr;
    uint8_t data;
    uint8_t iom;
    uint8_t wr;
    uint8_t rd;
    uint8_t inta;

    bool readRam() const { return _inject == false; }
    bool writeRam() const { return _capture == false; }

    static void disassembleCycles();
    static void printCycles();
    static Signals &currCycle();
    static void resetCycles();
    static void nextCycle();

private:
    enum Status : uint8_t {
        S_HALT = 0,   // HALT
        S_WRITE = 1,  // MW or IOW
        S_READ = 2,   // MR or IOR
        S_FETCH = 3,  // OF or INA
    };

    Status s;
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
