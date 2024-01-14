#include "pins_mos6502.h"
#include "debugger.h"
#include "devs_mos6502.h"
#include "digital_bus.h"
#include "inst_mos6502.h"
#include "mems_mos6502.h"
#include "mems_w65c816.h"
#include "regs_mos6502.h"
#include "signals_mos6502.h"
#include "target_mos6502.h"

namespace debugger {
namespace mos6502 {

struct PinsMos6502 Pins;

/**
 * W65C02S bus cycle.
 *         __________            __________            ______
 * PHI2  _|          |__________|          |__________|
 *       __            __________            __________
 * PHI1O   |__________|          |__________|          |_____
 *           __________            __________            ____
 * PHI2O ___|          |__________|          |__________|
 *       ________  ____________________  ____________________
 *  SYNC ________><____________________><____________________
 *                ______________________
 *   R/W --------<                      |____________________
 *                           ____               _________
 *  Data -------------------<rrrr>-------------<wwwwwwwww>---
 *
 * - edges from PHI2 to PHI1O are about 8ns.
 * - edges from PHI1O to PHI2O are about 8ns.
 */

namespace {

constexpr auto phi0_raw_lo_ns = 30;
constexpr auto phi0_lo_ns = 195;
constexpr auto phi0_read_hi_ns = 370;
constexpr auto phi0_write_hi_ns = 390;

inline void phi0_hi() __attribute__((always_inline));
inline void phi0_hi() {
    digitalWriteFast(PIN_PHI0, HIGH);
}

inline void phi0_lo() __attribute__((always_inline));
inline void phi0_lo() {
    digitalWriteFast(PIN_PHI0, LOW);
}

void assert_abort() __attribute__((unused));
void assert_abort() {
    digitalWriteFast(W65C816_ABORT, LOW);
}

void negate_abort() {
    digitalWriteFast(W65C816_ABORT, HIGH);
}

void assert_nmi() __attribute__((unused));
void assert_nmi() {
    digitalWriteFast(PIN_NMI, LOW);
}

void negate_nmi() {
    digitalWriteFast(PIN_NMI, HIGH);
}

void assert_irq() {
    digitalWriteFast(PIN_IRQ, LOW);
}

void negate_irq() {
    digitalWriteFast(PIN_IRQ, HIGH);
}

void assert_be() __attribute__((unused));
void assert_be() {
    digitalWriteFast(PIN_BE, HIGH);
}

void negate_be() __attribute__((unused));
void negate_be() {
    digitalWriteFast(PIN_BE, LOW);
}

void assert_rdy() {
    digitalWriteFast(PIN_RDY, HIGH);
    pinMode(PIN_RDY, INPUT_PULLUP);
}

void negate_rdy() {
    digitalWriteFast(PIN_RDY, LOW);
    pinMode(PIN_RDY, OUTPUT_OPENDRAIN);
}

void assert_reset() {
    // Drive RESET condition
    phi0_lo();
    negate_be();
    digitalWriteFast(PIN_RES, LOW);
    negate_abort();
    negate_nmi();
    negate_irq();
    assert_rdy();
}

void negate_reset() {
    // Release RESET conditions
    digitalWriteFast(PIN_RES, HIGH);
}

const uint8_t PINS_LOW[] = {
        PIN_PHI0,
        PIN_RES,
        // NC on 6502
        // BE(input) for W65Cxx
        PIN_BE,
};

const uint8_t PINS_HIGH[] = {
        W65C816_ABORT,
        PIN_NMI,
        PIN_IRQ,
};

const uint8_t PINS_PULLUP[] = {
        // RDY(input) for 6502
        // RDY(bi-directional) for W65Cxx, pullup
        PIN_RDY,
        // PHI1O(output) from 6502
        // #ABORT(input) for 65816, pullup
        PIN_PHI1O,
        // NC on 6502
        // E(output) from 65816
        PIN_E,
        // NC for 6502
        // #ML(output) from W65Cxx
        PIN_ML,
};

const uint8_t PINS_INPUT[] = {
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
        // SYNC(output) from 6502
        // VPA(output) from 65816
        PIN_SYNC,
        // #SO(input) for 6502, pullup
        // #MX(output) from 65816
        PIN_SO,
        // PHI2O(output) from 6502
        // VDA(output) from 65816
        PIN_PHI2O,
        // VSS for 6502
        // #VP(output) from W65Cxx
        PIN_VP,
};

}  // namespace

bool PinsMos6502::native65816() const {
    return _type == HW_W65C816 && digitalReadFast(PIN_E) == LOW;
}

void PinsMos6502::checkHardwareType() {
    digitalWriteFast(PIN_PHI0, LOW);
    delayNanoseconds(500);
    digitalWriteFast(PIN_PHI0, HIGH);
    delayNanoseconds(500);

    // See Pins::begin() for details.
    if (digitalReadFast(PIN_PHI1O) == LOW) {
        // PIN_PHI1O is inverting of PHI0
        if (digitalReadFast(PIN_VP) == LOW) {
            // PIN_VP keeps LOW, means Vss of MOS6502, G65SC02, R65C02.
            _type = HW_MOS6502;
        } else {
            // PIN_VP goes HIGH, means #VP of W65C02S.
            _type = HW_W65C02S;
        }
        digitalWriteFast(PIN_SO, HIGH);
        pinMode(PIN_SO, OUTPUT);
        Target6502.setMems(mos6502::Memory);
    } else {
        // PIN_PHI1O keeps HIGH, means pulled-up #ABORT of W65C816.
        _type = HW_W65C816;
        digitalWriteFast(W65C816_ABORT, HIGH);
        pinMode(W65C816_ABORT, OUTPUT);
        Target6502.setMems(w65c816::Memory);
    }

    digitalWriteFast(PIN_PHI0, LOW);
}

void PinsMos6502::reset() {
    pinsMode(PINS_LOW, sizeof(PINS_LOW), OUTPUT, LOW);
    pinsMode(PINS_HIGH, sizeof(PINS_HIGH), OUTPUT, HIGH);
    pinsMode(PINS_PULLUP, sizeof(PINS_PULLUP), INPUT_PULLUP);
    pinsMode(PINS_INPUT, sizeof(PINS_INPUT), INPUT);

    checkHardwareType();

    assert_reset();
    // #RES must be held low for at lease two clock cycles.
    for (auto i = 0; i < 10; i++) {
        Signals::put()->capture();  // there may be suprious write
        cycle();
    }
    Signals::resetCycles();
    cycle();
    assert_be();
    negate_reset();
    // When a positive edge is detected, there is an initalization
    // sequence lasting seven clock cycles.
    for (auto i = 0; i < 7; i++) {
        // there may be suprious write
        auto *signals = prepareCycle()->capture();
        if (signals->vector() && signals->addr == InstMos6502::VECTOR_RESET)
            break;
        completeCycle(signals);
    }
    // Read Reset vector
    cycle();
    cycle();
    // printCycles() calls idle() and inject clocks.
    negate_rdy();
    // The first instruction will be saving registers, and certainly can be
    // injected.
    Registers.reset();
    Registers.save();
}

Signals *PinsMos6502::rawPrepareCycle() {
    auto *signals = Signals::put();
    delayNanoseconds(phi0_raw_lo_ns);
    signals->getAddr();
    return signals;
}

Signals *PinsMos6502::prepareCycle() {
    delayNanoseconds(phi0_lo_ns);
    return rawPrepareCycle();
}

Signals *PinsMos6502::completeCycle(Signals *signals) {
    // PHI2=HIGH
    phi0_hi();

    if (signals->write()) {
        delayNanoseconds(phi0_write_hi_ns);
        signals->getData();
        if (signals->writeMemory()) {
            Target6502.mems().write(signals->addr, signals->data);
        } else {
            ;  // capture data to signals->data
        }
    } else {
        if (signals->readMemory()) {
            signals->data = Target6502.mems().read(signals->addr);
        } else {
            ;  // inject data from signals->data
        }
        busWrite(D, signals->data);
        // change data bus to output
        busMode(D, OUTPUT);
        delayNanoseconds(phi0_read_hi_ns);
    }
    Signals::nextCycle();
    phi0_lo();
    // Set clock low to handle hold times and tristate data bus.
    busMode(D, INPUT);

    return signals;
}

Signals *PinsMos6502::cycle() {
    return completeCycle(prepareCycle());
}

void PinsMos6502::execInst(const uint8_t *inst, uint8_t len) {
    execute(inst, len, nullptr, nullptr, 0);
}

uint8_t PinsMos6502::captureWrites(const uint8_t *inst, uint8_t len,
        uint16_t *addr, uint8_t *buf, uint8_t max) {
    return execute(inst, len, addr, buf, max);
}

uint8_t PinsMos6502::execute(const uint8_t *inst, uint8_t len, uint16_t *addr,
        uint8_t *buf, uint8_t max) {
    unhalt();
    uint8_t inj = 0;
    uint8_t cap = 0;
    while (inj < len || cap < max) {
        auto *signals = prepareCycle();
        if (inj < len)
            signals->inject(inst[inj]);
        if (cap < max)
            signals->capture();
        completeCycle(signals);
        if (signals->read()) {
            if (inj < len)
                ++inj;
        } else {
            if (cap == 0 && addr)
                *addr = signals->addr;
            if (cap < max && buf)
                buf[cap++] = signals->data;
        }
    }
    negate_rdy();
    return cap;
}

void PinsMos6502::idle() {
    // CPU is stopping with RDY=L
    delayNanoseconds(398);
    phi0_hi();
    delayNanoseconds(472);
    phi0_lo();
}

void PinsMos6502::loop() {
    while (true) {
        Devices.loop();
        if (!rawStep(false) || haltSwitch()) {
            restoreBreakInsts();
            negate_rdy();
            disassembleCycles();
            Registers.save();
            return;
        }
    }
}

void PinsMos6502::run() {
    Registers.restore();
    Signals::resetCycles();
    saveBreakInsts();
    unhalt();
    loop();
}

Signals *PinsMos6502::unhalt() {
    assert_rdy();
    while (true) {
        auto *signals = prepareCycle();
        if (signals->fetch())
            return signals;
        completeCycle(signals);
    }
}

bool PinsMos6502::rawStep(bool step) {
    auto *signals = step ? unhalt() : Signals::put();
    const auto inst = Target6502.mems().raw_read(signals->addr);
    if (InstMos6502::isStop(inst))
        return false;
    if (inst == InstMos6502::BRK) {
        const auto opr = Target6502.mems().raw_read(signals->addr + 1);
        if (opr == 0 || isBreakPoint(signals->addr))
            return false;
    }
    completeCycle(unhalt());
    while (true) {
        auto s = prepareCycle();
        if (s->fetch())
            break;
        completeCycle(s);
    }
    if (step)
        negate_rdy();
    return true;
}

bool PinsMos6502::step(bool show) {
    Registers.restore();
    Signals::resetCycles();
    if (rawStep()) {
        if (show)
            printCycles();
        Registers.save();
        return true;
    }
    return false;
}

void PinsMos6502::assertInt(uint8_t name) {
    assert_irq();
}

void PinsMos6502::negateInt(uint8_t name) {
    negate_irq();
}

void PinsMos6502::setBreakInst(uint32_t addr) const {
    Target6502.mems().put_inst(addr, InstMos6502::BRK);
}

void PinsMos6502::printCycles() {
    const auto g = Signals::get();
    const auto cycles = g->diff(Signals::put());
    for (auto i = 0; i < cycles; i++) {
        g->next(i)->print();
        idle();
    }
}

void PinsMos6502::disassembleCycles() {
    const auto g = Signals::get();
    const auto cycles = g->diff(Signals::put());
    for (auto i = 0; i < cycles;) {
        const auto s = g->next(i);
        if (s->fetch()) {
            const auto len =
                    Target6502.mems().disassemble(s->addr, 1) - s->addr;
            i += len;
        } else {
            s->print();
            ++i;
        }
        idle();
    }
}

}  // namespace mos6502
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
