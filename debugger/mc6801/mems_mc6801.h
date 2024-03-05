#ifndef __MEMS_MC6801_H__
#define __MEMS_MC6801_H__

#include "mc6800/mems_mc6800.h"
#include "regs_mc6801.h"

namespace debugger {
namespace mc6801 {

using mc6800::MemsMc6800;

struct MemsMc6801 final : MemsMc6800 {
    MemsMc6801(RegsMc6801 *regs) : MemsMc6800(regs) {}

    uint16_t read(uint32_t addr) const override;
    void write(uint32_t addr, uint16_t data) const override;

    uint16_t get(uint32_t addr, const char *space = nullptr) const override;
    void put(uint32_t addr, uint16_t data,
            const char *space = nullptr) const override;

private:
    RegsMc6801 &regs() const { return *static_cast<RegsMc6801 *>(_regs); }

    static constexpr uint16_t RAMCR = 0x0014;
    static constexpr uint8_t RAME_bm = 0x40;
};

extern struct MemsMc6801 Memory;

}  // namespace mc6801
}  // namespace debugger
#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
