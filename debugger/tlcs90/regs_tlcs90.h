#ifndef __REGS_TLCS90_H__
#define __REGS_TLCS90_H__

#include "regs.h"
#include "signals_tlcs90.h"

namespace debugger {
namespace tlcs90 {

struct RegsTlcs90 final : Regs {
    const char *cpu() const override;
    const char *cpuName() const override;

    void print() const override;
    void save() override;
    void restore() override;

    void reset();
    bool saveContext(const Signals *frame);
    void saveRegisters();
    void setIp(uint16_t pc) { _pc = pc; }
    void offsetStack(int16_t delta) { _sp += delta; }

    uint32_t nextIp() const override { return _pc; }
    void helpRegisters() const override;
    const RegList *listRegisters(uint8_t n) const override;
    void setRegister(uint8_t reg, uint32_t value) override;

    static uint8_t internal_read(uint16_t addr);
    static void internal_write(uint16_t addr, uint8_t data);

private:
    uint16_t _pc;
    uint16_t _sp;
    uint16_t _ix;
    uint16_t _iy;
    struct reg {
        uint8_t a;
        uint8_t f;
        uint8_t b;
        uint8_t c;
        uint8_t d;
        uint8_t e;
        uint8_t h;
        uint8_t l;
        uint16_t bc() const { return r16(b, c); }
        uint16_t de() const { return r16(d, e); }
        uint16_t hl() const { return r16(h, l); }
        void setbc(uint16_t v) {
            b = hi(v);
            c = lo(v);
        }
        void setde(uint16_t v) {
            d = hi(v);
            e = lo(v);
        }
        void sethl(uint16_t v) {
            h = hi(v);
            l = lo(v);
        }
    } _main, _alt;

    void outFlags(char *, uint8_t) const;

    static constexpr uint16_t r16(const uint8_t hi, const uint8_t lo) {
        return static_cast<uint16_t>(hi) << 8 | lo;
    }

    static void setle16(uint8_t *p, uint8_t h, uint8_t l) {
        p[0] = l;
        p[1] = h;
    }
    static void setle16(uint8_t *p, uint16_t v) { setle16(p, hi(v), lo(v)); }
};

extern struct RegsTlcs90 Regs;

}  // namespace tlcs90
}  // namespace debugger
#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
