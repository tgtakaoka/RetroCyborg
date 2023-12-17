#include "pins.h"

#include <libcli.h>

#include "cdp1802_sci_handler.h"
#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "mc6850.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli cli;
class Pins Pins;

Mc6850 Acia(Console);
Cdp1802SciHandler<PIN_EF3, PIN_Q> SciH(Console);

static constexpr bool debug_cycles = false;

/**
 * CDP1802 bus cycle.
 *      __    __    __    __    __    __    __    __    __    __
 * OSC1 c7|__|c0|__|c1|__|c2|__|c3|__|c4|__|c5|__|c6|__|c7|__|c0|__
 *              |>> _____
 *  TPA ___________|  |>>|_________________________________________
 *                                               |>>______
 *  TPB ___________________________________________|   |>>|________
 *             ____________  _____________________________
 *   MA -------<HHHHHHHHHHHH><LLLLLLLLLLLLLLLLLLLLLLLLLLLLL>-------
 *      _____________                                      ________
 * #MRD            |>|____________________________________|
 *      __________________________________              ___________
 * #MWR                                 |>|____________|
 *       _ |_______________________________________________  _______
 *   SC  _><_______________________________________________><_______
 *
 *  BUS --------------WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW---------
 *                                                     v
 *      -------------------------------------------RRRRRRR---------
 *
 *  - c7 falling edge to SC valid is ~130ns (300~450ns).
 *  - c0 falling edge to TPA rising edge is ~100ns (200~300ns)
 *  - c1 rising edge to #MRD falling edge is ~
 *  - c1 falling edge to TPA falling edge is ~100ns (200~300ns)
 *  - BUS data is sampled at rising edge of c7 with hodling time 200ns.
 */

#if defined(ARDUINO_TEENSY41)
static constexpr auto clock_hi_ns = 120;
static constexpr auto clock_lo_ns = 120;
static constexpr auto c0_raw_lo_ns = 50;
static constexpr auto c0_lo_ns = 0;
static constexpr auto c0_hi_ns = 120;
static constexpr auto c1_lo_ns = 120;
static constexpr auto c1_hi_ns = 120;
static constexpr auto c2_lo_ns = 82;
static constexpr auto c2_hi_ns = 120;
static constexpr auto c3_lo_ns = 120;
static constexpr auto c3_hi_ns = 120;
static constexpr auto c4_lo_ns = 82;
static constexpr auto c4_hi_ns = 120;
static constexpr auto c5_lo_ns = 120;
static constexpr auto c5_hi_ns = 104;
static constexpr auto c6_read_lo_ns = 46;
static constexpr auto c6_read_hi_ns = 75;
static constexpr auto c7_read_lo_ns = 112;
static constexpr auto c7_read_hi_ns = 72;
static constexpr auto c6_write_lo_ns = 52;
static constexpr auto c6_write_hi_ns = 100;
static constexpr auto c7_write_lo_ns = 78;
static constexpr auto c7_write_hi_ns = 72;
static constexpr auto c6_nobus_lo_ns = 48;
static constexpr auto c6_nobus_hi_ns = 96;
static constexpr auto c7_nobus_lo_ns = 120;
static constexpr auto c7_nobus_hi_ns = 64;
#endif

static inline void clock_hi() __attribute__((always_inline));
static inline void clock_hi() {
    digitalWriteFast(PIN_CLOCK, HIGH);
}

static inline void clock_lo() __attribute__((always_inline));
static inline void clock_lo() {
    digitalWriteFast(PIN_CLOCK, LOW);
}

static inline void clock_cycle() __attribute__((always_inline));
static inline void clock_cycle() {
    clock_hi();
    delayNanoseconds(clock_hi_ns);
    clock_lo();
    delayNanoseconds(clock_lo_ns);
}

static void assert_intr() {
    digitalWriteFast(PIN_INTR, LOW);
}

static void negate_intr() {
    digitalWriteFast(PIN_INTR, HIGH);
}

static inline uint8_t signal_mrd() __attribute__((always_inline));
static inline uint8_t signal_mrd() {
    return digitalReadFast(PIN_MRD);
}

static void assert_wait() __attribute__((unused));
static void assert_wait() {
    digitalWriteFast(PIN_WAIT, LOW);
}

