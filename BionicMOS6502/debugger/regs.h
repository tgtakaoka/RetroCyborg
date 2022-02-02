#ifndef __REGS_H__
#define __REGS_H__

#include <stdint.h>

#include <dis_memory.h>
#include "signals.h"

struct Regs {
    uint16_t pc;
    uint16_t d;
    uint16_t s;
    uint16_t x;
    uint16_t y;
    uint8_t a;
    uint8_t b;
    uint8_t p;
    uint8_t e;
    uint8_t pbr;
    uint8_t dbr;
    uint16_t getC() const { return (static_cast<uint16_t>(b) << 8) | a; }
    void setC(uint16_t c) {
        a = c;
        b = c >> 8;
    }

    void print() const;
    void save(bool show = false);
    void restore(bool show = false);
    void reset();

    const char *cpu() const;
    const char *cpuName() const;

    bool longa() const { return e == 0 && (p & 0x20) == 0; }
    bool longx() const { return e == 0 && (p & 0x10) == 0; }

    uint32_t nextIp() const { return pc; }
    uint32_t maxAddr() const { return is65816() ? 0xFFFFFF : UINT16_MAX; }
    void printRegList() const;
    char validUint8Reg(const char *word) const;
    char validUint16Reg(const char *word) const;
    char validUint32Reg(const char *word) const { return 0; }
    void setRegValue(char reg, uint32_t value);
    uint32_t disassemble(uint32_t addr, uint16_t numInsn) const;

private:
    const char *_cpuType;

    void setCpuType();
    bool is65816() const;
    void save65816(bool show);
    void restore65816(bool show);
};

extern Regs Regs;

struct Memory : public libasm::DisMemory {
public:
    Memory() : libasm::DisMemory(0) {}
    bool hasNext() const override { return address() < memory_size; }
    void setAddress(uint32_t addr) { resetAddress(addr); }

    uint8_t read(uint32_t addr) const;
    void write(uint32_t addr, uint8_t data);
    void write(uint32_t addr, const uint8_t *data, uint8_t len);
    uint8_t raw_read(uint32_t addr) const;
    void raw_write(uint32_t addr, uint8_t data);
    uint16_t raw_read_uint16(uint32_t addr) const;
    void raw_write_uint16(uint32_t addr, uint16_t data);

    static constexpr auto memory_size = 0x1000000;
    static constexpr uint32_t reset_vector = 0xFFFC;
    static bool is_internal(uint32_t addr);

protected:
    uint8_t nextByte() { return read(address()); }
};

extern Memory Memory;

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
