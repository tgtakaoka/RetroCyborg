#include "pins_scn2650.h"
#include "debugger.h"
#include "devs_scn2650.h"
#include "digital_bus.h"
#include "inst_scn2650.h"
#include "mems_scn2650.h"
#include "regs_scn2650.h"

namespace debugger {
namespace scn2650 {

struct PinsScn2650 Pins;

// clang-format off
/*
 * SCN25650 Bus cycle
 *            _____       _____       _____       _____       _____       _____       __
 *  CLOCK ___|    T|0____|    T|1____|    T|2____|    T|0____|    T|1____|    T|2____|
 *                        \__________________\                 \_________________\
 *  OPREQ ________________|                  |_________________|                 |_______
 *        _______________ /__________________\__ _____________ /__________________\_ ____
 * A0~A14 _______________X______________________X_____________X_____________________X____
 *  M/#IO _______________                     |  _________________________________|_
 *   #R/W                \____________________|_/                                 | \____
 *        ______________________________ _____ ___________________________________ ______
 *   DATA ______________________________X__in_X________________________out________X______
 *        ________________________    ________________________________    _______________
 * #OPACK                         \__/                                \__/
 *        ____________________________________________________       _______       ______
 *   #WRP                                                     \_____/       \_____/
 */
// clang-format on

namespace {

constexpr auto clock_hi_ns = 380;
constexpr auto clock_lo_ns = 380;
constexpr auto clock_hi_t0p = 250;
constexpr auto clock_hi_t0c = 340;
constexpr auto clock_hi_tcor = 100;
constexpr auto clock_hi_t1 = 100;
constexpr auto clock_hi_t2w = 360;
constexpr auto clock_hi_idle = 120;
constexpr auto clock_lo_t0 = 390;
constexpr auto clock_lo_t1w = 350;
constexpr auto clock_lo_t1r = 320;

inline void clock_hi() __attribute__((always_inline));
inline void clock_hi() {
    digitalWriteFast(PIN_CLOCK, HIGH);
}

inline void clock_lo() __attribute__((always_inline));
inline void clock_lo() {
    digitalWriteFast(PIN_CLOCK, LOW);
}

inline uint8_t signal_clock() {
    return digitalReadFast(PIN_CLOCK);
}

inline uint8_t signal_opreq() {
    return digitalReadFast(PIN_OPREQ);
}

void assert_intreq() {
    digitalWriteFast(PIN_INTREQ, LOW);
}

void negate_intreq() {
    digitalWriteFast(PIN_INTREQ, HIGH);
}

void assert_reset() {
    digitalWriteFast(PIN_RESET, HIGH);
    negate_intreq();
}

void negate_reset() {
    digitalWriteFast(PIN_RESET, LOW);
}

void negate_pause() {
    digitalWriteFast(PIN_PAUSE, HIGH);
}

void assert_opack() {
    digitalWriteFast(PIN_OPACK, LOW);
}

void negate_opack() {
    digitalWriteFast(PIN_OPACK, HIGH);
}

const uint8_t PINS_LOW[] = {
        PIN_ADREN,
        PIN_DBUSEN,
        PIN_SENSE,
};

const uint8_t PINS_HIGH[] = {
        PIN_RESET,
        PIN_CLOCK,
        PIN_PAUSE,
        PIN_OPACK,
        PIN_INTREQ,
};

const uint8_t PINS_INPUT[] = {
        PIN_DBUS0,
        PIN_DBUS1,
        PIN_DBUS2,
        PIN_DBUS3,
        PIN_DBUS4,
        PIN_DBUS5,
        PIN_DBUS6,
        PIN_DBUS7,
        PIN_ADR0,
        PIN_ADR1,
        PIN_ADR2,
        PIN_ADR3,
        PIN_ADR4,
        PIN_ADR5,
        PIN_ADR6,
        PIN_ADR7,
        PIN_ADR8,
        PIN_ADR9,
        PIN_ADR10,
        PIN_ADR11,
        PIN_ADR12,
        PIN_ADR13,
        PIN_ADR14,
        PIN_OPREQ,
        PIN_MIO,
        PIN_RW,
        PIN_WRP,
        PIN_INTACK,
        PIN_RUNWAIT,
        PIN_FLAG,
};

inline void clock_cycle() {
    clock_lo();
    delayNanoseconds(clock_lo_ns);
    clock_hi();
    delayNanoseconds(clock_hi_ns);
}

}  // namespace

void PinsScn2650::reset() {
    pinsMode(PINS_LOW, sizeof(PINS_LOW), OUTPUT, LOW);
    pinsMode(PINS_HIGH, sizeof(PINS_HIGH), OUTPUT, HIGH);
    pinsMode(PINS_INPUT, sizeof(PINS_INPUT), INPUT);

    negate_pause();
    assert_reset();
    // RESET must remain high for at least 3 cycles
    for (auto i = 0; i < 5; ++i)
        clock_cycle();
    negate_reset();

    Regs.save();
}

Signals *PinsScn2650::prepareCycle() {
    auto *signals = Signals::put();
    // T0H
    while (signal_opreq() == LOW) {
        delayNanoseconds(clock_hi_t0p);
        clock_lo();  // T0L
        delayNanoseconds(clock_lo_t0);
        clock_hi();  // T1H
        delayNanoseconds(clock_hi_tcor);
    }
    // T1H, OPREQ=H
    signals->getAddr();
    return signals;
}

Signals *PinsScn2650::completeCycle(Signals *signals) {
    // T1H, OPREQ=H
    delayNanoseconds(clock_hi_t1);
    clock_lo();  // T1L
    if (signals->write()) {
        signals->getData();
        assert_opack();
        delayNanoseconds(clock_lo_t1w);
        clock_hi();  // T2H
        if (signals->io()) {
            if (signals->addr & 0x2000) {
                signals->addr &= 0xFF;
                if (Devs.isSelected(signals->addr)) {
                    Devs.write(signals->addr, signals->data);
                }
            }
        } else if (signals->writeMemory()) {
            Memory.write(signals->addr, signals->data);
        } else {
            ;  // capture data to signals->data
        }
        delayNanoseconds(clock_hi_t2w);
    } else {
        if (signals->io()) {
            if (signals->vector()) {
                signals->data = Devs.vector();
            } else if (signals->addr & 0x2000) {
                signals->addr &= 0xFF;
                if (Devs.isSelected(signals->addr)) {
                    signals->data = Devs.read(signals->addr);
                }
            }
        } else if (signals->readMemory()) {
            signals->data = Memory.read(signals->addr);
        } else {
            ;  // inject data from signals->data
        }
        signals->outData();
        assert_opack();
        delayNanoseconds(clock_lo_t1r);
        clock_hi();  // T2H
        delayNanoseconds(clock_hi_ns);
    }
    negate_opack();
    clock_lo();  // T2L
    Signals::nextCycle();
    busMode(DBUS, INPUT_PULLDOWN);
    delayNanoseconds(clock_lo_ns);
    clock_hi();  // T0H
    delayNanoseconds(clock_hi_t0c);
    return signals;
}

void PinsScn2650::execInst(const uint8_t *inst, uint8_t len) {
    execute(inst, len, nullptr, nullptr, 0);
}

uint8_t PinsScn2650::captureWrites(const uint8_t *inst, uint8_t len,
        uint16_t *addr, uint8_t *buf, uint8_t max) {
    return execute(inst, len, addr, buf, max);
}

uint8_t PinsScn2650::execute(const uint8_t *inst, uint8_t len, uint16_t *addr,
        uint8_t *buf, uint8_t max) {
    uint8_t inj = 0;
    uint8_t cap = 0;
    while (inj < len || cap < max) {
        auto *signals = prepareCycle();
        if (cap < max)
            signals->capture();
        if (inj < len)
            signals->inject(inst[inj]);
        completeCycle(signals);
        if (inj == 0 && addr)
            *addr = signals->addr;
        if (signals->read()) {
            if (inj < len)
                inj++;
        } else {
            if (buf && cap < max)
                buf[cap++] = signals->data;
        }
    }
    return cap;
}

void PinsScn2650::idle() {
    clock_lo();
    delayNanoseconds(clock_lo_ns);
    clock_hi();
    delayNanoseconds(clock_hi_idle);
}

void PinsScn2650::loop() {
    while (true) {
        Devs.loop();
        if (!rawStep() || haltSwitch()) {
            restoreBreakInsts();
            disassembleCycles();
            Regs.save();
            return;
        }
    }
}

void PinsScn2650::run() {
    Regs.restore();
    Signals::resetCycles();
    saveBreakInsts();
    loop();
}

bool PinsScn2650::rawStep() {
    delayNanoseconds(clock_hi_ns);
    auto *signals = prepareCycle();
    const auto inst = Memory.read(signals->addr);
    const auto len = InstScn2650::instLen(inst);
    if (inst == InstScn2650::HALT || len == 0) {
        // HALT or unknown instruction. Just return.
        return false;
    }

    completeCycle(signals);
    const auto opr = Memory.read(signals->addr + 1);
    const auto busCycles = len + InstScn2650::busCycles(inst, opr);
    signals->fetchMark() = true;
    for (auto i = 1; i < busCycles; ++i) {
        auto s = prepareCycle();
        if (s->vector()) {
            signals->fetchMark() = false;
            completeCycle(s);
            if (InstScn2650::busCycles(InstScn2650::ZBSR, s->data)) {
                // Indirect vector
                completeCycle(prepareCycle());
                completeCycle(prepareCycle());
            }
            break;
        }
        completeCycle(s);
    }
    prepareCycle();
    return true;
}

bool PinsScn2650::step(bool show) {
    Regs.restore();
    Signals::resetCycles();
    if (rawStep()) {
        if (show)
            printCycles();
        Regs.save();
        return true;
    }
    return false;
}

void PinsScn2650::assertInt(uint8_t name) {
    (void)name;
    assert_intreq();
}

void PinsScn2650::negateInt(uint8_t name) {
    (void)name;
    negate_intreq();
}

void PinsScn2650::setBreakInst(uint32_t addr) const {
    Memory.put_inst(addr, InstScn2650::HALT);
}

void PinsScn2650::printCycles() {
    const auto g = Signals::get();
    const auto cycles = g->diff(Signals::put());
    for (auto i = 0; i < cycles; i++) {
        g->next(i)->print();
    }
}

void PinsScn2650::disassembleCycles() {
    const auto g = Signals::get();
    const auto cycles = g->diff(Signals::put());
    for (auto i = 0; i < cycles;) {
        const auto s = g->next(i);
        if (s->fetch()) {
            const auto len = Memory.disassemble(s->addr, 1) - s->addr;
            i += len;
        } else {
            s->print();
            ++i;
        }
    }
}

}  // namespace scn2650
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
