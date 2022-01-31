#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "mc6850.h"
#include "mc68hc11_sci_handler.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli &cli;

class Pins Pins;
Mc6850 Acia(Console);
Mc68hc11SciHandler<PIN_SCIRXD, PIN_SCITXD> SciH(Console);

static constexpr bool debug_cycles = false;

/**
 * MC68HC11 bus cycle.
 *          __    __    __    __    __    __    __    __    __
 * EXTAL __|c1|__|c2|__|c3|__|c4|__|c1|__|c2|__|c3|__|c4|__|c1|_
 *          \     \  \___________\  \     \  \___________\  \
 *     E ____________|           |___________|           |______
 *           _____                    _____   \               __
 *    AS ___|     |__________________|     |_________________|
 *       ________________________                         ______
 *   R/W                         |_______________________|
 *                           ____               _________
 *  Data -------------------<rrrr>-------------<wwwwwwwww>------
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

#if defined(ARDUINO_TEENSY35)
static constexpr auto delay_extal_e_ns = 100;
static constexpr auto extal_hi_ns = 117;
static constexpr auto extal_lo_ns = 0;
static constexpr auto raw_c1_lo_ns = 0;
static constexpr auto c1_lo_ns = 0;
static constexpr auto c1_hi_ns = 60;
static constexpr auto c2_lo_ns = 1;
static constexpr auto c2_hi_ns = 0;
static constexpr auto c3_read_lo_ns = 0;
static constexpr auto c3_read_hi_ns = 0;
static constexpr auto c4_read_lo_ns = 10;
static constexpr auto c4_read_hi_ns = 10;
static constexpr auto c3_write_lo_ns = 120;
static constexpr auto c3_write_hi_ns = 20;
static constexpr auto c4_write_lo_ns = 0;
static constexpr auto c4_write_hi_ns = 20;
static constexpr auto next_c1_lo_ns = 0;
#endif
#if defined(ARDUINO_TEENSY41)
static constexpr auto delay_extal_e_ns = 100;
static constexpr auto extal_hi_ns = 150;
static constexpr auto extal_lo_ns = 0;
static constexpr auto raw_c1_lo_ns = 60;
static constexpr auto c1_lo_ns = 0;
static constexpr auto c1_hi_ns = 96;
static constexpr auto c2_lo_ns = 90;
static constexpr auto c2_hi_ns = 45;
static constexpr auto c3_read_lo_ns = 58;
static constexpr auto c3_read_hi_ns = 50;
static constexpr auto c4_read_lo_ns = 77;
static constexpr auto c4_read_hi_ns = 60;
static constexpr auto c3_write_lo_ns = 100;
static constexpr auto c3_write_hi_ns = 80;
static constexpr auto c4_write_lo_ns = 34;
static constexpr auto c4_write_hi_ns = 58;
static constexpr auto next_c1_lo_ns = 40;
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

static inline uint8_t clock_e() __attribute__((always_inline));
static inline uint8_t clock_e() {
    return digitalReadFast(PIN_E);
}

static void assert_xirq() {
    digitalWriteFast(PIN_XIRQ, LOW);
    pinMode(PIN_XIRQ, OUTPUT_OPENDRAIN);
}

static void negate_xirq() {
    digitalWriteFast(PIN_XIRQ, HIGH);
    pinMode(PIN_XIRQ, INPUT);
}

static void assert_irq() {
    digitalWriteFast(PIN_IRQ, LOW);
    pinMode(PIN_IRQ, OUTPUT_OPENDRAIN);
}

static void negate_irq() {
    digitalWriteFast(PIN_IRQ, HIGH);
    pinMode(PIN_IRQ, INPUT);
}

static void assert_reset() {
    // Drive RESET with opend drain
    digitalWriteFast(PIN_RESET, LOW);
    pinMode(PIN_RESET, OUTPUT_OPENDRAIN);
    negate_xirq();
    negate_irq();
    extal_lo();
}

static void negate_reset() {
    // Release RESET conditions
    digitalWriteFast(PIN_RESET, HIGH);
    pinMode(PIN_RESET, INPUT);
}

static inline uint8_t reset_signal() __attribute__((always_inline));
static inline uint8_t reset_signal() {
    return digitalReadFast(PIN_RESET);
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
};

void Pins::begin() {
    // Set directions.
    for (uint8_t i = 0; i < sizeof(BUS_PINS); i++)
        pinMode(BUS_PINS[i], INPUT);
    busMode(AD, INPUT);
    busMode(AH, INPUT);

    pinMode(PIN_AS, INPUT);
    pinMode(PIN_RW, INPUT);
    pinMode(PIN_IRQ, OUTPUT_OPENDRAIN);
    pinMode(PIN_XIRQ, OUTPUT_OPENDRAIN);
    pinMode(PIN_EXTAL, OUTPUT);
    pinMode(PIN_E, INPUT);
    pinMode(PIN_MODA, INPUT);
    pinMode(PIN_MODB, INPUT);
    pinMode(PIN_RESET, OUTPUT_OPENDRAIN);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    _freeRunning = false;
    assert_reset();
    extal_lo();
    reset(true);

    setDeviceBase(Device::SCI);
}

Signals &Pins::cycle() {
    // MC68HC11 clock E is CLK/4, so we toggle CLK 4 times
    // c1
    delayNanoseconds(c1_lo_ns);
    extal_hi();
    Signals &signals = Signals::currCycle();
    delayNanoseconds(c1_hi_ns);
    extal_lo();
    // c2
    delayNanoseconds(c2_lo_ns);
    signals.getAddrMux();
    extal_hi();
    signals.getAddrNonmux();
    delayNanoseconds(c2_hi_ns);
    if (signals.rw == HIGH) {
        extal_lo();
        // c3
        if (SciH.isSelected(signals.addr)) {
            signals.debug('s').data = SciH.read(signals.addr);
        } else if (Acia.isSelected(signals.addr)) {
            signals.debug('a').data = Acia.read(signals.addr);
        } else if (signals.readRam()) {
            signals.debug('m').data = Memory.raw_read(signals.addr);
        } else {
            ;  // inject data from signals.data
        }
        delayNanoseconds(c3_read_lo_ns);
        extal_hi();
        busWrite(AD, signals.data);
        busMode(AD, OUTPUT);
        delayNanoseconds(c3_read_hi_ns);
        extal_lo();
        // c4
        delayNanoseconds(c4_read_lo_ns);
        signals.getControl();
        extal_hi();
        // change data bus to output
        Signals::nextCycle();
        delayNanoseconds(c4_read_hi_ns);
        // To ensure data hold time (10ns).
        extal_lo();
        delayNanoseconds(next_c1_lo_ns);
        busMode(AD, INPUT);
    } else {
        extal_lo();
        // c3
        delayNanoseconds(c3_write_lo_ns);
        extal_hi();
        delayNanoseconds(c3_write_hi_ns);
        signals.getControl();
        extal_lo();
        signals.getData();
        // c4
        if (SciH.isSelected(signals.addr)) {
            SciH.write(signals.debug('s').data, signals.addr);
        } else if (Acia.isSelected(signals.addr)) {
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
        extal_lo();
    }

    return signals;
}

Signals &Pins::raw_cycle() {
    delayNanoseconds(raw_c1_lo_ns);
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

static inline void inject_mode_pin(uint8_t pin, uint8_t val) {
    digitalWriteFast(pin, val ? HIGH : LOW);
    pinMode(pin, OUTPUT_OPENDRAIN);
}

static void inject_mode(uint8_t mode) {
    inject_mode_pin(PIN_MODB, mode & 2);
    inject_mode_pin(PIN_MODA, mode & 1);
}

static void release_mode() {
    // PIN_PC0 is now PIN_LIR
    pinMode(PIN_MODB, INPUT);
    pinMode(PIN_MODA, INPUT);
}

void Pins::reset(bool show) {
    // Reset vector should not point internal registers.
    Memory.setRamDevBase(_ramBaseAddress, _devBaseAddress);
    const uint16_t reset_vec = Memory.raw_read_uint16(Memory::reset_vector);
    if (Memory.is_internal(reset_vec))
        Memory.raw_write_uint16(Memory::reset_vector, 0x0123);

    // To get out from Clock Monitor Reset, inject EXTAL pulses
    while (reset_signal() == LOW) {
        negate_reset();
        clock_cycle();
    }
    assert_reset();

    // Synchronize clock output and E clock input.
    while (clock_e() == LOW) {
        clock_cycle();
        delayNanoseconds(delay_extal_e_ns);
    }
    while (clock_e() != LOW) {
        clock_cycle();
        delayNanoseconds(delay_extal_e_ns);
    }

    // #RESET should be at least 8 cycles
    for (auto i = 0; i < 8; i++)
        raw_cycle();

    Signals::resetCycles();
    // Mode Programming Setup Time for #RESET rising is 2 E cycles at minimum.
    inject_mode(MPU_MODE);
    raw_cycle().debug('R');
    raw_cycle().debug('R');
    raw_cycle().debug('R');

    negate_reset();
    raw_cycle().debug('r');
    // Mode Programming Hold Time: min 10ns
    release_mode();
    // Read Reset vector
    raw_cycle().debug('v');
    raw_cycle().debug('v');
    // The first instruction will be saving registers, and certainly can be
    // injected.
    Regs.save(false);
    Memory.disableCOP();
    Memory.setINIT();
    if (show)
        Signals::printCycles();
    if (Memory.is_internal(reset_vec)) {
        Memory.raw_write_uint16(Memory::reset_vector, reset_vec);
        Regs.pc = reset_vec;
    }

    Acia.reset();
    SciH.reset();
}

void Pins::idle() {
    // MC68HC11D is fully static, so we can stop clock safely
    // But not to trigger Clock Monitor Reset, we have to drive EXTAL anyway.
    Signals::inject(0x20);  // Inject "BRA *"
    raw_cycle();
    Signals::inject(0xFE);
    raw_cycle();
    Signals::inject(0xFF);
    raw_cycle();
}

void Pins::loop() {
    if (_freeRunning) {
        Acia.loop();
        SciH.loop();
        raw_cycle();
        if (user_sw() == LOW)
            Commands.halt(true);
    } else {
        idle();
    }
}

void Pins::suspend(bool show) {
    uint8_t writes = 0;
    assert_xirq();
    // Wait for consequtive 9 writes which means registers saved onto stack.
    while (writes < 9) {
        Signals::capture();
        Signals &signals = raw_cycle();
        if (signals.rw == LOW) {
            writes++;
            signals.debug('0' + writes);
        } else {
            writes = 0;
        }
    }
    negate_xirq();
    // Capture registers pushed onto stack.
    const Signals *end = &Signals::currCycle() - writes;
    Regs.capture(end);
    // Inject the current PC as XIRQ vector.
    raw_cycle().debug('-');
    Signals::inject(Regs.pc >> 8);
    raw_cycle().debug('v');
    Signals::inject(Regs.pc);
    raw_cycle().debug('v');
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
static const char TEXT_SCI[] PROGMEM = "SCI";
static const char TEXT_RAM[] PROGMEM = "RAM";
static const char TEXT_DEV[] PROGMEM = "DEV";

Pins::Device Pins::parseDevice(const char *name) const {
    if (strcasecmp_P(name, TEXT_ACIA) == 0)
        return Device::ACIA;
    if (strcasecmp_P(name, TEXT_SCI) == 0)
        return Device::SCI;
    if (strcasecmp_P(name, TEXT_RAM) == 0)
        return Device::RAM;
    if (strcasecmp_P(name, TEXT_DEV) == 0)
        return Device::DEV;
    return Device::NONE;
}

void Pins::getDeviceName(Pins::Device dev, char *name) const {
    *name = 0;
    if (dev == Device::ACIA)
        strcpy_P(name, TEXT_ACIA);
    if (dev == Device::SCI)
        strcpy_P(name, TEXT_SCI);
    if (dev == Device::RAM)
        strcpy_P(name, TEXT_RAM);
    if (dev == Device::DEV)
        strcpy_P(name, TEXT_DEV);
}

void Pins::setDeviceBase(Pins::Device dev, bool hasValue, uint16_t base) {
    switch (dev) {
    case Device::ACIA:
        setSerialDevice(Device::ACIA, hasValue ? base : ACIA_BASE_ADDR);
        break;
    case Device::SCI:
        setSerialDevice(Device::SCI, hasValue ? base : 0);
        break;
    case Device::RAM:
        setRamBaseAddress(hasValue ? base : 0);
        break;
    case Device::DEV:
        setDevBaseAddress(hasValue ? base : 0);
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
    cli.print(F("SCI ("));
    cli.print(Regs.cpuName());
    cli.print(F(") "));
    if (serial == Device::SCI) {
        cli.print(F("at $"));
        cli.printlnHex(baseAddr, 4);
    } else {
        cli.println(F("disabled"));
    }
    cli.print(F("RAM Base at $"));
    cli.printlnHex(ramBaseAddress(), 4);
    cli.print(F("DEV Base at $"));
    cli.printlnHex(devBaseAddress(), 4);
}

Pins::Device Pins::getSerialDevice(uint16_t &baseAddr) const {
    if (_serialDevice == Device::ACIA) {
        baseAddr = Acia.baseAddr();
    } else {
        baseAddr = SciH.baseAddr();
    }
    return _serialDevice;
}

void Pins::setSerialDevice(Pins::Device device, uint16_t baseAddr) {
    _serialDevice = device;
    if (device == Device::ACIA) {
        SciH.enable(false, 0);
        Acia.enable(true, baseAddr);
    } else {
        Acia.enable(false, 0);
        SciH.enable(true, _devBaseAddress);
    }
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
