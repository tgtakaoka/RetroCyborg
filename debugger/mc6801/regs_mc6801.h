#ifndef __REGS_MC6801_H__
#define __REGS_MC6801_H__

#include "mc6800/regs_mc6802.h"

namespace debugger {
namespace mc6801 {

using mc6800::PinsMc6800;
using mc6802::RegsMc6802;

struct RegsMc6801 final : RegsMc6802 {
    RegsMc6801(PinsMc6800 *pins) : RegsMc6802(pins) {}

    void helpRegisters() const override;
    const RegList *listRegisters(uint8_t n) const override;
    void setRegister(uint8_t reg, uint32_t value) override;

    SoftwareType checkSoftwareType() override;

protected:
    void _d(uint16_t d) {
        _b = lo(d);
        _a = hi(d);
    }
};

extern struct RegsMc6801 Regs;

}  // namespace mc6801
}  // namespace debugger
#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
