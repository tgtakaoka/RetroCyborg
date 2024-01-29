#include "signals_mc6801.h"
#include "debugger.h"
#include "digital_bus.h"
#include "pins_mc6801.h"

namespace debugger {
namespace mc6801 {

void Signals::getMuxedAddr() {
    addr = busRead(AD);
    vma() = 1;
    clearFetch();
}

void Signals::getDirection() {
    addr |= busRead(AH);
    rw() = digitalReadFast(PIN_RW);
}

void Signals::getData() {
    data = busRead(AD);
}

}  // namespace mc6801
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
