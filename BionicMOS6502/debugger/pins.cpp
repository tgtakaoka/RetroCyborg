#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "mc6850.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli &cli;

class Pins Pins;
Mc6850 Acia(Console);

static constexpr bool debug_cycles = false;

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

#if defined(ARDUINO_TEENSY35)
static constexpr auto phi0_raw_lo_ns = 200;
static constexpr auto phi0_lo_ns = 30;
static constexpr auto phi0_read_hi_ns = 500;
static constexpr auto phi0_write_hi_ns = 500;
#endif
#if defined(ARDUINO_TEENSY41)
static constexpr auto phi0_raw_lo_ns = 195;
static constexpr auto phi0_lo_ns = 30;
static constexpr auto phi0_read_hi_ns = 370;
static constexpr auto phi0_write_hi_ns = 390;
#endif

static inline void phi0_hi() __attribute__((always_inline));
static inline void phi0_hi() {
    digitalWriteFast(PIN_PHI0, HIGH);
}

static inline void phi0_lo() __attribute__((always_inline));
static inline void phi0_lo() {
    digitalWriteFast(PIN_PHI0, LOW);
}

static void assert_abort() __attribute__((unused));
static void assert_abort() {
    digitalWriteFast(W65C816S_ABORT, LOW);
}

static void negate_abort() {
    digitalWriteFast(W65C816S_ABORT, HIGH);
}

static void assert_nmi() __attribute__((unused));
static void assert_nmi() {
    digitalWriteFast(PIN_NMI, LOW);
}

static void negate_nmi() {
    digitalWriteFast(PIN_NMI, HIGH);
}

static void assert_irq() {
    digitalWriteFast(PIN_IRQ, LOW);
}

static void negate_irq() {
    digitalWriteFast(PIN_IRQ, HIGH);
}

static void assert_be() __attribute__((unused));
static void assert_be() {
    digitalWriteFast(PIN_BE, HIGH);
}

static void negate_be() __attribute__((unused));
static void negate_be() {
    digitalWriteFast(PIN_BE, LOW);
}

static void assert_rdy() __attribute__((unused));
static void assert_rdy() {
    digitalWriteFast(PIN_RDY, HIGH);
    pinMode(PIN_RDY, INPUT_PULLUP);
}

static void negate_rdy() __attribute__((unused));
static void negate_rdy() {
    digitalWriteFast(PIN_RDY, LOW);
    pinMode(PIN_RDY, OUTPUT_OPENDRAIN);
}

static void assert_reset() {
    // Drive RESET condition
    phi0_lo();
    negate_be();
    digitalWriteFast(PIN_RES, LOW);
    negate_abort();
    negate_nmi();
    negate_irq();
    assert_rdy();
}

static void negate_reset() {
    // Release RESET conditions
    digitalWriteFast(PIN_RES, HIGH);
}

static void turn_on_led() {
    digitalWriteFast(PIN_USRLED, LOW);
}

static void turn_off_led() {
    digitalWriteFast(PIN_USRLED, HIGH);
}

static void toggle_led() __attribute__((unused));
static void toggle_led() {
    digitalToggleFast(PIN_USRLED);
}

static uint8_t user_sw() {
    return digitalReadFast(PIN_USRSW);
}

static const uint8_t BUS_PINS[] = {
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
#if defined(PIN_AL8)
        PIN_AL8,
        PIN_AL9,
        PIN_AL10,
        PIN_AL11,
#endif
#if defined(PIN_AM8)
        PIN_AM8,
        PIN_AM9,
        PIN_AM10,
        PIN_AM11,
#endif
        PIN_AH12,
        PIN_AH13,
        PIN_AH14,
        PIN_AH15,
};

