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
 *          __    __    __    __    __    __    __    __    __
 * EXTAL __|c1|__|c2|__|c3|__|c4|__|c1|__|c2|__|c3|__|c4|__|c1|_
 *             \___________\           \___________\
 *     Q ______|           |___________|           |___________|
 *                   \___________\           \___________\     \
 *     E ____________|           |___________|           |______
 *             ________________________
 *   R/W -----<                        |________________________
 *                           ____               _________
 *  Data -------------------<rrrr>-------------<wwwwwwwww>------
 *
 * - EXTAL falling-edge to E and Q edges takes 40ns.
 * - R/W, Address, BA and BS are valid before 40ns of rising edge of Q.
 * - Read data setup to falling E egde is 80ns.
 * - Read data hold to falling E edge is 10ns.
 * - Write data gets valid after 200ns of rising Q edge.
 */

#if defined(ARDUINO_TEENSY35)
static constexpr auto extal_hi_ns = 120;
static constexpr auto extal_lo_ns = 120;
static constexpr auto delay_extal_e_ns = 60;
static constexpr auto c1_lo_ns = 0;
static constexpr auto c1_hi_ns = 45;
static constexpr auto c2_lo_ns = 45;
static constexpr auto c2_hi_ns = 115;
static constexpr auto c3_read_lo_ns = 5;
static constexpr auto c3_read_hi_ns = 0;
static constexpr auto c4_read_lo_ns = 0;
static constexpr auto c4_read_hi_ns = 0;
static constexpr auto c3_write_lo_ns = 5;
static constexpr auto c3_write_hi_ns = 0;
static constexpr auto c4_write_lo_ns = 0;
static constexpr auto c4_write_hi_ns = 0;
static constexpr auto next_c1_lo_ns = 0;
#endif
#if defined(ARDUINO_TEENSY41)
static constexpr auto extal_hi_ns = 103;
static constexpr auto extal_lo_ns = 33;
static constexpr auto delay_extal_e_ns = 40;
static constexpr auto c1_lo_ns = 0;
static constexpr auto c1_hi_ns = 78;
static constexpr auto c2_lo_ns = 60;
static constexpr auto c2_hi_ns = 101;
static constexpr auto c3_read_lo_ns = 89;
static constexpr auto c3_read_hi_ns = 68;
static constexpr auto c4_read_lo_ns = 74;
static constexpr auto c4_read_hi_ns = 55;
static constexpr auto c3_write_lo_ns = 89;
static constexpr auto c3_write_hi_ns = 75;
static constexpr auto c4_write_lo_ns = 68;
static constexpr auto c4_write_hi_ns = 53;
static constexpr auto next_c1_lo_ns = 5;
#endif

static inline void extal_hi() __attribute__((always_inline));
static inline void extal_hi() {
    digitalWriteFast(PIN_EXTAL, HIGH);
}

static inline void extal_lo() __attribute__((always_inline));
static inline void extal_lo() {
    digitalWriteFast(PIN_EXTAL, LOW);
}

static inline void clock_cycle() __attribute__((always_inline));
static inline void clock_cycle() {
    extal_hi();
    delayNanoseconds(extal_hi_ns);
    extal_lo();
    delayNanoseconds(extal_lo_ns);
}

static inline uint8_t clock_q() __attribute__((always_inline));
static inline uint8_t clock_q() {
    return digitalReadFast(PIN_Q);
}

