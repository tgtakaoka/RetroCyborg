#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <stdint.h>

struct Signals {
    bool getAddr();
    bool read() const { return (flags & F_READ) != 0; }
    bool write() const { return (flags & F_READ) == 0; }
    bool fetchInsn() const { return (flags & F_INSN) != 0; }
    bool delay() const { return (flags & F_DELAY) != 0; }
    bool halt() const { return (flags & F_HALT) != 0; }
    void getData();
    void clear();
    static void inject(uint8_t data);
    static void capture();
    void print() const;
    Signals &debug(char c);

    uint16_t addr;
    uint8_t flags;
    uint8_t data;

    enum Flags : uint8_t {
        F_READ = 0x10,
        F_INSN = 0x20,
        F_DELAY = 0x40,
        F_HALT = 0x80,
    };

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
