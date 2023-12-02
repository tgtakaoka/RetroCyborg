#ifndef __REGS_H__
#define __REGS_H__

#include <stdint.h>

#include <dis_memory.h>

#include "signals.h"

struct Regs {
    uint16_t _pc0;
    uint16_t _pc1;
    uint16_t _dc0;
    uint16_t _dc1;

    bool romc_read(Signals &signals);
    bool romc_write(Signals &signals);
    uint8_t read_reg(uint8_t addr);
    void write_reg(uint8_t addr, uint8_t val);
    uint8_t read_io(uint8_t addr);
    void write_io(uint8_t addr, uint8_t val);

    void print() const;
    void save(bool show = false);
    void restore(bool show = false);
    static uint8_t insnLen(uint8_t insn);
    static uint8_t busCycles(uint8_t insn);

    const char *cpu() const;
    const char *cpuName() const;

    uint16_t nextIp() const { return _pc0; }
    uint32_t maxAddr() const { return UINT16_MAX; }
    void printRegList() const;
    char validUint8Reg(const char *word) const;
    char validUint16Reg(const char *word) const;
    char validUint32Reg(const char *word) const { return 0; }
    void setRegValue(char reg, uint32_t value);

    static constexpr uint8_t BR = 0x90;
    static constexpr uint8_t UNKN = 0x2F;

private:
    uint8_t _isar;
    uint8_t _a;
    uint8_t _w;
    uint8_t _r[16];
    uint8_t _ioaddr;
    uint16_t _delay;

    static constexpr uint8_t HU = 10;
    static constexpr uint8_t HL = 11;
    static constexpr uint8_t KU = 12;
    static constexpr uint8_t KL = 13;
    static constexpr uint8_t QU = 14;
    static constexpr uint8_t QL = 15;

    static constexpr uint8_t hi(const uint16_t v) {
        return static_cast<uint8_t>(v >> 8);
    }
    static constexpr uint8_t lo(const uint16_t v) {
        return static_cast<uint8_t>(v);
    }
    static constexpr uint16_t uint16(const uint8_t hi, const uint8_t lo) {
        return static_cast<uint16_t>(hi) << 8 | lo;
    }

    uint8_t isl() const { return _isar & 7; }
    uint8_t isu() const { return (_isar >> 3) & 7; }
    void set_isl(uint8_t val) { _isar = isu() | (val & 7); }
    void set_isu(uint8_t val) { _isar = ((val & 7) << 3) | isl(); }
    void update_r(uint8_t num, uint8_t val);
};

extern Regs Regs;

struct Memory : libasm::DisMemory {
    Memory() : libasm::DisMemory(0) {}
    bool hasNext() const override { return address() < memory_size; }
    void setAddress(uint16_t addr) { resetAddress(addr); }

    uint8_t read(uint16_t addr, const char *space = nullptr) const;
    void write(uint16_t addr, uint8_t data, const char *space = nullptr);
    void write(uint16_t addr, const uint8_t *data, uint8_t len);

    static constexpr auto memory_size = 0x10000;

    uint16_t disassemble(uint16_t addr, uint16_t numInsn);
    uint16_t assemble(uint16_t addr, const char *line);

private:
    uint8_t nextByte() { return read(address()); }

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