static void negate_wait() {
    digitalWriteFast(PIN_WAIT, HIGH);
}

static void assert_reset() {
    // Drive RESET condition
    digitalWriteFast(PIN_CLEAR, LOW);
    negate_wait();
    negate_intr();
    digitalWriteFast(PIN_DMAIN, HIGH);
    digitalWriteFast(PIN_DMAOUT, HIGH);
    clock_lo();
}

static void negate_reset() {
    digitalWriteFast(PIN_CLEAR, HIGH);
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
        PIN_DBUS0,
        PIN_DBUS1,
        PIN_DBUS2,
        PIN_DBUS3,
        PIN_DBUS4,
        PIN_DBUS5,
        PIN_DBUS6,
        PIN_DBUS7,
        PIN_MA0,
        PIN_MA1,
        PIN_MA2,
        PIN_MA3,
        PIN_MA4,
        PIN_MA5,
        PIN_MA6,
        PIN_MA7,
        PIN_N0,
        PIN_N1,
        PIN_N2,
        PIN_EF1,
        PIN_EF2,
        PIN_EF3,
        PIN_EF4,
};

void Pins::begin() {
    // Set directions.
    for (uint8_t i = 0; i < sizeof(BUS_PINS); i++)
        pinMode(BUS_PINS[i], INPUT);
    pinMode(PIN_Q, INPUT);
    pinMode(PIN_TPA, INPUT);
    pinMode(PIN_TPB, INPUT);
    pinMode(PIN_INTR, OUTPUT);
    pinMode(PIN_SC0, INPUT);
    pinMode(PIN_SC1, INPUT);
    pinMode(PIN_CLOCK, OUTPUT);
    pinMode(PIN_MRD, INPUT);
    pinMode(PIN_MWR, INPUT);
    pinMode(PIN_WAIT, OUTPUT);
    pinMode(PIN_CLEAR, OUTPUT);
    pinMode(PIN_DMAIN, OUTPUT);
    pinMode(PIN_DMAOUT, OUTPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    busMode(EF, OUTPUT);
    busWrite(EF, 0xF);
    turn_off_led();

    assert_reset();
    _freeRunning = false;

    setDeviceBase(Device::ACIA);
}

Signals &Pins::prepareCycle() {
    // CDP1802 bus cycle is CLOCK/8, so we toggle CLOCK 8 times
    Signals &signals = Signals::currCycle();
    delayNanoseconds(c0_raw_lo_ns);
    busMode(DBUS, INPUT);
    signals.getStatus();
    return signals;
}

Signals &Pins::rawPrepareCycle() {
    delayNanoseconds(c0_raw_lo_ns);
    return prepareCycle();
}

Signals &Pins::directCycle(Signals &signals) {
    // c0
    clock_hi();
    delayNanoseconds(c0_hi_ns);
    clock_lo();
    // c1
    delayNanoseconds(c1_lo_ns);
    clock_hi();
    delayNanoseconds(c1_hi_ns);
    clock_lo();
    // c2
    delayNanoseconds(c2_lo_ns);
    signals.getAddr1();
    clock_hi();
    delayNanoseconds(c2_hi_ns);
    clock_lo();
    // c3
    delayNanoseconds(c3_lo_ns);
    clock_hi();
    delayNanoseconds(c3_hi_ns);
    clock_lo();
    // c4
    delayNanoseconds(c4_lo_ns);
    signals.getAddr2();
    clock_hi();
    delayNanoseconds(c4_hi_ns);
    clock_lo();
    // c5
    delayNanoseconds(c5_lo_ns);
    clock_hi();
    delayNanoseconds(c5_hi_ns);
    signals.getDirection();
    clock_lo();

    return signals;
}

Signals &Pins::completeCycle(Signals &signals) {
    if (signals.mwr == LOW) {
        // c6
        delayNanoseconds(c6_write_lo_ns);
        clock_hi();
        signals.getData();
        delayNanoseconds(c6_write_hi_ns);
        clock_lo();
        // c7
        if (Acia.isSelected(signals.addr)) {
            Acia.write(signals.debug('a').data, signals.addr);
        } else if (signals.writeRam()) {
            Memory.raw_write(signals.addr, signals.debug('m').data);
        } else {
            signals.debug('c');  // capture data to signals.data
        }
        delayNanoseconds(c7_write_lo_ns);
        clock_hi();
        Signals::nextCycle();
        delayNanoseconds(c7_write_hi_ns);
        clock_lo();
        return signals;
    }
    if (signals.mrd == LOW) {
        // c6
        if (Acia.isSelected(signals.addr)) {
            signals.debug('a').data = Acia.read(signals.addr);
        } else if (signals.readRam()) {
            signals.debug('m').data = Memory.raw_read(signals.addr);
        } else {
            signals.debug('i');  // inject data from signals.data
        }
        delayNanoseconds(c6_read_lo_ns);
        clock_hi();
        busWrite(DBUS, signals.data);
        busMode(DBUS, OUTPUT);
        delayNanoseconds(c6_read_hi_ns);
        clock_lo();
        // c7
        delayNanoseconds(c7_read_lo_ns);
        clock_hi();
        // DBUS is sampled
        Signals::nextCycle();
        delayNanoseconds(c7_read_hi_ns);
        clock_lo();
        // DBUS will goes INPUT at prepareCycle() to fullfill hold time.
        return signals;
    }
    // c6
    delayNanoseconds(c6_nobus_lo_ns);
    clock_hi();
    signals.getData();
    signals.debug(' ');
    delayNanoseconds(c6_nobus_hi_ns);
    clock_lo();
    // c7
    delayNanoseconds(c7_nobus_lo_ns);
    clock_hi();
    Signals::nextCycle();
    delayNanoseconds(c7_nobus_hi_ns);
    clock_lo();
    return signals;
}

Signals &Pins::cycle() {
    return completeCycle(directCycle(prepareCycle()));
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
    uint8_t inj = 0;
    uint8_t cap = 0;
    while (inj < len || cap < max) {
        Signals &signals = rawPrepareCycle();
        directCycle(signals);
        if (signals.mwr == LOW)
            Signals::capture();
        if (signals.mrd == LOW)
            Signals::inject(inst[inj++]);
        completeCycle(signals);
        if (signals.mwr == LOW && buf) {
            if (cap == 0 && addr)
                *addr = signals.addr;
            if (cap < max)
                buf[cap++] = signals.data;
        }
    }
    while (true) {
        Signals &signals = rawPrepareCycle();
        if (signals.fetchInsn())
            break;
        completeCycle(directCycle(signals));
    }
    return cap;
}

void Pins::reset(bool show) {
    assert_reset();
    for (auto i = 0; i < 100; i++)
        clock_cycle();
    negate_reset();
    delayNanoseconds(clock_lo_ns);
    // The first machine cycle after termination of reset is an
    // intialization cycle which requires 9 clock pulses.
    for (auto i = 0; i < 9; i++) {
        clock_cycle();
        const Signals &signals = rawPrepareCycle();
        if (signals.fetchInsn())
            break;
    }
    Regs.reset();
    Regs.save(show);

    Acia.reset();
}

void Pins::idle() {
    // CDP1802 is fully static.
}

void Pins::loop() {
    if (_freeRunning) {
        Acia.loop();
        SciH.loop();
        Signals &signals = directCycle(prepareCycle());
        if (signals.fetchInsn() && Memory.raw_read(signals.addr) == 0x00) {
            // Detect IDL, inject LBR $ instead and halt.
            if (debug_cycles)
                cli.println(F("@@ break"));
            const auto addr = signals.addr;
            Signals::inject(0xC0);  // LBR $
            completeCycle(signals).debug('B');
            Signals::inject(addr >> 8);
            cycle().debug('B');
            Signals::inject(addr).debug('B');
            Commands.halt(true);
        } else {
            completeCycle(signals);
        }
        if (user_sw() == LOW) {
            Commands.halt(true);
            Signals::resetCycles();
        }
    } else {
        idle();
    }
}

void Pins::suspend() {
    // At first searching non-instruction fetch cycle, in order to
    // correctly handle double fetch instructions.
    while (true) {
        Signals &signals = rawPrepareCycle();
        if (!signals.fetchInsn())
            break;
        completeCycle(directCycle(signals));
    }
    while (true) {
        Signals &signals = rawPrepareCycle();
        if (signals.fetchInsn())
            break;
        completeCycle(directCycle(signals));
    }
}

void Pins::halt(bool show) {
    if (_freeRunning) {
        if (debug_cycles)
            cli.println(F("@@ halt"));
        suspend();
        if (show)
            Signals::disassembleCycles();
        Regs.save(debug_cycles);
        turn_off_led();
        _freeRunning = false;
    }
}

void Pins::run() {
    Regs.restore(debug_cycles);
    // Reset cycles for dump valid bus cycles at IDL.
    Signals::resetCycles();
    _freeRunning = true;
    turn_on_led();
}

void Pins::step(bool show) {
    const auto insn = Memory.raw_read(Regs.nextIp());
    if (insn == 0x00)  // IDL
        return;
    Regs.restore(debug_cycles);
    if (show)
        Signals::resetCycles();
    if (debug_cycles)
        cli.println(F("@@ step"));
    char c = '1';
    Signals &s = cycle().debug(c++);
    // See if it was CDP1804/CDP1804A's double fetch instruction.
    if (s.insnFetch() && s.data == 0x68) {
        cycle().debug(c++);
    }
    while (true) {
        Signals &signals = rawPrepareCycle();
        if (signals.insnFetch())
            break;
        completeCycle(directCycle(signals)).debug(c++);
    }
    if (show)
        Signals::printCycles();
    Regs.save(debug_cycles);
}

bool Pins::skip(uint8_t insn) {
    Signals &org = prepareCycle();
    Signals::inject(insn);
    completeCycle(directCycle(org));
    const auto skipi = org.addr;
    while (true) {
        Signals &signals = rawPrepareCycle();
        if (signals.insnFetch()) {
            Signals::inject(0xC4);  // NOP
            completeCycle(directCycle(signals));
            const auto nexti = signals.addr;
            while (true) {
                Signals &s = rawPrepareCycle();
                if (s.insnFetch())
                    return nexti == skipi + 3;
                completeCycle(directCycle(Signals::currCycle()));
            }
        }
        completeCycle(directCycle(signals));
    }
}

uint8_t Pins::allocateIrq() {
    static uint8_t irq = 0;
    return irq++;
}

void Pins::assertIrq(uint8_t irq) {
    _intr |= (1 << irq);
    if (_intr)
        assert_intr();
}

void Pins::negateIrq(uint8_t irq) {
    _intr &= ~(1 << irq);
    if (_intr == 0)
        negate_intr();
}

static const char TEXT_ACIA[] PROGMEM = "ACIA";
static const char TEXT_BITBANG[] PROGMEM = "BITBANG";

Pins::Device Pins::parseDevice(const char *name) const {
    if (strcasecmp_P(name, TEXT_ACIA) == 0)
        return Device::ACIA;
    if (strcasecmp_P(name, TEXT_BITBANG) == 0)
        return Device::BITBANG;
    return Device::NONE;
}

void Pins::getDeviceName(Pins::Device dev, char *name) const {
    *name = 0;
    if (dev == Device::ACIA)
        strcpy_P(name, TEXT_ACIA);
    if (dev == Device::BITBANG)
        strcpy_P(name, TEXT_BITBANG);
}

void Pins::setDeviceBase(Pins::Device dev, bool hasValue, uint16_t base) {
    switch (dev) {
    case Device::ACIA:
        setSerialDevice(Device::ACIA, hasValue ? base : ACIA_BASE_ADDR);
        break;
    case Device::BITBANG:
        setSerialDevice(Device::BITBANG, hasValue ? base : 0);
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
    cli.print(F("Bitbang (CDP1802) "));
    if (serial == Device::BITBANG) {
        cli.printDec(baseAddr);
        cli.println(F(" bps at Q and #EF3"));
    } else {
        cli.println(F("disabled"));
    }
}

Pins::Device Pins::getSerialDevice(uint16_t &baseAddr) const {
    if (_serialDevice == Device::ACIA) {
        baseAddr = Acia.baseAddr();
    }
    if (_serialDevice == Device::BITBANG) {
        baseAddr = SciH.baudrate();
    }
    return _serialDevice;
}

void Pins::setSerialDevice(Pins::Device device, uint16_t baseAddr) {
    _serialDevice = device;
    if (device == Device::ACIA) {
        Acia.enable(true, baseAddr);
        SciH.enable(false, 0);
    }
    if (device == Device::BITBANG) {
        Acia.enable(false, 0);
        SciH.enable(true, baseAddr);
    }
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
