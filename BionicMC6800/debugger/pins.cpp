#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "mc6850.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli cli;

class Pins Pins;
Mc6850 Acia(Console);

static constexpr bool debug_cycles = false;

/**
 * MC6800 bus cycle.
 *         __________            __________            _____
 *  PHI1 _|          |__________|          |__________|
 *       _     _________________     _________________     _
 *   DBE  |___|                 |___|                 |___|
 *       _            __________            __________
 *  PHI2  |__________|          |__________|          |______
 *       ________  ____________________  ____________________
 *   VMA ________><____________________><____________________
 *                ______________________
 *   R/W --------<                      |____________________
 *                              ____               _________
 *  Data -----------------------<rrrr>-------------<wwwwwwwww>---
 *
 * - Minimum DBE asserted period is 150ns.
 * - Minimum PHI1/PHI2 high period is 400ns.
 * - Minimum overrapping of PHI1 and PHI2 is 0ns.
 * - Maximum separation of PHI1 and PHI2 is 9100ns.
 * - PHI1 rising-edge to valid VMA, R/W and address is 270ns.
 * - Read data setup to falling PHI2 egde is 100ns.
 * - Read data hold to falling PHI2 edge is 10ns.
 * - Write data gets valid after 225ns of rising DBE edge.
 */

#if defined(ARDUINO_TEENSY41)
static constexpr auto dbe_negate_ns = 120;
static constexpr auto phi1_hi_ns = 180;
static constexpr auto phi2_novma_hi_ns = 280;
static constexpr auto phi2_write_hi_ns = 220;
static constexpr auto phi2_read_hi_ns = 200;
#endif

static inline void phi1_hi_dbe_lo() __attribute__((always_inline));
static inline void phi1_hi_dbe_lo() {
    digitalWriteFast(PIN_PHI1, HIGH);
    digitalWriteFast(PIN_DBE, LOW);
}

static inline void phi1_lo_phi2_hi() __attribute__((always_inline));
static inline void phi1_lo_phi2_hi() {
    digitalWriteFast(PIN_PHI1, LOW);
    digitalWriteFast(PIN_PHI2, HIGH);
}

static inline void phi2_lo() __attribute__((always_inline));
static inline void phi2_lo() {
    digitalWriteFast(PIN_PHI2, LOW);
}

static inline void assert_dbe() __attribute__((always_inline));
static inline void assert_dbe() {
    digitalWriteFast(PIN_DBE, HIGH);
}

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

// TODO: Utilize #HALT to Pins.step()
static void assert_halt() __attribute__((unused));
static void assert_halt() {
    digitalWriteFast(PIN_HALT, LOW);
}

static void negate_halt() {
    digitalWriteFast(PIN_HALT, HIGH);
}

static void negate_tsc() {
    digitalWriteFast(PIN_TSC, LOW);
}

static void assert_reset() {
    // Drive RESET condition
    digitalWriteFast(PIN_RESET, LOW);
    negate_halt();
    negate_nmi();
    negate_irq();
    negate_tsc();
    assert_dbe();
}

static void negate_reset() {
    // Release RESET conditions
    digitalWriteFast(PIN_RESET, HIGH);
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
        pinMode(BUS_PINS[i], INPUT_PULLUP);
    busMode(D, INPUT);
    busMode(AL, INPUT);
#if defined(AM_vp)
    busMode(AM, INPUT);
#endif
    busMode(AH, INPUT);

    pinMode(PIN_HALT, OUTPUT);
    pinMode(PIN_RW, INPUT);
    pinMode(PIN_IRQ, OUTPUT);
    pinMode(PIN_NMI, OUTPUT);
    pinMode(PIN_PHI1, OUTPUT);
    pinMode(PIN_PHI2, OUTPUT);
    pinMode(PIN_VMA, INPUT);
    pinMode(PIN_DBE, OUTPUT);
    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_TSC, OUTPUT);
    pinMode(PIN_BA, INPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    assert_reset();

    _freeRunning = false;
    turn_off_led();

    setDeviceBase(Device::ACIA);
}

