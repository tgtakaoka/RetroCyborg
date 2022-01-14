#ifndef __SIGNALS_H__
#define __SIGNALS_H__

#include <stdint.h>

struct Signals {
    void getAddr();
    void getData();
    bool fetchVector() const { return _vector; }
    bool fetchInsn() const { return _insn; }
    bool busLock() const { return _lock; }
    bool waiting() const { return _waiting; }
    void clear();
    static void inject(uint8_t data);
    static void capture();
    void print() const;
    Signals &debug(char c);

    uint16_t addr;
    uint8_t data;
    uint8_t rw;

    bool readRam() const { return _inject == false; }
    bool writeRam() const { return _capture == false; }

    static void printCycles(const Signals *end = nullptr);
    static Signals &currCycle();
    static void resetCycles();
    static void nextCycle();

    enum MpuType : uint8_t {
        MOS6502 = 0,
        W65C02S = 1,
        W65C816S = 2,
    };
    static void checkHardwareType();
    static MpuType mpuType() { return _type; }
    static bool stopInsn(uint8_t insn) {
        // Check W65C's WAI and STP instructions.
        return _type != MOS6502 && (insn & ~(WAI ^ STP)) == WAI;
    }

private:
    bool _insn;
    bool _vector;
    bool _lock;
    bool _waiting;
    bool _inject;
    bool _capture;
    char _debug;

    static constexpr uint8_t MAX_CYCLES = 60;
    static uint8_t _cycles;
    static Signals _signals[MAX_CYCLES + 1];

    static MpuType _type;
    static constexpr uint8_t WAI = 0xCB;
    static constexpr uint8_t STP = 0xDB;
};
#endif /* __SIGNALS_H__ */

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
