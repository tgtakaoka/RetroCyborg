
#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <stdint.h>

#include "config.h"

#ifndef _BV
#define _BV(n) (1 << (n))
#endif

class Signals {
public:
    void get();
    void print(const Signals *prev = nullptr) const;
    uint8_t dbus() const { return _dbus; }
    bool running() const { return (_pins & babs) == 0; }
    bool fetchingVector() const { return (_pins & babs) == bs; }
    bool halting() const { return (_pins & babs) == babs; }
    bool lastInstructionCycle() const { return _pins & lic; }
    bool advancedValidMemoryAddress() const { return _pins & avma; }
    bool readCycle(const Signals *prev) const {
        return prev && prev->advancedValidMemoryAddress() && (_pins & rw);
    }
    bool writeCycle(const Signals *prev) const {
        return prev && prev->advancedValidMemoryAddress() && (_pins & rw) == 0;
    }
    void debug(char c) { _debug = c; }

    static void printCycles();
    static Signals &currCycle();
    static void resetCycles();
    static void nextCycle();

private:
    enum : uint8_t {
#ifdef SIGNALS_BUS
        bs = _BV(BS_PIN),
        ba = _BV(BA_PIN),
        babs = _BV(BA_PIN) | _BV(BS_PIN),
        reset = _BV(RESET_PIN),
        halt = _BV(HALT_PIN),
        lic = _BV(LIC_PIN),
        avma = _BV(AVMA_PIN),
        rw = _BV(RD_WR_PIN),
        busy = _BV(BUSY_PIN),
#else
        bs = _BV(0),
        ba = _BV(1),
        babs = _BV(1) | _BV(0),
        reset = _BV(2),
        halt = _BV(3),
        lic = _BV(4),
        avma = _BV(5),
        rw = _BV(6),
        busy = _BV(7),
#endif
    };
    uint8_t _pins;
    uint8_t _dbus;
    char _debug;

    static constexpr auto MAX_CYCLES = 40;
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
