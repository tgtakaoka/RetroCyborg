#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "ins8060_sci_handler.h"
#include "mc6850.h"
#include "regs.h"
#include "signals.h"
#include "string_util.h"

extern libcli::Cli &cli;

class Pins Pins;
Mc6850 Acia(Console);
Ins8060SciHandler<PIN_SENSEB, PIN_FLAG0> SciH(Console);

static constexpr bool debug_cycles = false;

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
 * - XIN rising edge to XOUT falling edge is 24ns.
 * - XIN falling edge to XOUT rising edge is 40ns.
 * - XIN 1st rising edge to #BREQ falling edge is 72ns.
 * - XIN 3rd falling edge to #RDS falling edge is 84ns.
 * - XIN 7th falling edge to #RDS rising edge is 88ns.
 * - XIN 8th falling edge to #BREQ rising edge is 176ns.
 * - #BREQ falling edge to #RDS/#WDS falling edge is 2 cycles/
 * - #RST rising edge to 1st #BREQ takes ~14 XIN cycles.
 */

#if defined(ARDUINO_TEENSY35)
static constexpr auto xin_hi_ns = 125;
static constexpr auto xin_lo_ns = 115;
#endif
#if defined(ARDUINO_TEENSY41)
static constexpr auto xin_hi_ns = 125;
static constexpr auto xin_lo_ns = 115;
#endif

static inline void xin_hi() __attribute__((always_inline));
static inline void xin_hi() {
    digitalWriteFast(PIN_XIN, HIGH);
}

static inline void xin_lo() __attribute__((always_inline));
static inline void xin_lo() {
    digitalWriteFast(PIN_XIN, LOW);
}

static inline uint8_t signal_breq() __attribute__((always_inline));
static inline uint8_t signal_breq() {
    return digitalReadFast(PIN_BREQ);
}

static inline uint8_t signal_ads() __attribute__((always_inline));
static inline uint8_t signal_ads() {
    return digitalReadFast(PIN_ADS);
}

static inline uint8_t signal_wds() __attribute__((always_inline));
static inline uint8_t signal_wds() {
    return digitalReadFast(PIN_WDS);
}

static inline uint8_t signal_rds() __attribute__((always_inline));
static inline uint8_t signal_rds() {
    return digitalReadFast(PIN_RDS);
}

static void assert_sense_a() {
    digitalWriteFast(PIN_SENSEA, HIGH);
}

static void negate_sense_a() {
    digitalWriteFast(PIN_SENSEA, LOW);
}

static void assert_cont() __attribute__((unused));
static void assert_cont() {
    digitalWriteFast(PIN_CONT, HIGH);
}

static void negate_cont() __attribute__((unused));
static void negate_cont() {
    digitalWriteFast(PIN_CONT, LOW);
}

static void assert_hold() __attribute__((unused));
static void assert_hold() {
    digitalWriteFast(PIN_HOLD, LOW);
}

static void negate_hold() __attribute__((unused));
static void negate_hold() {
    digitalWriteFast(PIN_HOLD, HIGH);
}

static void assert_enin() {
    digitalWriteFast(PIN_ENIN, LOW);
}

static void negate_enin() {
    digitalWriteFast(PIN_ENIN, HIGH);
}

static void assert_reset() {
    // Drive RESET condition
    digitalWriteFast(PIN_RST, LOW);
    negate_sense_a();
    assert_cont();
    negate_hold();
    negate_enin();
}

static void negate_reset() {
    // Release RESET conditions
    digitalWriteFast(PIN_RST, HIGH);
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
        PIN_DB0,
        PIN_DB1,
        PIN_DB2,
        PIN_DB3,
        PIN_DB4,
        PIN_DB5,
        PIN_DB6,
        PIN_DB7,
        PIN_ADL00,
        PIN_ADL01,
        PIN_ADL02,
        PIN_ADL03,
        PIN_ADL04,
        PIN_ADL05,
        PIN_ADL06,
        PIN_ADL07,
#if defined(PIN_ADL08)
        PIN_ADL08,
        PIN_ADL09,
        PIN_ADL10,
        PIN_ADL11,
#endif
#if defined(PIN_ADM08)
        PIN_ADM08,
        PIN_ADM09,
        PIN_ADM10,
        PIN_ADM11,
#endif
        PIN_FLAG0,
        PIN_FLAG1,
        PIN_FLAG2,
};

