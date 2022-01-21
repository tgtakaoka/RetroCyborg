#ifndef __REGS_H__
#define __REGS_H__

#include <stdint.h>

#include <dis_memory.h>
#include "signals.h"

struct Regs {
    uint8_t d;
    uint8_t x;
    uint8_t p;
    uint8_t t;
    uint8_t q;
    uint8_t df;
    uint8_t ie;
    uint16_t r[16];
    bool dirty[16];

    void print() const;
    void save(bool show = false);
    void restore(bool show = false);
    void reset();

    const char *cpu() const;
    const char *cpuName() const;
    bool is1804() const;

    uint16_t nextIp() const { return r[p]; }
    uint32_t maxAddr() const { return UINT16_MAX; }
    void printRegList() const;
    char validUint8Reg(const char *word) const;
    char validUint16Reg(const char *word) const;
    char validUint32Reg(const char *word) const { return 0; }
    void setRegValue(char reg, uint32_t value);
    uint16_t disassemble(uint16_t addr, uint16_t numInsn) const;
    uint16_t assemble(uint16_t addr, const char *line) const;

private:
    const char *_cpuType;

    void setCpuType();
};

extern Regs Regs;

struct Memory : public libasm::DisMemory {
public:
    Memory() : libasm::DisMemory(0) {}
    bool hasNext() const override { return address() < memory_size; }
    void setAddress(uint16_t addr) { resetAddress(addr); }

    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t data);
    void write(uint16_t addr, const uint8_t *data, uint8_t len);
    uint8_t internal_read(uint8_t addr) const;
    void internal_write(uint8_t addr, uint8_t data) const;
    uint8_t raw_read(uint16_t addr) const;
    void raw_write(uint16_t addr, uint8_t data);
    uint16_t raw_read_uint16(uint16_t addr) const;
    void raw_write_uint16(uint16_t addr, uint16_t data);

    static constexpr auto memory_size = 0x10000;
    static bool is_internal(uint16_t addr);

protected:
    uint8_t nextByte() { return read(address()); }

private:
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
