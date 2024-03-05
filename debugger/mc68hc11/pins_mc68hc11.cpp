#include "pins_mc68hc11.h"
#include "debugger.h"
#include "devs_mc68hc11.h"
#include "digital_bus.h"
#include "mc6800/inst_mc6800.h"
#include "mc68hc11_sci_handler.h"
#include "mems_mc68hc11.h"
#include "regs_mc68hc11.h"
#include "signals_mc68hc11.h"

namespace debugger {
namespace mc68hc11 {

/**
 * MC68HC11 bus cycle.
 *       _    __    __    __    __    __    __    __    __    __
 * EXTAL  |_c|1 |_c|2 |_c|3 |_c|4 |_c|1 |_c|2 |_c|3 |_c|4 |_c|1 |_
 *            \     \  \___________\  \     \  \___________\  \
 *     E _____|______|_|           |__|_____|__|           |__|___
 *            |______|                |_____|                 |___
 *    AS _____|      |________________|     |_________________|
 *           _______________________                         _____
 *   R/W ---<                       |_______________________|
 *           _______________________ _______________________ _____
 *  Addr ---<_______________________X_______________________X_____
 *              _______       ________   _______   __________    _
 *  Data ------<___A___>-----<______R_>-<___A___>-<_______W__>--<_
 *
 * - EXTAL falling-edge to E edges takes 40ns.
 * - EXTAL rising-edge of c1 to AS edges takes 40ns.
 * - EXTAL falling-edge of c4 to R/W edges takes 100ns.
 * - EXTAL falling-edge to #LIR edges takes 40ns.
 * - R/W and non-muxed address are valid after 281.5ns of falling edge of E.
 * - R/W and non-muxed address are valid before rising edge of c1.
 * - Muxed address is valid before 151ns of falling edge of AS.
 * - Muxed address is valid until 95.4ns of falling edge of AS.
 * - Read data setup to falling E egde is 30ns.
 * - Read data hold to falling E egde is 145.5ns.
 * - Write data gets valid after 190.5ns of rising E edge.
 */

namespace {

//  fXTAL: max   4.0 MHz
//   tCYC: min 1,000 ns
// pwRSTL: min 8,000 ns; reset_lo width
//   tMPH: min 2,000 ns: mode setup to reset_hi
//  pwIRQ: min 1,020 ns: irq_lo width
//  tPWEL: min   477 ns: e_lo
//  tPWEH: min   472 ns: e_hi
//    tAV: min   282 ns: high-addr to e_hi
//   tDSR: min   146 ns: data to e_lo
//   tDDW: max   191 ns: e_hi to data
//   tDHW: min    96 ns: e_lo to data hold
//   tAVM: min   272 ns: muxed-addr to e_hi
//   tASL: min   151 ns: muxed-addr to as_lo
//   tAHL: min    95 ns: as_lo to muxed-addr hold
//   tASD: min   116 ns: e_lo to as_hi
//  pwASH: min:  221 ns: as_hi
//  tASED: min   116 ns: as_lo to e_hi
//   tMAD: min   145 ns: el_lo to muxed-addr
//  tPCSU: min   300 ns; processor control setup to e_lo
constexpr auto extal_e_ns = 50;
constexpr auto extal_hi_ns = 100;   // 125
constexpr auto extal_lo_ns = 100;   // 125
constexpr auto c1_lo_ns = 80;       // 125
constexpr auto c1_hi_ns = 77;       // 125
constexpr auto c2_lo_ns = 62;       // 125
constexpr auto c2_hi_read = 54;     // 125
constexpr auto c3_lo_read = 1;      // 125
constexpr auto c3_lo_inject = 81;   // 125
constexpr auto c3_hi_read = 49;     // 125
constexpr auto c4_lo_read = 73;     // 125
constexpr auto c4_hi_read = 54;     // 125
constexpr auto c2_hi_write = 54;    // 125
constexpr auto c3_lo_write = 78;    // 125
constexpr auto c3_hi_write = 78;    // 125
constexpr auto c4_lo_write = 0;     // 125
constexpr auto c4_lo_capture = 89;  // 125
constexpr auto c4_hi_write = 53;    // 125
constexpr auto tpcsu = 300;

inline void extal_hi() __attribute__((always_inline));
inline void extal_hi() {
    digitalWriteFast(PIN_EXTAL, HIGH);
}

inline void extal_lo() __attribute__((always_inline));
inline void extal_lo() {
    digitalWriteFast(PIN_EXTAL, LOW);
}

inline void extal_cycle() __attribute__((always_inline));
inline void extal_cycle() {
    delayNanoseconds(extal_lo_ns);
    extal_hi();
    delayNanoseconds(extal_hi_ns);
    extal_lo();
}

inline uint8_t clock_e() __attribute__((always_inline));
inline uint8_t clock_e() {
    return digitalReadFast(PIN_E);
}

void assert_xirq() {
    digitalWriteFast(PIN_XIRQ, LOW);
    pinMode(PIN_XIRQ, OUTPUT_OPENDRAIN);
}

void negate_xirq() {
    digitalWriteFast(PIN_XIRQ, HIGH);
    pinMode(PIN_XIRQ, INPUT);
}

void assert_irq() {
    digitalWriteFast(PIN_IRQ, LOW);
    pinMode(PIN_IRQ, OUTPUT_OPENDRAIN);
}

void negate_irq() {
    digitalWriteFast(PIN_IRQ, HIGH);
    pinMode(PIN_IRQ, INPUT);
}

void assert_reset() {
    // Drive RESET with opend drain
    digitalWriteFast(PIN_RESET, LOW);
    pinMode(PIN_RESET, OUTPUT_OPENDRAIN);
    negate_xirq();
    negate_irq();
    extal_lo();
}

void negate_reset() {
    // Release RESET conditions
    digitalWriteFast(PIN_RESET, HIGH);
    pinMode(PIN_RESET, INPUT);
}

inline uint8_t reset_signal() __attribute__((always_inline));
inline uint8_t reset_signal() {
    return digitalReadFast(PIN_RESET);
}

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

inline void inject_mode_pin(uint8_t pin, uint8_t val) {
    digitalWriteFast(pin, val ? HIGH : LOW);
    pinMode(pin, OUTPUT_OPENDRAIN);
}

void inject_mode(uint8_t mode) {
    inject_mode_pin(PIN_MODB, mode & 2);
    inject_mode_pin(PIN_MODA, mode & 1);
}

void release_mode() {
    // PIN_PC0 is now PIN_LIR
    pinMode(PIN_MODB, INPUT);
    pinMode(PIN_MODA, INPUT);
}

}  // namespace

void PinsMc68hc11::reset() {
    pinsMode(PINS_INPUT, sizeof(PINS_INPUT), INPUT);
    pinMode(PIN_IRQ, OUTPUT_OPENDRAIN);
    pinMode(PIN_XIRQ, OUTPUT_OPENDRAIN);
    pinMode(PIN_EXTAL, OUTPUT);
    pinMode(PIN_MODA, INPUT);
    pinMode(PIN_MODB, INPUT);
    pinMode(PIN_RESET, OUTPUT_OPENDRAIN);
    extal_lo();

    // Reset vector should not point internal registers.
    const auto reset_vec = _mems->raw_read16(InstMc6800::VEC_RESET);
    _mems->raw_write16(InstMc6800::VEC_RESET, 0x8000);

    // To get out from Clock Monitor Reset, inject EXTAL pulses
    while (reset_signal() == LOW) {
        negate_reset();
        extal_cycle();
    }
    assert_reset();

    // Synchronize clock output and E clock input.
    while (clock_e() == LOW) {
        extal_cycle();
        delayNanoseconds(extal_e_ns);
    }
    while (clock_e() != LOW) {
        extal_cycle();
        delayNanoseconds(extal_e_ns);
    }
    // #RESET should be at least 8 cycles
    for (auto i = 0; i < 8; i++)
        cycle();

    Signals::resetCycles();
    // Mode Programming Setup Time for #RESET rising is 2 E cycles at minimum.
    inject_mode(MPU_MODE);
    cycle();
    cycle();
    cycle();
    delayNanoseconds(tpcsu);
    negate_reset();
    // Mode Programming Hold Time: min 10ns
    release_mode();
    cycle();
    // Read Reset vector
    cycle();
    cycle();
    // The first instruction will be saving registers, and certainly can be
    // injected.
    _regs->reset();
    _regs->save();
    _regs->checkSoftwareType();
    _mems->raw_write16(InstMc6800::VEC_RESET, reset_vec);
    _regs->setIp(reset_vec);
}

mc6800::Signals *PinsMc68hc11::cycle() {
    delayNanoseconds(c1_lo_ns);
    return rawCycle();
}

mc6800::Signals *PinsMc68hc11::rawCycle() {
    // MC68HC11 clock E is CLK/4, so we toggle CLK 4 times
    // C1H
    extal_hi();
    busMode(AD, INPUT);
    delayNanoseconds(c1_hi_ns);
    // C2L
    extal_lo();
    auto signals = Signals::put();
    delayNanoseconds(c2_lo_ns);
    signals->getMuxedAddr();
    // C2H
    extal_hi();
    signals->getHighAddr();
    if (signals->read()) {
        _writes = 0;
        delayNanoseconds(c2_hi_read);
        // C3L
        extal_lo();
        if (signals->readMemory()) {
            signals->data = _mems->read(signals->addr);
            if (c3_lo_read)
                delayNanoseconds(c3_lo_read);
        } else {
            delayNanoseconds(c3_lo_inject);
        }
        // C3H
        extal_hi();
        busWrite(AD, signals->data);
        busMode(AD, OUTPUT);
        delayNanoseconds(c3_hi_read);
        // C4L
        extal_lo();
        delayNanoseconds(c4_lo_read);
        signals->getLoadInstruction();
        // C4H
        extal_hi();
        // change data bus to output
        Signals::nextCycle();
        delayNanoseconds(c4_hi_read);
    } else {
        ++_writes;
        delayNanoseconds(c2_hi_write);
        // C3L
        extal_lo();
        delayNanoseconds(c3_lo_write);
        signals->getLoadInstruction();
        // C3H
        extal_hi();
        delayNanoseconds(c3_hi_write);
        signals->getData();
        // C4L
        extal_lo();
        if (signals->writeMemory()) {
            _mems->write(signals->addr, signals->data);
            if (c4_lo_write)
                delayNanoseconds(c4_lo_write);
        } else {
            delayNanoseconds(c4_lo_capture);
        }
        // C4H
        extal_hi();
        Signals::nextCycle();
        delayNanoseconds(c4_hi_write);
    }
    // C1L
    extal_lo();

    return signals;
}

void PinsMc68hc11::assertNmi() const {
    assert_xirq();
}

void PinsMc68hc11::negateNmi() const {
    negate_xirq();
}

void PinsMc68hc11::assertInt(uint8_t name) {
    assert_irq();
}

void PinsMc68hc11::negateInt(uint8_t name) {
    negate_irq();
}

namespace {
/**
 * MC68HC11 BRSET and BRCLR needs to special handling to print bus cycles.
 *   BRxxx a8,#n8,r8   ; 1:2:D:3:4:j
 *   BRxxx n8,X,#n8,r8 ; 1:2:x:R:3:4:j
 *   BRxxx n8,Y,#n8,r8 ; 1:2:3:x:R:4:5:j
 */
void printBrxxx(const Signals *s, const Mems *mems, uint8_t len) {
    auto opc = s->data;
    if (opc == 0x18)
        opc = mems->raw_read(s->addr + 1);
    const auto inst = opc & ~1;
    const auto brxxx = (inst == 0x12 || inst == 0x1E);
    if (brxxx)
        s->next(len - 1)->print();
}
}  // namespace

void PinsMc68hc11::disassembleCycles() {
    const auto g = Signals::get();
    const auto cycles = g->diff(Signals::put());
    for (auto i = 0; i < cycles;) {
        const auto s = g->next(i);
        if (s->fetch()) {
            const auto len = _mems->disassemble(s->addr, 1) - s->addr;
            printBrxxx(s, _mems, len);
            i += len;
        } else {
            s->print();
            ++i;
        }
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
