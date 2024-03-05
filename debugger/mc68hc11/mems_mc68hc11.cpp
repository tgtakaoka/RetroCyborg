#include "mems_mc68hc11.h"
#include "debugger.h"
#include "devs_mc68hc11.h"
#include "mc68hc11_init.h"

namespace debugger {
namespace mc68hc11 {

bool MemsMc68hc11::is_internal(uint16_t addr) const {
    return _devs->init()->is_internal(addr);
}

uint16_t MemsMc68hc11::read(uint32_t addr) const {
    return _devs->isSelected(addr) ? _devs->read(addr) : raw_read(addr);
}

void MemsMc68hc11::write(uint32_t addr, uint16_t data) const {
    if (_devs->isSelected(addr)) {
        _devs->write(addr, data);
    } else {
        raw_write(addr, data);
    }
}

uint16_t MemsMc68hc11::get(uint32_t addr, const char *space) const {
    (void)space;
    if (is_internal(addr))
        return regs().internal_read(addr);
    return read(addr);
}

void MemsMc68hc11::put(uint32_t addr, uint16_t data, const char *space) const {
    if (is_internal(addr)) {
        regs().internal_write(addr, data);
    } else {
        write(addr, data);
    }
}

}  // namespace mc68hc11
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
