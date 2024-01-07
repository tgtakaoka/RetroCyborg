#include "pins_z88.h"
#include "debugger.h"
#include "devs_z88.h"
#include "digital_bus.h"
#include "inst_z88.h"
#include "mems_z88.h"
#include "regs_z88.h"
#include "signals_z88.h"
#include "z88_uart_handler.h"

namespace debugger {
namespace z88 {

struct PinsZ88 Pins;

// clang-format off
/**
 * Z8 External Bus cycle
 *         -|-----T1----|-----T2----|-----T3----|-----T1----|-----T2----|-----T3----|-----
 *         _    __    __    __    __    __    __    __    __    __    __    __    __
 * XTAL1/2  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|
 *           \     \     \           \     \     \     \     \     \     \     \     \
 *         ___|     |_____|___________|_____|_____|    |_____|_____|_____|_____|_____|
 *   #AS      \_____/                  \_____/ \___
 *         _______________|                 |_______________________| |_________
 *   #DS                  \_________________/     \___________/
 *         ___|___________________________________| |___
 *  R/#W   ___/                                         \___________________________________/___
 *         ___|___________________________________|___________________________________|___
 *    P0   ___X______________A15-18_______________X_______________A15-A8______________X___
 *            |___________|           |_____|     |___________|_______________________|___
 *    P1   ---<__A7-A0____>-----------<D7-D0>-----<__A7-A0____X__________D7-D0________X___
 *         ___|___________________________________|___________________________________|___
 *   #DM   ___X___________________________________X___________________________________X___
 */
// clang-format on

namespace {

constexpr auto xtal1_hi_ns = 0;
constexpr auto xtal1_lo_ns = 1;

inline void xtal1_hi() __attribute__((always_inline));
inline void xtal1_hi() {
    // XTAL1 is negated
    digitalWriteFast(PIN_XTAL1, LOW);
}

inline void xtal1_lo() __attribute__((always_inline));
inline void xtal1_lo() {
    // XTAL1 is negated
    digitalWriteFast(PIN_XTAL1, HIGH);
}

inline uint8_t signal_as() __attribute__((always_inline));
inline uint8_t signal_as() {
    return digitalReadFast(PIN_AS);
}

inline uint8_t signal_ds() __attribute__((always_inline));
inline uint8_t signal_ds() {
    return digitalReadFast(PIN_DS);
}

void assert_irq0() {
    digitalWriteFast(PIN_IRQ0, LOW);
}

void negate_irq0() {
    digitalWriteFast(PIN_IRQ0, HIGH);
}

void assert_irq1() {
    digitalWriteFast(PIN_IRQ1, LOW);
}

void negate_irq1() {
    digitalWriteFast(PIN_IRQ1, HIGH);
}

void assert_irq2() {
    digitalWriteFast(PIN_IRQ2, LOW);
}

void negate_irq2() {
    digitalWriteFast(PIN_IRQ2, HIGH);
}

void assert_reset() {
    digitalWriteFast(PIN_RESET, LOW);
    negate_irq0();
    negate_irq1();
    negate_irq2();
}

void negate_reset() {
    digitalWriteFast(PIN_RESET, HIGH);
}

const uint8_t PINS_LOW[] = {
        PIN_XTAL1,
        PIN_RESET,
};

const uint8_t PINS_HIGH[] = {
        PIN_IRQ0,
        PIN_IRQ1,
        PIN_IRQ2,
};

const uint8_t PINS_PULLDOWN[] = {
        PIN_ADDR0,
        PIN_ADDR1,
        PIN_ADDR2,
        PIN_ADDR3,
        PIN_ADDR4,
        PIN_ADDR5,
        PIN_ADDR6,
        PIN_ADDR7,
        PIN_ADDR8,
        PIN_ADDR9,
        PIN_ADDR10,
        PIN_ADDR11,
        PIN_ADDR12,
        PIN_ADDR13,
        PIN_ADDR14,
        PIN_ADDR15,
};

const uint8_t PINS_INPUT[] = {
        PIN_AS,
        PIN_DS,
        PIN_RW,
        PIN_DM,
};

inline void xtal1_cycle() {
    xtal1_lo();
    delayNanoseconds(xtal1_lo_ns);
    UartH.loop();
    xtal1_hi();
    delayNanoseconds(xtal1_hi_ns);
}

}  // namespace

void PinsZ88::reset() {
    pinsMode(PINS_LOW, sizeof(PINS_LOW), OUTPUT, LOW);
    pinsMode(PINS_HIGH, sizeof(PINS_HIGH), OUTPUT, HIGH);
    pinsMode(PINS_PULLDOWN, sizeof(PINS_PULLDOWN), INPUT_PULLDOWN);
    pinsMode(PINS_INPUT, sizeof(PINS_INPUT), INPUT);

    assert_reset();
    // #RESET must remain low for a minimum of 18 clocks.
    for (auto i = 0; i < 18 * 2; i++) {
        xtal1_cycle();
    }
    negate_reset();
    // Wait for #AS pulse with #DS high
    while (signal_ds() == LOW) {
        while (signal_as() == LOW)
            xtal1_cycle();
        while (signal_as() != LOW)
            xtal1_cycle();
    }

    Regs.reset();
    Regs.save();
}

Signals *PinsZ88::prepareCycle() {
    auto *signals = Signals::put();
    // Wait for bus cycle
    while (signal_ds() != LOW) {
        if (signal_as() == LOW) {
            // #AS is asserted
            while (signal_as() == LOW) {
                xtal1_cycle();
            }
            // #AS is high
            // Read address and bus status
            signals->getAddr();
        } else {
            xtal1_cycle();
        }
    }
    // #DS is low
    return signals;
}

Signals *PinsZ88::completeCycle(Signals *signals) {
    // #DS is low
    if (signals->write()) {
        signals->getData();
        if (signals->writeMemory()) {
            Memory.write(signals->addr, signals->data);
        } else {
            ;  // capture data to signals->data
        }
    } else {
        if (signals->readMemory()) {
            signals->data = Memory.read(signals->addr);
        } else {
            ;  // inject data from signals->data
        }
        signals->outData();
    }
    while (signal_ds() == LOW) {
        xtal1_cycle();
    }
    // #DS is high
    Signals::nextCycle();
    busMode(DATA, INPUT_PULLDOWN);
    return signals;
}

Signals *PinsZ88::cycle() {
    return completeCycle(prepareCycle());
}

Signals *PinsZ88::cycle(uint8_t inst) {
    return completeCycle(prepareCycle()->inject(inst));
}

void PinsZ88::execInst(const uint8_t *inst, uint8_t len) {
    execute(inst, len, nullptr, nullptr, 0);
}

uint8_t PinsZ88::captureWrites(const uint8_t *inst, uint8_t len, uint16_t *addr,
        uint8_t *buf, uint8_t max) {
    return execute(inst, len, addr, buf, max);
}

uint8_t PinsZ88::execute(const uint8_t *inst, uint8_t len, uint16_t *addr,
        uint8_t *buf, uint8_t max) {
    uint8_t inj = 0;
    uint8_t cap = 0;
    while (inj < len || cap < max) {
        auto *signals = prepareCycle();
        if (inj == 0)
            signals->fetch() = true;
        if (cap < max)
            signals->capture();
        if (inj < len)
            signals->inject(inst[inj]);
        completeCycle(signals);
        if (signals->read()) {
            if (inj < len)
                inj++;
        } else {
            if (cap == 0 && addr)
                *addr = signals->addr;
            if (buf && cap < max)
                buf[cap++] = signals->data;
        }
    }
    return cap;
}

void PinsZ88::idle() {
    const auto *signals = cycle(InstZ88::JR);
    cycle(InstZ88::JR_HERE);
    Signals::discard(signals);
}

void PinsZ88::loop() {
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

void PinsZ88::run() {
    Regs.restore();
    Signals::resetCycles();
    saveBreakInsts();
    loop();
}

bool PinsZ88::rawStep() {
    auto signals = prepareCycle();
reentry:
    const auto inst = Memory.read(signals->addr);
    const auto len = InstZ88::instLen(inst);
    if (len == 0 && signals->addr >= 0x20) {
        idle();
        return false;
    }
    auto fetch = signals;
    for (auto i = 0; i < len;) {
        if (signals->addr < 0x20) {
            // interrupt acknowledge is ongoing, fetching vector first.
            completeCycle(signals);
            cycle();
            // saving PC and FLAGS, though no writes if fast interrupt.
            signals = prepareCycle();
            while (signals->write()) {
                completeCycle(signals);
                signals = prepareCycle();
            }
            goto reentry;
        }
        completeCycle(signals);
        ++i;
        if (i < len)
            signals = prepareCycle();
    }
    fetch->fetch() = true;
    const auto cycles = InstZ88::busCycles(inst);
    for (auto c = 0; c < cycles; c++) {
        cycle();
    }
    return true;
}

bool PinsZ88::step(bool show) {
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

void PinsZ88::assertInt(uint8_t name) {
    switch (name) {
    default:
        break;
    case INTR_IRQ0:
        assert_irq0();
        break;
    case INTR_IRQ1:
        assert_irq1();
        break;
    case INTR_IRQ2:
        assert_irq2();
        break;
    }
}

void PinsZ88::negateInt(uint8_t name) {
    switch (name) {
    default:
        break;
    case INTR_IRQ0:
        negate_irq0();
        break;
    case INTR_IRQ1:
        negate_irq1();
        break;
    case INTR_IRQ2:
        negate_irq2();
        break;
    }
}

void PinsZ88::setBreakInst(uint32_t addr) const {
    Memory.put_inst(addr, InstZ88::UNKNOWN);
}

void PinsZ88::printCycles() const {
    const auto g = Signals::get();
    const auto cycles = g->diff(Signals::put());
    for (auto i = 0; i < cycles; i++) {
        g->next(i)->print();
    }
}

void PinsZ88::disassembleCycles() const {
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

}  // namespace z88
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
