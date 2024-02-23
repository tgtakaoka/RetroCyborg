#include "signals_mc6809.h"
#include "debugger.h"
#include "digital_bus.h"
#include "pins_mc6809.h"
#include "string_util.h"

namespace debugger {
namespace mc6809 {

void Signals::getHighAddr() {
    addr = busRead(AM) | busRead(AH);
}

void Signals::getLowAddr() {
    addr |= busRead(AL);
}

void Signals::getDirection() {
    rw() = digitalReadFast(PIN_RW);
    uint8_t s = digitalReadFast(PIN_BS) ? 1 : 0;
    if (digitalReadFast(PIN_BA))
        s += 2;
    status() = s;
}

void Signals::clearControl() {
    lic() = 0;
    avma() = 0;
    vma() = 1;
}

void Signals::getData() {
    data = busRead(D);
}

void Signals::print() const {
    LOG_MATCH(cli.printDec(pos(), 3));
    LOG_MATCH(if (fetch()) cli.printDec(matched(), 3); else cli.print("   "));
    LOG_MATCH(cli.print(' '));
    static char buffer[] = {
            ' ',                        // status=0
            'W',                        // rw=1
            ' ', 'A', '=', 0, 0, 0, 0,  // addr=5
            ' ',                        // valid=9
            'D', '=', 0, 0,             // data=12
            ' ', 'L',                   // lic=15
            0,                          // eos
    };
    static constexpr char STATUS[] = {' ', 'V', 'S', 'H'};
    buffer[0] = STATUS[status() & 3];
    buffer[1] = write() ? 'W' : 'R';
    buffer[15] = lic() ? 'L' : ' ';
    outHex16(buffer + 5, addr);
    outHex8(buffer + 12, data);
    cli.println(buffer);
}

}  // namespace mc6809
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
