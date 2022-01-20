#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <stdint.h>

struct Signals {
    bool getDirection();
    bool read() const { return rds == 0; }
    bool write() const { return wds == 0; }
    void getAddr();
    void getData();
    void clear();
    static void inject(uint8_t data);
    static void capture();
    void print() const;
    Signals &debug(char c);

    uint16_t addr;
    uint8_t data;
    uint8_t rds;
    uint8_t wds;

    bool readRam() const { return _inject == false; }
    bool writeRam() const { return _capture == false; }

    static void printCycles(const Signals *end = nullptr);
    static Signals &currCycle();
    static void resetCycles();
    static void nextCycle();

private:
    bool _inject;
    bool _capture;
    char _debug;

    static constexpr uint8_t MAX_CYCLES = 60;
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
