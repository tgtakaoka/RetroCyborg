#ifndef __DEBUGGER_PINS_H__
#define __DEBUGGER_PINS_H__

#include <stdint.h>

namespace debugger {
struct Pins {
    Pins();

    virtual void idle() = 0;
    virtual bool step(bool show) = 0;
    virtual void run() = 0;
    virtual void assertInt(uint8_t name = 0) = 0;
    virtual void negateInt(uint8_t name = 0) = 0;

    void resetPins();
    bool setBreakPoint(uint32_t addr);
    bool clearBreakPoint(uint8_t index);
    bool printBreakPoints() const;

    // Control USER LED
    void setRun() const;
    void setHalt() const;

private:
    static constexpr auto BREAK_POINTS = 4;
    uint8_t _breakNum;
    uint32_t _breakPoints[BREAK_POINTS];
    uint16_t _breakInsts[BREAK_POINTS];

protected:
    virtual void reset() = 0;
    virtual void setBreakInst(uint32_t addr) const = 0;

    void saveBreakInsts();
    void restoreBreakInsts();
    bool isBreakPoint(uint32_t addr) const;

    static bool haltSwitch();

    static void pinsMode(const uint8_t *pins, uint8_t size, uint8_t mode);
    static void pinsMode(
            const uint8_t *pins, uint8_t size, uint8_t mode, uint8_t val);
    static void assert_debug();
    static void negate_debug();
    static void toggle_debug();

    static constexpr uint8_t hi(uint16_t v) {
        return static_cast<uint8_t>(v >> 8);
    }
    static constexpr uint8_t lo(uint16_t v) {
        return static_cast<uint8_t>(v >> 0);
    }
    static constexpr uint16_t uint16(uint8_t hi, uint8_t lo) {
        return static_cast<uint16_t>(hi) << 8 | lo;
    }
};
}  // namespace debugger

#endif /* __PINS_H__ */

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