static inline uint8_t clock_e() __attribute__((always_inline));
static inline uint8_t clock_e() {
    return digitalReadFast(PIN_E);
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

static void assert_mrdy() __attribute__((unused));
static void assert_mrdy() {
    digitalWriteFast(PIN_MRDY, LOW);
}

static void negate_mrdy() {
    digitalWriteFast(PIN_MRDY, HIGH);
}

static void assert_breq() __attribute__((unused));
static void assert_breq() {
    digitalWriteFast(PIN_BREQ, LOW);
}

static void negate_breq() {
    digitalWriteFast(PIN_BREQ, HIGH);
}

static void assert_reset() {
    // Drive RESET condition
    digitalWriteFast(PIN_RESET, LOW);
    negate_halt();
    negate_nmi();
    negate_irq();
    negate_firq();
    negate_mrdy();
    negate_breq();
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
    pinMode(PIN_Q, INPUT);
    pinMode(PIN_FIRQ, OUTPUT);
    pinMode(PIN_EXTAL, OUTPUT);
    pinMode(PIN_E, INPUT);
    pinMode(PIN_XTAL, INPUT_PULLUP);
    pinMode(PIN_BS, INPUT);
    pinMode(PIN_MRDY, OUTPUT);
    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_BREQ, OUTPUT);
    pinMode(PIN_BA, INPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    assert_reset();
    extal_lo();
    _freeRunning = false;

    setDeviceBase(Device::ACIA);
}

Signals &Pins::cycle() {
    // MC6809 clock E is CLK/4, so we toggle CLK 4 times
    // c1
    delayNanoseconds(c1_lo_ns);
    Signals &signals = Signals::currCycle();
    signals.getDirection();
    extal_hi();
    delayNanoseconds(c1_hi_ns);
    signals.getAddr1();
    extal_lo();
    // c2
    signals.getAddr2();
    delayNanoseconds(c2_lo_ns);
    extal_hi();
    delayNanoseconds(c2_hi_ns);
    extal_lo();

    if (signals.rw == LOW) {
        // c3
        delayNanoseconds(c3_write_lo_ns);
        extal_hi();
        delayNanoseconds(c3_write_hi_ns);
        signals.getData();
        extal_lo();
        // c4
        if (Acia.isSelected(signals.addr)) {
            Acia.write(signals.debug('a').data, signals.addr);
        } else if (signals.writeRam()) {
            Memory.raw_write(signals.addr, signals.debug('m').data);
        } else {
            ;  // capture data to signals.data
        }
        delayNanoseconds(c4_write_lo_ns);
        extal_hi();
        Signals::nextCycle();
        delayNanoseconds(c4_write_hi_ns);
    } else {
        // c3
        delayNanoseconds(c3_read_lo_ns);
        extal_hi();
        if (Acia.isSelected(signals.addr)) {
            signals.debug('a').data = Acia.read(signals.addr);
        } else if (signals.readRam()) {
            signals.debug('m').data = Memory.raw_read(signals.addr);
        } else {
            ;  // inject data from signals.data
        }
        delayNanoseconds(c3_read_hi_ns);
        extal_lo();
        // c4
        delayNanoseconds(c4_read_lo_ns);
        busWrite(D, signals.data);
        extal_hi();
        // change data bus to output
        busMode(D, OUTPUT);
        Signals::nextCycle();
        delayNanoseconds(c4_read_hi_ns);
    }
    // To ensure data hold time (10ns).
    extal_lo();
    delayNanoseconds(next_c1_lo_ns);
    busMode(D, INPUT);

    return signals;
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
    // Synchronize EXTAL input and Q and E output
    while (true) {
        clock_cycle();
        delayNanoseconds(delay_extal_e_ns);
        if (clock_q() == LOW && clock_e() != LOW)
            break;
    }
    while (clock_e() != LOW) {
        clock_cycle();
        delayNanoseconds(delay_extal_e_ns);
    }
    // At least one #RESET cycle.
    cycle().debug('R');
    Signals::resetCycles();
    cycle().debug('R');
    negate_reset();
    while (true) {
        Signals &signals = cycle();
        if (signals.ba == LOW && signals.bs != LOW) {
            // MSB of reset vector has read.
            signals.debug('v');
            break;
        }
        signals.debug('r');
    }
    // LSB of reset vector;
    cycle().debug('v');
    // non-VMA
    cycle().debug('-');
    Regs.reset();
    Regs.save(show);
    if (show)
        Signals::printCycles();
}

void Pins::idle() {
    static const uint8_t BRA[] = { 0x20, 0xFE, 0xFF }; // BRA *
    execInst(BRA, sizeof(BRA));
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
    // Wait for consequtive 12 (or 14 writes in native 6309 mode)
    // which means registers saved onto stack.
    bool native6309 = true;
    while (writes < 14) {
        Signals::capture();
        Signals &signals = cycle();
        if (signals.rw == LOW) {
            writes++;
            signals.debug('0' + writes);
        } else if (writes == 12) {
            native6309 = false;
            break;
        } else {
            writes = 0;
        }
    }
    negate_nmi();
    // Capture registers pushed onto stack.
    Signals *end = &Signals::currCycle() - writes;
    if (native6309) {
        // Non-VMA cycle
        cycle().debug('-');
    } else {
        // The last cycle was Non-VMA cycle.
        end->debug('-');
        end--;
    }
    // Inject the current PC as NMI vector.
    Signals::inject(Regs.pc >> 8);
    cycle().debug('v');
    Signals::inject(Regs.pc);
    cycle().debug('v');
    Signals::flushWrites(end);
    // Non-VMA cycle
    cycle().debug('-');
    Regs.capture(end, native6309);
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
