#ifndef __REGS_H__
#define __REGS_H__

#include <stdint.h>

#include <dis_memory.h>

struct Regs {
    void print() const;
    void reset(bool show = false);
    void save(bool show = false);
    void restore(bool show = false);
    static uint8_t insnLen(uint8_t insn);
    static uint8_t busCycles(uint8_t insn);
    static bool hasIndirect(uint8_t insn);

    const char *cpu() const;
    const char *cpuName() const;

    uint16_t nextIp() const { return _pc; }
    uint32_t maxAddr() const { return UINT16_MAX; }
    void printRegList() const;
    char validUint8Reg(const char *word) const;
    char validUint16Reg(const char *word) const;
    char validUint32Reg(const char *word) const { return 0; }
    void setRegValue(char reg, uint32_t value);
    uint16_t disassemble(uint16_t addr, uint16_t numInsn) const;
    uint16_t assemble(uint16_t addr, const char *line) const;

private:
    uint16_t _pc;
    uint8_t _psu;
    uint8_t _psl;
    uint8_t rs() const { return (_psl & 0x10) ? 1 : 0; }
    uint8_t _r0;
    uint8_t _r[/*rs*/2][4];

    static constexpr uint8_t hi(const uint16_t v) {
        return static_cast<uint8_t>(v >> 8);
    }
    static constexpr uint8_t lo(const uint16_t v) {
        return static_cast<uint8_t>(v);
    }
};

extern Regs Regs;

struct Memory : public libasm::DisMemory {
public:
    Memory() : libasm::DisMemory(0) {}
    bool hasNext() const override { return address() < memory_size; }
    void setAddress(uint16_t addr) { resetAddress(addr); }

    uint8_t read(uint16_t addr, const char *space = nullptr) const;
    void write(uint16_t addr, uint8_t data, const char *space = nullptr);
    void write(uint16_t addr, const uint8_t *data, uint8_t len);

    static constexpr auto memory_size = 0x8000;

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
