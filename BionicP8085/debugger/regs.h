#ifndef __REGS_H__
#define __REGS_H__

#include <stdint.h>

#include <dis_memory.h>

struct Regs {
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    uint8_t a;
    uint8_t psw;
    uint16_t pc;
    uint16_t sp;
    uint8_t ie;

    void setBc(uint16_t bc) {
        c = bc;
        b = bc >> 8;
    }

    void setDe(uint16_t de) {
        e = de;
        d = de >> 8;
    }

    void setHl(uint16_t hl) {
        l = hl;
        h = hl >> 8;
    }

    void print() const;
    void save(bool show = false);
    void restore(bool show = false);
    static uint8_t busCycles(uint8_t insn);
    static uint8_t insnLen(uint8_t insn);
    uint16_t effectiveAddr(uint8_t insn, uint16_t insnp) const;

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
    uint16_t assemble(uint16_t addr, const char *line) const;

    static constexpr uint8_t NOP = 0x00;
    static constexpr uint8_t RST_0 = 0xC7;
    static constexpr uint8_t HLT = 0x76;

private:
    uint16_t selectBase(uint8_t insn) const;
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
    uint8_t raw_read(uint16_t addr) const;
    void raw_write(uint16_t addr, uint8_t data);
    uint16_t raw_read_uint16(uint16_t addr) const;
    void raw_write_uint16(uint16_t addr, uint16_t data);

    static constexpr auto memory_size = 0x10000;

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
