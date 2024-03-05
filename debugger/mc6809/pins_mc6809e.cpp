#include "pins_mc6809e.h"
#include "debugger.h"
#include "devs_mc6809.h"
#include "digital_bus.h"
#include "inst_mc6809.h"
#include "mems_mc6809.h"
#include "regs_mc6809.h"
#include "signals_mc6809e.h"

namespace debugger {
namespace mc6809e {

/**
 * MC6809E bus cycle.
 *               ___________             ___________
 *     Q _______|c2         |c4_________|c2         |c4_________|
 *              \      ___________      \      ___________
 *     E |c1__________|c3         |c1_________|c3         |c1____
 *       \      __________________\                       \
 *   R/W _>----<                   |_______________________>-----
 *                           ____               _________
 *  Data -------------------<rrrr>-------------<wwwwwwwww>-------
 *       _________      _________________       _________________
 *  AVMA _________>----<_________________>-----<_________________>
 */

namespace {

// tCYC: min 1,000 ns ; E/Q cycle
// PWEL: min   450 ns ; e_lo
// PWEH: min   450 ns ; e_hi
// PWQH: min   450 ns ; q_hi
//  tEQ: min   200 ns ; e to q
//  tAD: max   200 ns ; e_lo to addr/ba/bs/rw
//  tCD: max   300 ns ; q_hi to avma/lic/busy
// tDDQ: max   200 ns ; q_hi to write data
// tDSR: min    80 ns ; data to e_lo
// tPCS: min   200 ns ; reset_hi to q_lo
constexpr auto c1_ns = 146;      // 250 EQ=LL
constexpr auto c2_ns = 90;       // 250 EQ=LH
constexpr auto c3_novma = 180;   // 250 EQ=HH
constexpr auto c4_novma = 132;   // 250 EQ=HL
constexpr auto c3_write = 162;   // 250 EQ=HH
constexpr auto c4_write = 34;    // 250 EQ=HL
constexpr auto c4_capture = 42;  // 250 EQ=HL
constexpr auto c3_read = 92;     // 250 EQ=HH
constexpr auto c3_inject = 178;  // 250 EQ=HH
constexpr auto c4_read = 84;     // 250 EQ=HL

inline void c1_clock() __attribute__((always_inline));
inline void c1_clock() {
    digitalWriteFast(PIN_E, LOW);
}

inline void c2_clock() __attribute__((always_inline));
inline void c2_clock() {
    digitalWriteFast(PIN_Q, HIGH);
}

inline void c3_clock() __attribute__((always_inline));
inline void c3_clock() {
    digitalWriteFast(PIN_E, HIGH);
}

inline void c4_clock() __attribute__((always_inline));
inline void c4_clock() {
    digitalWriteFast(PIN_Q, LOW);
}

void negate_irq() {
    digitalWriteFast(PIN_IRQ, HIGH);
}

void negate_firq() {
    digitalWriteFast(PIN_FIRQ, HIGH);
}

void negate_nmi() {
    digitalWriteFast(PIN_NMI, HIGH);
}

void negate_halt() {
    digitalWriteFast(PIN_HALT, HIGH);
}

void negate_tsc() {
    digitalWriteFast(PIN_TSC, LOW);
}

void assert_reset() {
    // Drive RESET condition
    digitalWriteFast(PIN_RESET, LOW);
    negate_halt();
    negate_nmi();
    negate_irq();
    negate_firq();
    negate_tsc();
}

void negate_reset() {
    // Release RESET conditions
    digitalWriteFast(PIN_RESET, HIGH);
}

constexpr uint8_t PINS_LOW[] = {
        PIN_E,
        PIN_Q,
        PIN_RESET,
        PIN_TSC,
};

constexpr uint8_t PINS_HIGH[] = {
        PIN_HALT,
        PIN_IRQ,
        PIN_NMI,
        PIN_FIRQ,
};

constexpr uint8_t PINS_INPUT[] = {
        PIN_D0,
        PIN_D1,
        PIN_D2,
        PIN_D3,
        PIN_D4,
        PIN_D5,
        PIN_D6,
        PIN_D7,
        PIN_AL0,
        PIN_AL1,
        PIN_AL2,
        PIN_AL3,
        PIN_AL4,
        PIN_AL5,
        PIN_AL6,
        PIN_AL7,
        PIN_AM8,
        PIN_AM9,
        PIN_AM10,
        PIN_AM11,
        PIN_AH12,
        PIN_AH13,
        PIN_AH14,
        PIN_AH15,
        PIN_RW,
        PIN_BS,
        PIN_BA,
        PIN_LIC,
        PIN_AVMA,
        PIN_BUSY,
};

}  // namespace

void PinsMc6809E::resetPins() {
    pinsMode(PINS_LOW, sizeof(PINS_LOW), OUTPUT, LOW);
    pinsMode(PINS_HIGH, sizeof(PINS_HIGH), OUTPUT, HIGH);
    pinsMode(PINS_INPUT, sizeof(PINS_INPUT), INPUT);

    assert_reset();
    // At least one #RESET cycle.
    for (auto i = 0; i < 3; i++)
        cycle();
    cycle();
    negate_reset();
}

mc6809::Signals *PinsMc6809E::rawCycle() const {
    static uint8_t vma = LOW;
    // c1
    busMode(D, INPUT);
    auto signals = Signals::put();
    // c2
    c2_clock();
    delayNanoseconds(c2_ns);
    signals->getDirection();
    signals->getHighAddr();
    // c3
    c3_clock();
    signals->getLowAddr();
    if (vma == LOW) {
        delayNanoseconds(c3_novma);
        // c4
        c4_clock();
        Signals::nextCycle();
        delayNanoseconds(c4_novma);
    } else if (signals->write()) {
        delayNanoseconds(c3_write);
        signals->getData();
        // c4
        c4_clock();
        if (signals->writeMemory()) {
            _mems->write(signals->addr, signals->data);
            delayNanoseconds(c4_write);
        } else {
            delayNanoseconds(c4_capture);
        }
        Signals::nextCycle();
    } else {
        if (signals->readMemory()) {
            signals->data = _mems->read(signals->addr);
            delayNanoseconds(c3_read);
        } else {
            delayNanoseconds(c3_inject);
        }
        // c4
        c4_clock();
        busWrite(D, signals->data);
        busMode(D, OUTPUT);
        Signals::nextCycle();
        delayNanoseconds(c4_read);
    }
    // c1
    signals->getControl();
    c1_clock();
    vma = signals->avma();

    return signals;
}

mc6809::Signals *PinsMc6809E::cycle() const {
    delayNanoseconds(c1_ns);
    return rawCycle();
}

const mc6809::Signals *PinsMc6809E::findFetch(
        mc6809::Signals *begin, const mc6809::Signals *end) {
    const auto cycles = begin->diff(end);
    const auto native6309 = _regs->contextLength() == 14;
    for (auto i = 0; i < cycles; ++i) {
        auto s = begin->next(i);
        if (native6309) {
            if (s->lic()) {
                s->markFetch(1);
            } else {
                s->clearFetch();
            }
        } else {
            if (i && s->prev()->lic()) {
                s->markFetch(1);
            } else {
                s->clearFetch();
            }
        }
    }
    return begin;
}

}  // namespace mc6809e
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