Signals &Pins::cycle() {
    // PHI1=HIGH
    phi1_hi_dbe_lo();
    Signals &signals = Signals::currCycle();
    delayNanoseconds(dbe_negate_ns);
    assert_dbe();
    delayNanoseconds(phi1_hi_ns);
    signals.getDirection();
    phi1_lo_phi2_hi();
    // PHI2=HIGH
    signals.getAddr();

    if (signals.vma == LOW) {
        signals.debug('-');
        delayNanoseconds(phi2_novma_hi_ns);
    } else if (signals.rw == LOW) {
        signals.getData();
        if (Acia.isSelected(signals.addr)) {
            Acia.write(signals.debug('a').data, signals.addr);
        } else if (signals.writeRam()) {
            Memory.raw_write(signals.addr, signals.debug('m').data);
        } else {
            ;  // capture data to signals.data
        }
        delayNanoseconds(phi2_write_hi_ns);
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
        delayNanoseconds(phi2_read_hi_ns);
    }
    Signals::nextCycle();
    phi2_lo();
    // Set clock low to handle hold times and tristate data bus.
    busMode(D, INPUT);

    return signals;
}

void Pins::execInst(const uint8_t *inst, uint8_t len) {
    execute(inst, len, nullptr, nullptr, 0);
}

uint8_t Pins::captureWrites(const uint8_t *inst, uint8_t len, uint16_t *addr,
        void *buf, uint8_t max) {
    return execute(inst, len, addr, (uint8_t *)buf, max);
}

uint8_t Pins::execute(const uint8_t *inst, uint8_t len, uint16_t *addr,
        uint8_t *buf, uint8_t max) {
    for (uint8_t i = 0; i < len; i++) {
        Signals::inject(inst[i]);
        cycle();
    }
    uint8_t cap = 0;
    if (buf) {
        while (cap < max) {
            Signals::capture();
            const Signals &signals = cycle();
            if (cap == 0 && addr)
                *addr = signals.addr;
            buf[cap++] = signals.data;
        }
    }
    return cap;
}

void Pins::reset(bool show) {
    assert_reset();
    cycle();
    Signals::resetCycles();
    cycle().debug('R');
    negate_reset();
    cycle().debug('r');
    // Read Reset vector
    cycle().debug('v');
    cycle().debug('v');
    if (show)
        Signals::printCycles();
    // The first instruction will be saving registers, and certainly can be
    // injected.
    Regs.reset();
    Regs.save(show);

    Acia.reset();
}

void Pins::idle() {
    // Inject "BRA *"
    Signals::inject(0x20);
    cycle();
    Signals::inject(0xFE);
    cycle();
    Signals::inject(0);
    cycle();
    Signals::inject(0);
    cycle();
}

void Pins::loop() {
    if (_freeRunning) {
        Acia.loop();
        cycle();
        if (user_sw() == LOW)
            Commands.halt(true);
    } else {
        idle();
    }
}

void Pins::suspend(bool show) {
    uint8_t writes = 0;
    assert_nmi();
    // Wait for consequtive 7 writes which means registers saved onto stack.
    while (writes < 7) {
        Signals::capture();
        Signals &signals = cycle();
        if (signals.rw == LOW) {
            writes++;
            signals.debug('0' + writes);
        } else {
            writes = 0;
        }
    }
    negate_nmi();
    // Capture registers pushed onto stack.
    const Signals *end = &Signals::currCycle() - writes;
    Regs.capture(end);
    cycle().debug('n');
    // Inject the current PC as NMI vector.
    Signals::inject(Regs.pc >> 8);
    cycle().debug('v');
    Signals::inject(Regs.pc);
    cycle().debug('v');
    Signals::flushWrites(end);
    if (debug_cycles) {
        Signals::printCycles();
    } else if (show) {
        Signals::printCycles(end);
    }
}

void Pins::halt(bool show) {
    if (_freeRunning) {
        Signals::resetCycles();
        suspend(show);
        _freeRunning = false;
        turn_off_led();
    }
}

void Pins::run() {
    Regs.restore(debug_cycles);
    _freeRunning = true;
    turn_on_led();
}

void Pins::step(bool show) {
    Regs.restore(debug_cycles);
    Signals::resetCycles();
    suspend(show);
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
