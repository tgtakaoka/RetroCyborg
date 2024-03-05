#include "signals_mc146805e2.h"
#include "debugger.h"
#include "digital_bus.h"
#include "pins_mc146805e2.h"

namespace debugger {
namespace mc146805e2 {

void Signals::getDirection() {
    rw() = digitalReadFast(PIN_RW);
}

void Signals::getMuxedAddr() {
    addr = busRead(B);
}

void Signals::getHighAddr() {
    addr |= busRead(AH);
}

void Signals::getLoadInstruction() {
    li() = digitalReadFast(PIN_LI);
}

void Signals::getData() {
    data = busRead(B);
}

}  // namespace mc146805e2
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
