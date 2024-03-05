#ifndef __INST_MC68HC11_H__
#define __INST_MC68HC11_H__

#include "mc6800/inst_mc6800.h"

namespace debugger {
namespace mc68hc11 {

using mc6800::InstMc6800;

struct InstMc68hc11 final : InstMc6800 {
    InstMc68hc11(const DmaMemory *mems) : InstMc6800(mems) {}

    uint16_t vec_swi() const override { return 0xFFF6; }
};

extern struct InstMc68hc11 Inst;

}  // namespace mc68hc11
}  // namespace debugger
#endif
// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