void Pins::begin() {
    // Set directions.
    for (uint8_t i = 0; i < sizeof(BUS_PINS); i++)
        pinMode(BUS_PINS[i], INPUT_PULLUP);
    pinMode(PIN_XIN, OUTPUT);
    pinMode(PIN_BREQ, INPUT_PULLUP);
    pinMode(PIN_RDS, INPUT_PULLUP);
    pinMode(PIN_WDS, INPUT_PULLUP);
    pinMode(PIN_SENSEA, OUTPUT);
    pinMode(PIN_SENSEB, OUTPUT);
    pinMode(PIN_ADS, INPUT_PULLUP);
    pinMode(PIN_CONT, OUTPUT);
    pinMode(PIN_HOLD, OUTPUT);
    pinMode(PIN_RST, OUTPUT);
    pinMode(PIN_ENIN, OUTPUT);
    pinMode(PIN_ENOUT, INPUT);
    pinMode(PIN_SOUT, INPUT);
    pinMode(PIN_SIN, OUTPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    assert_reset();
    xin_lo();
    _freeRunning = false;
    // reset();

    setDeviceBase(Device::ACIA);
}

void Pins::clock_cycle() const {
    xin_hi();
    delayNanoseconds(xin_hi_ns);
    xin_lo();
    if (_freeRunning)
        SciH.loop();
    delayNanoseconds(xin_lo_ns);
}

Signals &Pins::prepareCycle() {
    Signals &signals = Signals::currCycle();
    while (signal_breq() != LOW) {
        clock_cycle();
    }
    assert_enin();
    // #BREQ=LOW
    while (!signals.getAddr()) {
        clock_cycle();
    }
    return signals;
}

Signals &Pins::completeCycle(Signals &signals) {
    if (signals.write()) {
        while (signal_wds() != LOW) {
            clock_cycle();
        }
        signals.getData();
        if (Acia.isSelected(signals.addr)) {
            Acia.write(signals.debug('a').data, signals.addr);
        } else if (signals.writeRam()) {
            Memory.raw_write(signals.addr, signals.debug('m').data);
        } else {
            ;  // capture data to signals.data
        }
        while (signal_wds() == LOW) {
            clock_cycle();
        }
    } else {
        if (Acia.isSelected(signals.addr)) {
            signals.debug('a').data = Acia.read(signals.addr);
        } else if (signals.readRam()) {
            signals.debug('m').data = Memory.raw_read(signals.addr);
        } else {
            ;  // inject data from signals.data
        }
        while (signal_rds() != LOW) {
            clock_cycle();
        }
        busWrite(DB, signals.data);
        busMode(DB, OUTPUT);
        while (signal_rds() == LOW) {
            clock_cycle();
        }
    }
    Signals::nextCycle();
    busMode(DB, INPUT);
    for (auto i = 0; signal_breq() == LOW && i < 2; i++) {
        clock_cycle();
    }
    if (signal_breq() != LOW) {
        negate_enin();
    }
    return signals;
}

Signals &Pins::cycle() {
    return completeCycle(prepareCycle());
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
        Signals &signals = prepareCycle();
        if (signals.read()) {
            Signals::inject(inst[inj++]);
        } else {
            Signals::capture();
        }
        completeCycle(signals);
        if (signals.write() && buf) {
            if (cap == 0 && addr)
                *addr = signals.addr;
            if (cap < max)
                buf[cap++] = signals.data;
        }
    }
    return cap;
}

void Pins::reset(bool show) {
    assert_reset();
    // #RST must remain low for a minimum of 4 Tc.
    for (auto i = 0; i < 2 * 4; i++)
        clock_cycle();
    negate_enin();
    negate_reset();
    // The EBREQ output goes low, indicating the start of execution;
    // this occurs at a time whithin 13 Tc after #RST is set high.
    for (auto i = 0; i < 2 * 13; i++)
        clock_cycle();
    Regs.save(show);

    Acia.reset();
}

void Pins::idle() {
    // #ENIN is HIGH and bus cycle is suspened.
    clock_cycle();
}

void Pins::loop() {
    if (_freeRunning) {
        Acia.loop();
        const Signals &signals = cycle();
        if (signals.halt())
            Commands.halt(true);
        if (user_sw() == LOW)
            Commands.halt(true);
    } else {
        idle();
    }
}

void Pins::suspend(bool show) {
    if (show)
        Signals::resetCycles();
    while (true) {
        Signals &signals = prepareCycle();
        if (signals.fetchInsn()) {
            rawStep(signals);
            if (show)
                Signals::printCycles();
            Regs.save(debug_cycles);
            return;
        }
        completeCycle(signals);
    }
}

void Pins::halt(bool show) {
    if (_freeRunning) {
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

void Pins::rawStep(Signals &signals) {
    const uint8_t insn = Memory.read(signals.addr);
    const uint8_t cycles = Regs::bus_cycles(insn);
    completeCycle(signals).debug('1');
    for (uint8_t c = 1; c < cycles; c++) {
        cycle().debug(c + '1');
    }
}

void Pins::step(bool show) {
    Regs.restore(debug_cycles);
    Signals::resetCycles();
    rawStep(prepareCycle());
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
        assert_sense_a();
}

void Pins::negateIrq(uint8_t irq) {
    _irq &= ~(1 << irq);
    if (_irq == 0)
        negate_sense_a();
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
    cli.print(F("Bitbang (INS8060) "));
    if (serial == Device::BITBANG) {
        cli.printDec(baseAddr);
        cli.println(F(" bps at FLAG0 and SENSEB"));
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
