#ifndef __REGS_H__
#define __REGS_H__

#include <stdint.h>

#include <dis_memory.h>
#include "signals.h"

struct Regs {
    uint16_t sp;
    uint16_t pc;
    uint16_t y;
    uint16_t x;
    uint8_t a;
    uint8_t b;
    uint8_t cc;
    uint16_t getD() const { return (static_cast<uint16_t>(a) << 8) | b; }
    void setD(uint16_t d) {
        b = d;
        a = d >> 8;
    }

    void print() const;
    void save(bool show = false);
    void restore(bool show = false);
    void capture(const Signals *stack);

    const char *cpu() const;
    const char *cpuName() const;

    uint16_t nextIp() const { return pc; }
    uint32_t maxAddr() const { return UINT16_MAX; }
    void printRegList() const;
    char validUint8Reg(const char *word) const;
    char validUint16Reg(const char *word) const;
    char validUint32Reg(const char *word) const { return 0; }
    void setRegValue(char reg, uint32_t value);
    uint16_t disassemble(uint16_t addr, uint16_t numInsn) const;
};

extern Regs Regs;

struct Memory : public libasm::DisMemory {
public:
    Memory() : libasm::DisMemory(0) {}
    bool hasNext() const override { return address() < memory_size; }
    void setAddress(uint16_t addr) { resetAddress(addr); }

    void setRamDevBase(uint16_t ram, uint16_t dev);
    void disableCOP();
    void setINIT();

    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t data);
    void write(uint16_t addr, const uint8_t *data, uint8_t len);
    uint8_t internal_read(uint16_t addr) const;
    void internal_write(uint16_t addr, uint8_t data) const;
    uint8_t raw_read(uint16_t addr) const;
    void raw_write(uint16_t addr, uint8_t data);
    uint16_t raw_read_uint16(uint16_t addr) const;
    void raw_write_uint16(uint16_t addr, uint16_t data);

    // Internal device and RAM
    static constexpr auto dev_start = 0x00;
    static constexpr auto dev_end = dev_start + 64;
    static constexpr auto ram_start = 0x40;
    static constexpr auto ram_end = ram_start + 192;
    // Main memory
    static constexpr auto memory_size = 0x10000;
    static constexpr auto reset_vector = 0xFFFE;
    bool is_internal(uint16_t addr) const;

protected:
    uint8_t nextByte() { return read(address()); }

private:
    uint16_t ram_base;
    uint16_t dev_base;
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
