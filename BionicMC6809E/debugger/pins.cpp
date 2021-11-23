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
 * MC6809 bus cycle.
 *               ___________             ___________
 *     Q _______|           |___________|           |___________|
 *               \     ___________       \     ___________
 *     E |____________|           |___________|           |______
 *        \    ____________________\                       \
 *   R/W -----<                    |_______________________>-----
 *                           ____               _________
 *  Data -------------------<rrrr>-------------<wwwwwwwww>-------
 *       ________       _________________       _________________
 *  AVMA ________>-----<_________________>-----<_________________>
 *
 * - Minimum pulse width of Q and E are 450 ns.
 * - Minimum delay time between Q and E edges are 200ns.
 * - R/W, Address, BA and BS are valid after 200ns of falling edge of E.
 * - R/W, Address, BA and BS get invalid after 20ns of falling edge of E.
 * - AVMA, BUSY and LIC get valid after 300ns of raising edge of Q.
 * - Read data setup to falling E egde is 80ns.
 * - Read data hold to falling E edge is 10ns.
 * - Write data gets valid after 200ns of raising Q edge.
 */

#if defined(ARDUINO_TEENSY35)
static constexpr auto raw_e_lo_q_lo_ns = 0;
static constexpr auto e_lo_q_lo_ns = 0;
static constexpr auto e_lo_q_hi_ns = 100;
static constexpr auto e_hi_q_hi_novma_ns = 0;
static constexpr auto e_hi_q_lo_novma_ns = 0;
static constexpr auto e_hi_q_hi_write_ns = 0;
static constexpr auto e_hi_q_lo_write_ns = 0;
static constexpr auto e_hi_q_hi_read_ns = 0;
static constexpr auto e_hi_q_lo_read_ns = 0;
#endif
#if defined(ARDUINO_TEENSY41)
static constexpr auto raw_e_lo_q_lo_ns = 30;
static constexpr auto e_lo_q_lo_ns = 0;
static constexpr auto e_lo_q_hi_ns = 122;
static constexpr auto e_hi_q_hi_novma_ns = 100;
static constexpr auto e_hi_q_lo_novma_ns = 120;
static constexpr auto e_hi_q_hi_write_ns = 100;
static constexpr auto e_hi_q_lo_write_ns = 60;
static constexpr auto e_hi_q_hi_read_ns = 40;
static constexpr auto e_hi_q_lo_read_ns = 70;
#endif

static inline void q_hi() __attribute__((always_inline));
static inline void q_hi() {
    digitalWriteFast(PIN_Q, HIGH);
}

static inline void e_hi() __attribute__((always_inline));
static inline void e_hi() {
    digitalWriteFast(PIN_E, HIGH);
}

static inline void q_lo() __attribute__((always_inline));
static inline void q_lo() {
    digitalWriteFast(PIN_Q, LOW);
}

static inline void e_lo() __attribute__((always_inline));
static inline void e_lo() {
    digitalWriteFast(PIN_E, LOW);
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

static void assert_firq() __attribute__((unused));
static void assert_firq() {
    digitalWriteFast(PIN_FIRQ, LOW);
}

static void negate_firq() {
    digitalWriteFast(PIN_FIRQ, HIGH);
}

static void assert_halt() __attribute__((unused));
static void assert_halt() {
    digitalWriteFast(PIN_HALT, LOW);
}

static void negate_halt() __attribute__((unused));
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
    negate_firq();
    negate_tsc();
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
    pinMode(PIN_BUSY, INPUT);
    pinMode(PIN_FIRQ, OUTPUT);
    pinMode(PIN_E, OUTPUT);
    pinMode(PIN_Q, OUTPUT);
    pinMode(PIN_AVMA, INPUT);
    pinMode(PIN_BS, INPUT);
    pinMode(PIN_LIC, INPUT);
    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_TSC, OUTPUT);
    pinMode(PIN_BA, INPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    assert_reset();
    q_lo();
    e_lo();
    _freeRunning = false;

    setIoDevice(SerialDevice::DEV_ACIA, ioBaseAddress());
}

