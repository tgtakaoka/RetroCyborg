#include "pins_mc6801.h"
#include "debugger.h"
#include "devs_mc6801.h"
#include "digital_bus.h"
#include "inst_hd6301.h"
#include "inst_mc6801.h"
#include "mems_mc6801.h"
#include "regs_mc6801.h"
#include "signals_mc6801.h"

namespace debugger {
namespace mc6801 {

struct PinsMc6801 Pins {
    &Regs, &Inst, &Memory, &Devices
};

/**
 * MC6801 bus cycle.
 *         __    __    __    __    __    __    __    __    __    __
 * EXTAL c|4 |_c|1 |_c|2 |_c|3 |_c|4 |_c|1 |_c|2 |_c|3 |_c|4 |_c| 1|_
 *       ____\  \     \  \ __________\  \     \  \ ___________\  \
 *     E      |___________|           |___________|           |______
 *                _____                   _____                    __
 *    AS ________|     |_________________|     |__________________|
 *       ______     ___________________                             _
 *   R/W ______>---|                   |---|___________________|---<_
 *       ______     ___________________     ___________________     _
 *  Addr ______>---<___________________>---<___________________>---<_
 *       ______     ____            ___     ____        _______     _
 *  Data ______>---<____>----------<___>---<____>------<_______>---<_
 *
 * - EXTAL falling-edge to E edges takes 50ns.
 * - EXTAL rising-edge of c1 to AS edges takes 40ns.
 * - EXTAL falling-edge of c4 to R/W edges takes 100ns.
 * - R/W is valid before rising edge of c1.
 * - Read data setup to falling E egde is 80ns.
 * - Write data gets valid after 225ns of rising E edge.
 */

namespace {

//  XTAL: max 4 MHz, min 0.4 MHz
//  tCYC: min 1000 ns, max 2,000 ns
//  PWEL: min  430 ns, max 1,000 ns; e_lo
//  PWEH: min  450 ns, max 1,000 ns; e_hi
//   tAV: min  200 ns; addr to e_hi
//  tDSR: min   80 ns; data to e_lo
//  tDDW: max  225 ns; e_hi to data
//  tASD: min   90 ns; e_lo to as_hi
//  tASL: min   60 ns; addr to as_lo
// tASED: min   90 ns; as_lo to e_hi
//  tPCS: min  200 ns; reset_hi to e_lo
constexpr auto extal_hi_ns = 108;   // 125
constexpr auto extal_lo_ns = 108;   // 125
constexpr auto c1_lo_ns = 100;      // 125
constexpr auto c1_hi_ns = 94;       // 125
constexpr auto c2_lo_ns = 77;       // 125
constexpr auto c2_hi_ns = 64;       // 125
constexpr auto c3_lo_read = 1;      // 125
constexpr auto c3_lo_inject = 73;   // 125
constexpr auto c3_hi_read = 73;     // 125
constexpr auto c4_lo_read = 80;     // 125
constexpr auto c4_hi_read = 49;     // 125
constexpr auto c3_lo_write = 85;    // 125
constexpr auto c3_hi_write = 80;    // 125
constexpr auto c4_lo_write = 1;     // 125
constexpr auto c4_lo_capture = 92;  // 125
constexpr auto c4_hi_write = 49;    // 125
constexpr auto tpcs_ns = 200;

inline void extal_hi() __attribute__((always_inline));
inline void extal_hi() {
    digitalWriteFast(PIN_EXTAL, HIGH);
}

inline void extal_lo() __attribute__((always_inline));
inline void extal_lo() {
    digitalWriteFast(PIN_EXTAL, LOW);
}

inline void clock_cycle() __attribute__((always_inline));
inline void clock_cycle() {
    extal_hi();
    delayNanoseconds(extal_hi_ns);
    extal_lo();
    delayNanoseconds(extal_lo_ns);
}

uint8_t clock_e() {
    return digitalReadFast(PIN_E);
}

void assert_nmi() {
    digitalWriteFast(PIN_NMI, LOW);
    // #NMI is connected to P21/PC1 for LILBUG trace.
    pinMode(PIN_NMI, OUTPUT_OPENDRAIN);
}

void negate_nmi() {
    digitalWriteFast(PIN_NMI, HIGH);
    // #NMI is connected to P21/PC1 for LILBUG trace.
    pinMode(PIN_NMI, INPUT_PULLUP);
}

void assert_irq1() {
    digitalWriteFast(PIN_IRQ1, LOW);
}

void negate_irq1() {
    digitalWriteFast(PIN_IRQ1, HIGH);
}

void assert_reset() {
    // Drive RESET condition
    digitalWriteFast(PIN_RESET, LOW);
    negate_nmi();
    negate_irq1();

    // Toggle reset to put MC6803/HD6303 in reset
    clock_cycle();
    clock_cycle();
    clock_cycle();
    clock_cycle();
}

void negate_reset() {
    // Release RESET conditions
    digitalWriteFast(PIN_RESET, HIGH);
}

inline void inject_mode_pin(uint8_t pin, uint8_t val) {
    if (val) {
        pinMode(pin, INPUT_PULLUP);
    } else {
        digitalWriteFast(pin, LOW);
        pinMode(pin, OUTPUT);
    }
}

void inject_mode(uint8_t mode) {
    inject_mode_pin(PIN_PC2, mode & 4);
    inject_mode_pin(PIN_PC1, mode & 2);
    inject_mode_pin(PIN_PC0, mode & 1);
}

void release_mode() {
    // #NMI is connected to P21/PC1 for LILBUG trace.
    pinMode(PIN_PC2, INPUT_PULLUP);
    pinMode(PIN_PC1, INPUT_PULLUP);
    pinMode(PIN_PC0, INPUT_PULLUP);
}

const uint8_t PINS_LOW[] = {
        PIN_EXTAL,
        PIN_RESET,
};

const uint8_t PINS_HIGH[] = {
        PIN_IRQ1,
};

const uint8_t PINS_OPENDRAIN[] = {
        // #NMI is connected to P21/PC1 for LILBUG trace.
        PIN_NMI,
};

const uint8_t PINS_PULLUP[] = {
        PIN_XTAL,
        PIN_PC0,
        PIN_PC1,
        PIN_PC2,
};

const uint8_t PINS_INPUT[] = {
        PIN_AD0,
        PIN_AD1,
        PIN_AD2,
        PIN_AD3,
        PIN_AD4,
        PIN_AD5,
        PIN_AD6,
        PIN_AD7,
        PIN_AH8,
        PIN_AH9,
        PIN_AH10,
        PIN_AH11,
        PIN_AH12,
        PIN_AH13,
        PIN_AH14,
        PIN_AH15,
        PIN_RW,
        PIN_AS,
        PIN_E,
};

}  // namespace

void PinsMc6801::reset() {
    pinsMode(PINS_LOW, sizeof(PINS_LOW), OUTPUT, LOW);
    pinsMode(PINS_HIGH, sizeof(PINS_HIGH), OUTPUT, HIGH);
    pinsMode(PINS_OPENDRAIN, sizeof(PINS_OPENDRAIN), OUTPUT_OPENDRAIN);
    pinsMode(PINS_PULLUP, sizeof(PINS_PULLUP), INPUT_PULLUP);
    pinsMode(PINS_INPUT, sizeof(PINS_INPUT), INPUT);

    // Reset vector should not point internal registers.
    const uint16_t reset_vec = _mems->raw_read16(InstMc6800::VEC_RESET);
    _mems->raw_write16(InstMc6800::VEC_RESET, 0x8000);

    assert_reset();
    // Synchronize clock output and E clock input.
    while (clock_e() == LOW)
        clock_cycle();
    while (clock_e() != LOW)
        clock_cycle();
    Signals::resetCycles();
    // #RESET Low Pulse Width: min 3 E cycles
    cycle();
    // Mode Programming Setup Time: min 2 E cycles
    inject_mode(MPU_MODE);
    cycle();
    cycle();
    delayNanoseconds(tpcs_ns);
    negate_reset();
    // Mode Programming Hold Time: min MC6803:100ns HD6303:150ns
    release_mode();
    if (isHd63())
        cycle();
    cycle();
    // Read Reset vector
    cycle();
    cycle();
    // The first instruction will be saving registers, and certainly can be
    // injected.
    _regs->reset();
    _regs->save();
    _mems->raw_write16(InstMc6800::VEC_RESET, reset_vec);
    _regs->setIp(reset_vec);
    if (_regs->checkSoftwareType() == SW_HD6301)
        _inst = &hd6301::Inst;
}

mc6800::Signals *PinsMc6801::cycle() {
    delayNanoseconds(c1_lo_ns);
    return rawCycle();
}

mc6800::Signals *PinsMc6801::rawCycle() {
    // MC6803/HD6303 clock E is CLK/4, so we toggle CLK 4 times
    busMode(AD, INPUT);
    // c1
    extal_hi();
    auto *signals = Signals::put();
    delayNanoseconds(c1_hi_ns);
    // c2
    extal_lo();
    delayNanoseconds(c2_lo_ns);
    signals->getMuxedAddr();
    extal_hi();
    delayNanoseconds(c2_hi_ns);
    signals->getDirection();
    // c3
    extal_lo();
    if (signals->write()) {
        ++_writes;
        delayNanoseconds(c3_lo_write);
        extal_hi();
        delayNanoseconds(c3_hi_write);
        signals->getData();
        // c4
        extal_lo();
        if (signals->writeMemory()) {
            _mems->write(signals->addr, signals->data);
            if (c4_lo_write)
                delayNanoseconds(c4_lo_write);
        } else {
            delayNanoseconds(c4_lo_capture);
        }
        extal_hi();
        Signals::nextCycle();
        delayNanoseconds(c4_hi_write);
    } else {
        if (signals->readMemory()) {
            signals->data = _mems->read(signals->addr);
            if (c3_lo_read)
                delayNanoseconds(c3_lo_read);
        } else {
            delayNanoseconds(c3_lo_inject);
        }
        extal_hi();
        busWrite(AD, signals->data);
        delayNanoseconds(c3_hi_read);
        // c4
        extal_lo();
        busMode(AD, OUTPUT);
        delayNanoseconds(c4_lo_read);
        extal_hi();
        _writes = 0;
        Signals::nextCycle();
        delayNanoseconds(c4_hi_read);
    }
    // c1
    extal_lo();

    return signals;
}

void PinsMc6801::idle() {
    auto s = Signals::put();
    s->inject(InstMc6800::BRA);
    rawCycle();
    injectCycle(InstMc6800::BRA_HERE);
    injectCycle(0);
    Signals::discard(s);
}

void PinsMc6801::assertNmi() const {
    assert_nmi();
}

void PinsMc6801::negateNmi() const {
    negate_nmi();
}

void PinsMc6801::assertInt(uint8_t name) {
    (void)name;
    assert_irq1();
}

void PinsMc6801::negateInt(uint8_t name) {
    (void)name;
    negate_irq1();
}

bool PinsMc6801::isHd63() const {
    return digitalReadFast(PIN_XTAL) != LOW;
}

}  // namespace mc6801
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