void Pins::begin() {
    // Set directions.
    for (uint8_t i = 0; i < sizeof(BUS_PINS); i++)
        pinMode(BUS_PINS[i], INPUT);  // pulledup
    busMode(D, INPUT);
    busMode(AL, INPUT);
#if defined(AM_vp)
    busMode(AM, INPUT);
#endif
    busMode(AH, INPUT);

    pinMode(PIN_PHI0, OUTPUT);
    pinMode(PIN_RES, OUTPUT);
    pinMode(PIN_NMI, OUTPUT);  // pulledup
    pinMode(PIN_IRQ, OUTPUT);  // pulledup
    pinMode(PIN_RW, INPUT);
    // RDY(input) for 6502
    // RDY(bi-directional) for W65Cxx, pulledup
    pinMode(PIN_RDY, INPUT_PULLUP);
    // SYNC(output) from 6502
    // VPA(output) from 65816
    pinMode(PIN_SYNC, INPUT);
    // NC on 6502
    // BE(input) for W65Cxx
    pinMode(PIN_BE, OUTPUT);
    // #SO(input) for 6502, pulledup
    // #MX(output) from 65816
    pinMode(PIN_SO, INPUT);
    // PHI1O(output) from 6502
    // #ABORT(input) for 65816, pulledup
    pinMode(PIN_PHI1O, INPUT_PULLUP);
    // PHI2O(output) from 6502
    // VDA(output) from 65816
    pinMode(PIN_PHI2O, INPUT);
    // NC on 6502
    // E(output) from 65816
    pinMode(PIN_E, INPUT_PULLUP);
    // NC for 6502
    // #ML(output) from W65Cxx
    pinMode(PIN_ML, INPUT_PULLUP);
    // VSS for 6502
    // #VP(output) from W65Cxx
    pinMode(PIN_VP, INPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    assert_reset();
    Signals::checkHardwareType();

    _freeRunning = false;
    turn_off_led();

    setDeviceBase(Device::ACIA);
}

Signals &Pins::prepareCycle() {
    Signals &signals = Signals::currCycle();
    delayNanoseconds(phi0_lo_ns);
    signals.getAddr();
    return signals;
}

Signals &Pins::completeCycle(Signals &signals) {
    // PHI2=HIGH
    phi0_hi();

    if (signals.rw == LOW) {
        delayNanoseconds(phi0_write_hi_ns);
        signals.getData();
        if (Acia.isSelected(signals.addr)) {
            Acia.write(signals.debug('a').data, signals.addr);
        } else if (signals.writeRam()) {
            Memory.raw_write(signals.addr, signals.debug('m').data);
        } else {
            ;  // capture data to signals.data
        }
    } else {
        if (Acia.isSelected(signals.addr)) {
            signals.debug('a').data = Acia.read(signals.addr);
        } else if (signals.readRam()) {
            signals.debug('m').data = Memory.raw_read(signals.addr);
        } else {
            ;  // inject data from signals.data
        }
        busWrite(D, signals.data);
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

Signals &Pins::cycle() {
    return completeCycle(prepareCycle());
}

Signals &Pins::rawPrepareCycle() {
    delayNanoseconds(phi0_raw_lo_ns);
    return prepareCycle();
}

Signals &Pins::rawCycle() {
    return completeCycle(rawPrepareCycle());
}

void Pins::execInst(const uint8_t *inst, uint8_t len) {
    execute(inst, len, nullptr, nullptr, 0);
}

uint8_t Pins::captureWrites(const uint8_t *inst, uint8_t len, uint16_t *addr,
        uint8_t *buf, uint8_t max) {
    return execute(inst, len, addr, buf, max);
}

uint8_t Pins::execute(const uint8_t *inst, uint8_t len, uint16_t *addr,
        uint8_t *buf, uint8_t max) {
    assert_rdy();
    while (true) {
        Signals &signals = rawPrepareCycle();
        if (signals.fetchInsn())
            break;
        completeCycle(signals);
    }
    uint8_t inj = 0;
    uint8_t cap = 0;
    while (inj < len || cap < max) {
        Signals &signals = rawPrepareCycle();
        if (signals.rw == LOW) {
            Signals::capture();
        } else if (inj < len) {
            Signals::inject(inst[inj++]);
        }
        completeCycle(signals);
        if (signals.rw == LOW) {
            if (buf) {
                if (cap == 0 && addr)
                    *addr = signals.addr;
                if (cap < max)
                    buf[cap++] = signals.data;
            }
        }
    }
    negate_rdy();
    return cap;
}

void Pins::reset(bool show) {
    assert_reset();
    // #RES must be held low for at lease two clock cycles.
    for (auto i = 0; i < 10; i++) {
        Signals::capture();  // there may be suprious write
        rawCycle().debug('R');
    }
    Signals::resetCycles();
    rawCycle().debug('R');
    assert_be();
    negate_reset();
    // When a positive edge is detected, there is an initalization
    // sequence lasting seven clock cycles.
    for (auto i = 0; i < 7; i++) {
        Signals::capture();  // there may be suprious write
        Signals &signals = rawPrepareCycle();
        if (signals.fetchVector() && signals.addr == Memory::reset_vector)
            break;
        completeCycle(signals).debug('r');
    }
    // Read Reset vector
    rawCycle().debug('v');
    rawCycle().debug('v');
    // Signals::printCycles() calls Pins::idle() and inject clocks.
    negate_rdy();
    // The first instruction will be saving registers, and certainly can be
    // injected.
    Regs.reset();
    Regs.save();
    if (show)
        Signals::printCycles();
}

void Pins::idle() {
    delayNanoseconds(398);
    phi0_hi();
    delayNanoseconds(472);
    phi0_lo();
}

void Pins::loop() {
    if (_freeRunning) {
        Acia.loop();
        const Signals &signals = cycle();
        if (signals.waiting()) {
            Regs.pc = signals.addr - 1;
            Commands.halt(true);
        }
        if (user_sw() == LOW)
            Commands.halt(true);
    } else {
        idle();
    }
}

void Pins::suspend() {
    while (true) {
        Signals &signals = rawPrepareCycle();
        if (signals.fetchInsn())
            break;
        completeCycle(signals);
        if (signals.waiting())
            break;
    }
    negate_rdy();
}

void Pins::halt(bool show) {
    if (_freeRunning) {
        Signals::resetCycles();
        suspend();
        if (show)
            Signals::printCycles();
        Regs.save(debug_cycles);
        _freeRunning = false;
        turn_off_led();
    }
}

void Pins::run() {
    Regs.restore(debug_cycles);
    unhalt();
    _freeRunning = true;
    turn_on_led();
}

Signals &Pins::unhalt() {
    assert_rdy();
    while (true) {
        Signals &signals = rawPrepareCycle();
        if (signals.fetchInsn())
            return signals;
        completeCycle(signals);
    }
}

void Pins::rawStep() {
    completeCycle(unhalt());
    while (true) {
        Signals &signals = rawPrepareCycle();
        if (signals.fetchInsn())
            break;
        completeCycle(signals);
        if (signals.waiting())
            break;
    }
    negate_rdy();
}

void Pins::step(bool show) {
    Regs.restore(debug_cycles);
    const auto insn = Memory.raw_read(Regs.pc);
    if (Signals::stopInsn(insn))
        return;
    Signals::resetCycles();
    rawStep();
    if (show)
        Signals::printCycles();
    Regs.save(debug_cycles);
}

uint8_t Pins::allocateIrq() {
    static uint8_t irq = 0;
    return irq++;
}

void Pins::assertIrq(uint8_t irq) {
    _irq |= (1 << irq);
    if (_irq)
        assert_irq();
}

void Pins::negateIrq(uint8_t irq) {
    _irq &= ~(1 << irq);
    if (_irq == 0)
        negate_irq();
}

static const char TEXT_ACIA[] PROGMEM = "ACIA";

Pins::Device Pins::parseDevice(const char *name) const {
    if (strcasecmp_P(name, TEXT_ACIA) == 0)
        return Device::ACIA;
    return Device::NONE;
}

void Pins::getDeviceName(Pins::Device dev, char *name) const {
    *name = 0;
    if (dev == Device::ACIA)
        strcpy_P(name, TEXT_ACIA);
}

void Pins::setDeviceBase(Pins::Device dev, bool hasValue, uint16_t base) {
    switch (dev) {
    case Device::ACIA:
        setSerialDevice(Device::ACIA, hasValue ? base : ACIA_BASE_ADDR);
        break;
    default:
        break;
    }
}

void Pins::printDevices() const {
    cli.println();
    uint16_t baseAddr;
    const auto serial = getSerialDevice(baseAddr);
    cli.print(F("ACIA (MC6850) "));
    if (serial == Device::ACIA) {
        cli.print(F("at $"));
        cli.printlnHex(baseAddr, 4);
    } else {
        cli.println(F("disabled"));
    }
}

Pins::Device Pins::getSerialDevice(uint16_t &baseAddr) const {
    if (_serialDevice == Device::ACIA) {
        baseAddr = Acia.baseAddr();
    }
    return _serialDevice;
}

void Pins::setSerialDevice(Pins::Device device, uint16_t baseAddr) {
    _serialDevice = device;
    if (device == Device::ACIA) {
        Acia.enable(true, baseAddr);
    }
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
