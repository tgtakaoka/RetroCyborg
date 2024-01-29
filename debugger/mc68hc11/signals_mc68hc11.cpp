#include "signals_mc68hc11.h"
#include "debugger.h"
#include "digital_bus.h"
#include "pins_mc68hc11.h"

namespace debugger {
namespace mc68hc11 {

void Signals::getMuxedAddr() {
    addr = busRead(AD);
    vma() = 1;
}

void Signals::getHighAddr() {
    rw() = digitalReadFast(PIN_RW);
    addr |= busRead(AH);
}

void Signals::getLoadInstruction() {
    lir() = digitalReadFast(PIN_LIR) == LOW;
}

void Signals::getData() {
    data = busRead(AD);
}

}  // namespace mc68hc11
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
