#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "mc6801_sci_handler.h"
#include "mc6850.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli cli;

class Pins Pins;
Mc6850 Acia(Console);
Mc6801SciHandler<PIN_SCIRXD, PIN_SCITXD> SciH(Console);

static constexpr bool debug_cycles = false;

/**
 * MC6801 bus cycle.
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
 * - EXTAL falling-edge to E edges takes 50ns.
 * - EXTAL rising-edge of c1 to AS edges takes 40ns.
 * - EXTAL falling-edge of c4 to R/W edges takes 100ns.
 * - R/W is valid before rising edge of c1.
 * - Read data setup to falling E egde is 80ns.
 * - Write data gets valid after 225ns of rising E edge.
 */

#if defined(ARDUINO_TEENSY35)
static constexpr auto extal_hi_ns = 120;
static constexpr auto extal_lo_ns = 120;
static constexpr auto c1_lo_ns = 0;
static constexpr auto c1_hi_ns = 70;
static constexpr auto c2_lo_ns = 52;
static constexpr auto c2_hi_ns = 118;
static constexpr auto c3_read_lo_ns = 1;
static constexpr auto c3_read_hi_ns = 50;
static constexpr auto c4_read_lo_ns = 10;
static constexpr auto c4_read_hi_ns = 45;
static constexpr auto c3_write_lo_ns = 40;
static constexpr auto c3_write_hi_ns = 65;
static constexpr auto c4_write_lo_ns = 0;
static constexpr auto c4_write_hi_ns = 100;
static constexpr auto next_c1_lo_ns = 40;
#endif
#if defined(ARDUINO_TEENSY41)
static constexpr auto extal_hi_ns = 118;
static constexpr auto extal_lo_ns = 108;
static constexpr auto c1_lo_ns = 0;
static constexpr auto c1_hi_ns = 85;
static constexpr auto c2_lo_ns = 70;
static constexpr auto c2_hi_ns = 100;
static constexpr auto c3_read_lo_ns = 88;
static constexpr auto c3_read_hi_ns = 74;
static constexpr auto c4_read_lo_ns = 74;
static constexpr auto c4_read_hi_ns = 85;
static constexpr auto c3_write_lo_ns = 88;
static constexpr auto c3_write_hi_ns = 78;
static constexpr auto c4_write_lo_ns = 75;
static constexpr auto c4_write_hi_ns = 103;
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

static uint8_t clock_e() {
    return digitalReadFast(PIN_E);
}

static void assert_nmi() {
    digitalWriteFast(PIN_NMI, LOW);
    // #NMI is connected to P21/PC1 for LILBUG trace.
    pinMode(PIN_NMI, OUTPUT_OPENDRAIN);
}

static void negate_nmi() {
    digitalWriteFast(PIN_NMI, HIGH);
    // #NMI is connected to P21/PC1 for LILBUG trace.
    pinMode(PIN_NMI, INPUT_PULLUP);
}

static void assert_irq1() {
    digitalWriteFast(PIN_IRQ1, LOW);
}

static void negate_irq1() {
    digitalWriteFast(PIN_IRQ1, HIGH);
}

static void assert_reset() {
    // Drive RESET condition
    digitalWriteFast(PIN_RESET, LOW);
    negate_nmi();
    negate_irq1();

    // Toggle reset to put MC6803/HD6303 in reset
    clock_cycle();
    clock_cycle();
    clock_cycle();
    clock_cycle();
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

    pinMode(PIN_PC0, INPUT_PULLUP);
    pinMode(PIN_PC1, INPUT_PULLUP);
    pinMode(PIN_PC2, INPUT_PULLUP);
    pinMode(PIN_AS, INPUT);
    pinMode(PIN_RW, INPUT);
    pinMode(PIN_IRQ1, OUTPUT);
    // #NMI is connected to P21/PC1 for LILBUG trace.
    pinMode(PIN_NMI, OUTPUT_OPENDRAIN);
    pinMode(PIN_EXTAL, OUTPUT);
    pinMode(PIN_XTAL, INPUT_PULLUP);
    pinMode(PIN_E, INPUT);
    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    assert_reset();
    extal_lo();
    _freeRunning = false;

    setDeviceBase(Device::SCI);
}

