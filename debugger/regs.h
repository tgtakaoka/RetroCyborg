#ifndef __DEBUGGER_REGS_H__
#define __DEBUGGER_REGS_H__

#include <stdint.h>

namespace debugger {
struct Regs {
    virtual const char *cpu() const = 0;
    virtual const char *cpuName() const = 0;

    virtual void print() const = 0;
    virtual void save() = 0;
    virtual void restore() = 0;

    virtual uint32_t nextIp() const = 0;
    uint8_t validRegister(const char *word, uint32_t &max) const;
    virtual void helpRegisters() const = 0;
    virtual void setRegister(uint8_t reg, uint32_t value) {}

protected:
    struct RegList {
        const char *const *head;
        uint8_t num;
        uint8_t name;
        uint32_t max;
    };
    virtual const RegList *listRegisters(uint8_t n) const = 0;

    static constexpr uint8_t hi(uint16_t v) {
        return static_cast<uint8_t>(v >> 8);
    }
    static constexpr uint8_t lo(uint16_t v) {
        return static_cast<uint8_t>(v >> 0);
    }
    static constexpr uint16_t uint16(uint8_t hi, uint8_t lo) {
        return static_cast<uint16_t>(hi) << 8 | lo;
    }
    static constexpr uint16_t le16(uint8_t *p) { return uint16(p[1], p[0]); }
    static constexpr uint16_t be16(uint8_t *p) { return uint16(p[0], p[1]); }
    static constexpr char bit1(uint8_t val, char c) { return val ? c : '_'; }
    static void le16(uint8_t *p, uint16_t v) {
        p[0] = lo(v);
        p[1] = hi(v);
    }
    static void be16(uint8_t *p, uint16_t v) {
        p[0] = hi(v);
        p[1] = lo(v);
    }
    static void swap8(uint8_t &a, uint8_t &b) {
        auto tmp = a;
        a = b;
        b = tmp;
    }
};
}  // namespace debugger

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
