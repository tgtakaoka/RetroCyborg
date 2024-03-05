#include "mems_mc6809.h"
#include <asm_mc6809.h>
#include <dis_mc6809.h>
#include "devs_mc6809.h"
#include "regs_mc6809.h"

namespace debugger {
namespace mc6809 {

uint16_t MemsMc6809::read(uint32_t addr) const {
    return Devs.isSelected(addr) ? Devs.read(addr) : raw_read(addr);
}

void MemsMc6809::write(uint32_t addr, uint16_t data) const {
    if (Devs.isSelected(addr)) {
        Devs.write(addr, data);
    } else {
        raw_write(addr, data);
    }
}

#ifdef WITH_ASSEMBLER
libasm::Assembler *MemsMc6809::assembler() const {
    static auto as = new libasm::mc6809::AsmMc6809();
    as->setCpu(_regs->cpu());
    return as;
}
#endif

#ifdef WITH_DISASSEMBLER
libasm::Disassembler *MemsMc6809::disassembler() const {
    static auto dis = new libasm::mc6809::DisMc6809();
    dis->setCpu(_regs->cpu());
    return dis;
}
#endif

}  // namespace mc6809
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