Signals &Pins::cycle() {
    // MC6803/HD6303 clock E is CLK/4, so we toggle CLK 4 times
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
    if (signals.rw == HIGH) {
        // c3
        delayNanoseconds(c3_read_lo_ns);
        extal_hi();
        if (SciH.isSelected(signals.addr)) {
            signals.debug('s').data = SciH.read(signals.addr);
        } else if (Acia.isSelected(signals.addr)) {
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
        busWrite(AD, signals.data);
        extal_hi();
        // change data bus to output
        busMode(AD, OUTPUT);
        Signals::nextCycle();
        delayNanoseconds(c4_read_hi_ns);
    } else {
        // c3
        delayNanoseconds(c3_write_lo_ns);
        extal_hi();
        delayNanoseconds(c3_write_hi_ns);
        signals.getData();
        extal_lo();
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
    }
    // To ensure data hold time (10ns).
    extal_lo();
    delayNanoseconds(next_c1_lo_ns);
    busMode(AD, INPUT);

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

static inline void inject_mode_pin(uint8_t pin, uint8_t val) {
    if (val) {
        pinMode(pin, INPUT_PULLUP);
    } else {
        digitalWriteFast(pin, LOW);
        pinMode(pin, OUTPUT);
    }
}

static void inject_mode(uint8_t mode) {
    inject_mode_pin(PIN_PC2, mode & 4);
    inject_mode_pin(PIN_PC1, mode & 2);
    inject_mode_pin(PIN_PC0, mode & 1);
}

static void release_mode() {
    // #NMI is connected to P21/PC1 for LILBUG trace.
    pinMode(PIN_PC2, INPUT_PULLUP);
    pinMode(PIN_PC1, INPUT_PULLUP);
    pinMode(PIN_PC0, INPUT_PULLUP);
}

void Pins::reset(bool show) {
    // Reset vector should not point internal registers.
    const uint16_t reset_vec = Memory.raw_read_uint16(Memory::reset_vector);
    if (Memory::is_internal(reset_vec))
        Memory.raw_write_uint16(Memory::reset_vector, 0x0020);

    assert_reset();
    // Synchronize clock output and E clock input.
    while (clock_e() == LOW)
        clock_cycle();
    while (clock_e() != LOW)
        clock_cycle();
    Signals::resetCycles();
    // #RESET Low Pulse Width: min 3 E cycles
    cycle().debug('R');
    // Mode Programming Setup Time: min 2 E cycles
    inject_mode(MPU_MODE);
    cycle().debug('R');
    cycle().debug('R');
    negate_reset();
    cycle().debug('r');
    // Mode Programming Hold Time: min MC6803:100ns HD6303:150ns
    release_mode();
    if (Regs.isHd63())
        cycle().debug('e');
    // Read Reset vector
    cycle().debug('v');
    cycle().debug('v');
    if (show)
        Signals::printCycles();
    // The first instruction will be saving registers, and certainly can be
    // injected.
    Regs.save(show);
    if (Memory::is_internal(reset_vec)) {
        Memory.raw_write_uint16(Memory::reset_vector, reset_vec);
        Regs.pc = reset_vec;
    }

    Acia.reset();
    SciH.reset();
}

void Pins::idle() {
    // Inject "BRA *"
    Signals::inject(0x20);
    cycle();
    Signals::inject(0xFE);
    cycle();
    Signals::inject(0);
    cycle();
}

void Pins::loop() {
    if (_freeRunning) {
        Acia.loop();
        SciH.loop();
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
    if (!Regs.isHd63())
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
        assert_irq1();
}

void Pins::negateIrq(uint8_t irq) {
    _irq &= ~(1 << irq);
    if (_irq == 0)
        negate_irq1();
}

static const char TEXT_ACIA[] PROGMEM = "ACIA";
static const char TEXT_SCI[] PROGMEM = "SCI";

Pins::Device Pins::parseDevice(const char *name) const {
    if (strcasecmp_P(name, TEXT_ACIA) == 0)
        return Device::ACIA;
    if (strcasecmp_P(name, TEXT_SCI) == 0)
        return Device::SCI;
    return Device::NONE;
}

void Pins::getDeviceName(Pins::Device dev, char *name) const {
    *name = 0;
    if (dev == Device::ACIA)
        strcpy_P(name, TEXT_ACIA);
    if (dev == Device::SCI)
        strcpy_P(name, TEXT_SCI);
}

void Pins::setDeviceBase(Pins::Device dev, bool hasValue, uint16_t base) {
    switch (dev) {
    case Device::ACIA:
        setSerialDevice(Device::ACIA, hasValue ? base : ACIA_BASE_ADDR);
        break;
    case Device::SCI:
        setSerialDevice(Device::SCI, hasValue ? base : 0);
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
        SciH.enable(false);
        Acia.enable(true, baseAddr);
    } else {
        Acia.enable(false, 0);
        SciH.enable(true);
    }
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