Signals &Pins::cycle() {
    static uint8_t vma = LOW;

    // E=L Q=L
    delayNanoseconds(e_lo_q_lo_ns);
    Signals &signals = Signals::currCycle();
    signals.getDirection();
    q_hi();
    // E=L Q=H
    signals.getAddr();
    delayNanoseconds(e_lo_q_hi_ns);
    e_hi();
    // E=H Q=H
    signals.getControl();
    if (vma == LOW) {
        delayNanoseconds(e_hi_q_hi_novma_ns);
        q_lo();
        // E=H Q=L
        signals.debug('-');
        Signals::nextCycle();
        delayNanoseconds(e_hi_q_lo_novma_ns);
    } else if (signals.rw == LOW) {
        delayNanoseconds(e_hi_q_hi_write_ns);
        q_lo();
        // E=H Q=L
        signals.getData();
        if (Acia.isSelected(signals.addr)) {
            Acia.write(signals.debug('a').data, signals.addr);
        } else if (signals.writeRam()) {
            Memory.raw_write(signals.addr, signals.debug('m').data);
        } else {
            ;  // capture data to signals.data
        }
        Signals::nextCycle();
        delayNanoseconds(e_hi_q_lo_write_ns);
    } else {
        // E=H Q=L
        if (Acia.isSelected(signals.addr)) {
            signals.debug('a').data = Acia.read(signals.addr);
        } else if (signals.readRam()) {
            signals.debug('m').data = Memory.raw_read(signals.addr);
        } else {
            ;  // inject data from signals.data
        }
        delayNanoseconds(e_hi_q_hi_read_ns);
        q_lo();
        busWrite(D, signals.data);
        // change data bus to output
        busMode(D, OUTPUT);
        Signals::nextCycle();
        delayNanoseconds(e_hi_q_lo_read_ns);
    }
    // E=L Q=L
    e_lo();
    // Set E clock low to handle hold times and tristate data bus.
    vma = signals.avma;
    busMode(D, INPUT);

    return signals;
}

Signals &Pins::raw_cycle() {
    delayNanoseconds(raw_e_lo_q_lo_ns);
    return cycle();
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
    for (uint8_t i = 0; i < len; i++) {
        Signals::inject(inst[i]);
        raw_cycle();
    }
    uint8_t cap = 0;
    if (buf) {
        while (cap < max) {
            Signals::capture();
            const Signals &signals = raw_cycle();
            if (cap == 0 && addr)
                *addr = signals.addr;
            buf[cap++] = signals.data;
        }
    }
    return cap;
}

void Pins::reset(bool show) {
    assert_reset();
    for (auto i = 0; i < 3; i++)
        raw_cycle().debug('R');
    Signals::resetCycles();
    raw_cycle().debug('R');
    negate_reset();
    // Three FFFE cycles.
    raw_cycle().debug('r');
    raw_cycle().debug('r');
    raw_cycle().debug('r');
    // Read Reset vector
    raw_cycle().debug('v');
    raw_cycle().debug('v');
    // non-VMA
    raw_cycle().debug('-');
    Regs.save();
    if (show)
        Signals::printCycles();
}

void Pins::idle() {
    // Inject "BRA *"
    Signals::inject(0x20);
    raw_cycle();
    Signals::inject(0xFE);
    raw_cycle();
    Signals::inject(0);
    raw_cycle();
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
    // Wait for consequtive 12 writes which means registers saved onto stack.
    while (writes < 12) {
        Signals::capture();
        Signals &signals = raw_cycle();
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
    // Non-VMA cycle
    raw_cycle().debug('-');
    // Inject the current PC as NMI vector.
    Signals::inject(Regs.pc >> 8);
    raw_cycle().debug('v');
    Signals::inject(Regs.pc);
    raw_cycle().debug('v');
    // Non-VMA cycle
    raw_cycle().debug('-');
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
    negate_halt();
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

Pins::SerialDevice Pins::getIoDevice(uint16_t &baseAddr) {
    baseAddr = Acia.baseAddr();
    return _ioDevice;
}

void Pins::setIoDevice(SerialDevice device, uint16_t baseAddr) {
    _ioDevice = device;
    Acia.enable(true, baseAddr);
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
