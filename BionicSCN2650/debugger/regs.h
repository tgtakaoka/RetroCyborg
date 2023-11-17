#ifndef __REGS_H__
#define __REGS_H__

#include <stdint.h>

#include <dis_memory.h>

enum RegSpace : uint8_t {
    SET_ONE = 0,    // %00~%FF
    SET_TWO = 1,    // %C0~%FF
    SET_BANK1 = 2,  // %E0-%FF
};

struct Regs {
    uint8_t read_reg(uint8_t addr, RegSpace space = SET_ONE);
    void write_reg(uint8_t addr, uint8_t val);

    void print() const;
    void reset(bool show = false);
    void save(bool show = false);
    void restore(bool show = false);
    static uint8_t insnLen(uint8_t insn);
    static uint8_t busCycles(uint8_t insn);

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
    static constexpr auto sfr_base = 213;
    static constexpr auto sfr_flags = sfr_base + 0;
    static constexpr auto sfr_rp0 = sfr_base + 1;
    static constexpr auto sfr_rp1 = sfr_base + 2;
    static constexpr auto sfr_sph = sfr_base + 3;
    static constexpr auto sfr_spl = sfr_base + 4;
    static constexpr auto sfr_iph = sfr_base + 5;
    static constexpr auto sfr_ipl = sfr_base + 6;
    uint8_t _sfr[7];
    uint8_t _r[16];

    void set_flags(uint8_t val) { set_sfr(sfr_flags, val); }
    void set_rp0(uint8_t val);
    void set_rp1(uint8_t val);
    void set_sp(uint16_t val);
    void set_ip(uint16_t val);
    void set_sfr(uint8_t name, uint8_t val);
    uint8_t get_sfr(uint8_t name) const { return _sfr[name - sfr_base]; }

    void update_r(uint8_t addr, uint8_t val);
    void save_r(uint8_t num);
    void restore_r(uint8_t num);
    void set_r(uint8_t num, uint8_t val);
    void set_rr(uint8_t num, uint16_t val) {
        set_r(num, hi(val));
        set_r(num + 1, lo(val));
    }

    static void switchBank(RegSpace space);
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
