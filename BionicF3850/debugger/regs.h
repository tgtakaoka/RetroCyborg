#ifndef __REGS_H__
#define __REGS_H__

#include <stdint.h>

#include <dis_memory.h>

#include "signals.h"

struct Regs {
    void print() const;
    void reset(bool show = false);
    void save(bool show = false);
    void restore(bool show = false);
    void interrupt(const Signals &signals, bool nmi);
    void breakPoint(const Signals &signals);

    const char *cpu() const;
    const char *cpuName() const;

    uint16_t nextIp() const { return _pc; }
    uint32_t maxAddr() const { return UINT16_MAX; }
    void printRegList() const;
    char validUint8Reg(const char *word) const;
    char validUint16Reg(const char *word) const;
    char validUint32Reg(const char *word) const { return 0; }
    void setRegValue(char reg, uint32_t value);

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

    void saveRegs();

    static constexpr uint8_t hi(const uint16_t v) {
        return static_cast<uint8_t>(v >> 8);
    }
    static constexpr uint8_t lo(const uint16_t v) {
        return static_cast<uint8_t>(v);
    }
    static constexpr uint16_t r16(const uint8_t hi, const uint8_t lo) {
        return static_cast<uint16_t>(hi) << 8 | lo;
    }
};

extern Regs Regs;

struct Memory : libasm::DisMemory {
    Memory() : libasm::DisMemory(0) {}
    bool hasNext() const override { return address() < memory_size; }
    void setAddress(uint16_t addr) { resetAddress(addr); }

    uint8_t read(uint16_t addr, const char *space = nullptr) const;
    void write(uint16_t addr, uint8_t data, const char *space = nullptr);
    uint8_t raw_read(uint16_t addr) const;
    void raw_write(uint16_t addr, uint8_t data);
    uint8_t internal_read(uint16_t addr) const;
    void internal_write(uint16_t addr, uint8_t data) const;
    void write(uint16_t addr, const uint8_t *data, uint8_t len);

    static constexpr auto memory_size = 0x10000;
    static bool is_internal(uint16_t addr);

    uint16_t disassemble(uint16_t addr, uint16_t numInsn);
    uint16_t assemble(uint16_t addr, const char *line);

private:
    uint8_t nextByte() { return raw_read(address()); }

    uint8_t memory[memory_size];
};

extern Memory Memory;

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
