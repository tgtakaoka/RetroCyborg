#include "pins_ins8070.h"
#include "debugger.h"
#include "devs_ins8070.h"
#include "digital_bus.h"
#include "ins8070_sci_handler.h"
#include "inst_ins8070.h"
#include "mems_ins8070.h"
#include "regs_ins8070.h"
#include "signals_ins8070.h"

namespace debugger {
namespace ins8070 {

struct PinsIns8070 Pins;

// clang-format off
/**
 * INS8070 External Bus cycle
 *       __    __    __    __    __    __    __    __    __    __
 *  XOUT   |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__
 *         __    __    __    __    __    __    __    __    __    __
 *   XIN _|  |__| 1|__| 2|__| 3|__| 4|__| 5|__| 6|__| 7|__| 8|__|  |
 *       ________\        \     \     \                 \     \ ____
 * #BREQ         |_____________________________________________|
 *                          _________________________________
 *  ADDR-------------------<_________________________________>------
 *       ________________________                         __________
 *  #RDS                         |______________________r|r
 *       _____________________________                    __________
 *  #WDS                              |wwwwwwwwwwwwwwwwww|
 *
 * - XIN edges to XOUT edges are 8ns.
 * - XIN 1st rising edge to #BREQ falling edge is 72ns.
 * - XIN 3rd falling edge to #RDS falling edge is 84ns.
 * - XIN 7th falling edge to #RDS rising edge is 88ns.
 * - XIN 8th falling edge to #BREQ rising edge is 176ns.
 * - #BREQ falling edge to #RDS/#WDS falling edge is 2 cycles/
 * - #RST rising edge to 1st #BREQ takes ~14 XIN cycles.
 */
// clang-format on

namespace {

constexpr auto xin_hi_ns = 110;
constexpr auto xin_lo_ns = 105;

inline void xin_hi() __attribute__((always_inline));
inline void xin_hi() {
    digitalWriteFast(PIN_XIN, HIGH);
}

inline void xin_lo() __attribute__((always_inline));
inline void xin_lo() {
    digitalWriteFast(PIN_XIN, LOW);
}

inline uint8_t signal_breq() __attribute__((always_inline));
inline uint8_t signal_breq() {
    return digitalReadFast(PIN_BREQ);
}

inline uint8_t signal_wds() __attribute__((always_inline));
inline uint8_t signal_wds() {
    return digitalReadFast(PIN_WDS);
}

inline uint8_t signal_rds() __attribute__((always_inline));
inline uint8_t signal_rds() {
    return digitalReadFast(PIN_RDS);
}

void assert_sa() __attribute__((unused));
void assert_sa() {
    digitalWriteFast(PIN_SA, LOW);
}

void negate_sa() {
    digitalWriteFast(PIN_SA, HIGH);
}

void assert_sb() __attribute__((unused));
void assert_sb() {
    digitalWriteFast(PIN_SB, LOW);
}

void negate_sb() {
    digitalWriteFast(PIN_SB, HIGH);
}

void assert_hold() __attribute__((unused));
void assert_hold() {
    digitalWriteFast(PIN_HOLD, LOW);
}

void negate_hold() __attribute__((unused));
void negate_hold() {
    digitalWriteFast(PIN_HOLD, HIGH);
}

void assert_enin() {
    digitalWriteFast(PIN_ENIN, LOW);
}

void negate_enin() {
    digitalWriteFast(PIN_ENIN, HIGH);
}

void assert_reset() {
    // Drive RESET condition
    digitalWriteFast(PIN_RST, LOW);
    negate_sa();
    negate_sb();
    negate_hold();
    negate_enin();
}

void negate_reset() {
    // Release RESET conditions
    digitalWriteFast(PIN_RST, HIGH);
}

const uint8_t PINS_LOW[] = {
        PIN_XIN,
        PIN_RST,
};

const uint8_t PINS_HIGH[] = {
        PIN_SA,
        PIN_SB,
        PIN_ENIN,
        PIN_HOLD,
};

const uint8_t PINS_PULLUP[] = {
        PIN_BREQ,
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
        PIN_RDS,
        PIN_WDS,
        PIN_ENOUT,
        PIN_F1,
        PIN_F2,
        PIN_F3,
};

inline void clock_cycle() {
    xin_hi();
    delayNanoseconds(xin_hi_ns);
    xin_lo();
    SciH.loop();
    delayNanoseconds(xin_lo_ns);
}

}  // namespace

void PinsIns8070::reset() {
    pinsMode(PINS_LOW, sizeof(PINS_LOW), OUTPUT, LOW);
    pinsMode(PINS_HIGH, sizeof(PINS_HIGH), OUTPUT, HIGH);
    pinsMode(PINS_PULLUP, sizeof(PINS_PULLUP), INPUT_PULLUP);
    pinsMode(PINS_INPUT, sizeof(PINS_INPUT), INPUT);

    assert_reset();
    // #RST must remain low is 8 Tc.
    for (auto i = 0; i < 4 * 8 * 10; i++)
        clock_cycle();
    negate_reset();
    negate_enin();
    // The first instruction will be fetched within 13 Tc after #RST
    // has gone high.
    Regs.save();
}

Signals *PinsIns8070::prepareCycle() {
    auto signals = Signals::put();
    while (signal_breq() != LOW) {
        clock_cycle();
    }
    assert_enin();
    // #BREQ=LOW
    while (!signals->getDirection()) {
        clock_cycle();
    }
    signals->getAddr();
    return signals;
}

Signals *PinsIns8070::completeCycle(Signals *signals) {
    if (signals->write()) {
        signals->getData();
        if (signals->writeMemory()) {
            Memory.write(signals->addr, signals->data);
        } else {
            ;  // capture data to signals->data
        }
        while (signal_wds() == LOW) {
            clock_cycle();
        }
    } else {
        if (signals->readMemory()) {
            signals->data = Memory.read(signals->addr);
        } else {
            ;  // inject data from signals->data
        }
        busWrite(D, signals->data);
        busMode(D, OUTPUT);
        while (signal_rds() == LOW) {
            clock_cycle();
        }
    }
    busMode(D, INPUT);
    Signals::nextCycle();
    for (auto i = 0; signal_breq() == LOW && i < 2; i++) {
        clock_cycle();
    }
    if (signal_breq() != LOW) {
        negate_enin();
    }
    return signals;
}

Signals *PinsIns8070::cycle() {
    return completeCycle(prepareCycle());
}

Signals *PinsIns8070::cycle(uint8_t data) {
    return completeCycle(prepareCycle()->inject(data));
}

void PinsIns8070::execInst(const uint8_t *inst, uint8_t len) {
    execute(inst, len, nullptr, nullptr, 0);
}

uint8_t PinsIns8070::captureWrites(const uint8_t *inst, uint8_t len,
        uint16_t *addr, uint8_t *buf, uint8_t max) {
    return execute(inst, len, addr, buf, max);
}

uint8_t PinsIns8070::execute(const uint8_t *inst, uint8_t len, uint16_t *addr,
        uint8_t *buf, uint8_t max) {
    uint8_t inj = 0;
    uint8_t cap = 0;
    while (inj < len || cap < max) {
        auto signals = prepareCycle();
        if (inj < len)
            signals->inject(inst[inj]);
        if (cap < max)
            signals->capture();
        completeCycle(signals);
        if (signals->read() && inj < len) {
            ++inj;
        } else if (signals->write()) {
            if (cap == 0 && addr)
                *addr = signals->addr;
            if (buf && cap < max)
                buf[cap++] = signals->data;
        }
    }
    return cap;
}

void PinsIns8070::idle() {
    // #ENIN is HIGH and bus cycle is suspened.
    clock_cycle();
}

void PinsIns8070::loop() {
    while (true) {
        Devs.loop();
        auto signals = prepareCycle();
        if (signals->addr == InstIns8070::VEC_CALL15 && signals->read()) {
            if (signals->prev(1)->write() && signals->prev(2)->write()) {
                const auto call = signals->prev(3);
                const auto isBP =
                        isBreakPoint(call->addr) ||
                        Memory.raw_read16(InstIns8070::VEC_CALL15) == 0;
                if (call->read() && call->data == InstIns8070::CALL15 && isBP) {
                    const uint16_t addr = call->addr - 1;
                    cycle(0);                 // inject low vector
                    cycle(0);                 // inject high vector
                    cycle(InstIns8070::RET);  // cancel CALL 15
                    cycle(lo(addr));          // inject low address
                    cycle(hi(addr));          // inject high address
                    halt(call);
                    return;
                }
            }
        }
        completeCycle(signals);
        if (haltSwitch()) {
            _stop = nullptr;
            halt(nullptr);
            return;
        }
    }
}

void PinsIns8070::halt(Signals *stop) {
    restoreBreakInsts();
    while (stop == nullptr) {
        auto signals = prepareCycle();
        if (signals->fetch()) {
            stop = signals;
            completeCycle(signals->inject(InstIns8070::BRA));
            cycle(InstIns8070::BRA_HERE);
        } else {
            completeCycle(signals);
        }
    }
    Signals::discard(stop);
    disassembleCycles();
    Regs.save();
}

void PinsIns8070::run() {
    Regs.restore();
    Signals::resetCycles();
    saveBreakInsts();
    loop();
}

bool PinsIns8070::step(bool show) {
    InstIns8070 inst;
    const auto cycles = Regs.busCycles(inst);
    if (cycles == 0)
        return false;
    if (inst.opc == InstIns8070::CALL15 &&
            Memory.raw_read16(InstIns8070::VEC_CALL15) == 0)
        return false;
    Regs.restore();
    Signals::resetCycles();
    if (inst.addrMode() == M_SSM) {
        // SSM instruction
        const auto addr = cycle()->addr;
        for (auto n = 0; n < 256; ++n) {
            auto signals = prepareCycle();
            if (signals->addr == addr + 3) {
                completeCycle(signals->inject(InstIns8070::BRA));
                cycle(InstIns8070::BRA_HERE);
                Signals::discard(signals);
                break;
            }
            completeCycle(signals);
        }
    } else {
        for (auto c = 0; c < cycles; ++c) {
            cycle();
        }
    }
    if (show)
        printCycles();
    Regs.save();
    return true;
}

void PinsIns8070::assertInt(uint8_t name) {
    // #INTA is negative-edge sensed.
    assert_sa();
    delayNanoseconds(20);
    negate_sa();
}

void PinsIns8070::negateInt(uint8_t name) {
    negate_sa();
}

void PinsIns8070::setBreakInst(uint32_t addr) const {
    Memory.put_inst(addr, InstIns8070::CALL15);
}

void PinsIns8070::printCycles(const Signals *end) {
    const auto g = Signals::get();
    const auto cycles = g->diff(end);
    for (auto i = 0; i < cycles; ++i) {
        g->next(i)->print();
        idle();
    }
}

bool PinsIns8070::matchAll(Signals *begin, const Signals *end) {
    const auto cycles = begin->diff(end);
    LOG_MATCH(cli.print("@@  matchAll: begin="));
    LOG_MATCH(begin->print());
    LOG_MATCH(cli.print("@@           cycles="));
    LOG_MATCH(cli.printlnDec(cycles));
    for (auto i = 0; i < cycles;) {
        idle();
        auto s = begin->next(i);
        InstIns8070 inst;
        if (inst.match(s, end->next())) {
            s->markFetch(true);
            i += inst.matchedCycles();
            continue;
        }
        return false;
    }
    return true;
}
const Signals *PinsIns8070::findFetch(Signals *begin, const Signals *end) {
    const auto cycles = begin->diff(end);
    LOG_MATCH(cli.print("@@ findFetch: begin="));
    LOG_MATCH(begin->print());
    LOG_MATCH(cli.print("@@              end="));
    LOG_MATCH(end->print());
    for (auto i = 0; i < cycles; ++i) {
        idle();
        auto s = begin->next(i);
        if (matchAll(s, end))
            return s;
        for (auto j = i; j < cycles; ++j)
            begin->next(j)->markFetch(false);
    }
    return end;
}

void PinsIns8070::disassembleCycles() {
    const auto end = Signals::put();
    const auto begin = findFetch(Signals::get(), end);
    printCycles(begin);
    const auto cycles = begin->diff(end);
    for (auto i = 0; i < cycles;) {
        const auto s = begin->next(i);
        if (s->fetchMark()) {
            const auto len = Memory.disassemble(s->addr, 1) - s->addr;
            if (InstIns8070::isJsr(s->data)) {
                s->next(2)->print();
                s->next(3)->print();
                i += 5;
            } else {
                i += len;
            }
        } else {
            s->print();
            ++i;
        }
        idle();
    }
}

}  // namespace ins8070
}  // namespace debugger

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
