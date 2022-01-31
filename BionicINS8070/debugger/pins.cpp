#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "ins8070_sci_handler.h"
#include "mc6850.h"
#include "regs.h"
#include "signals.h"
#include "string_util.h"

extern libcli::Cli &cli;

class Pins Pins;
Mc6850 Acia(Console);
Ins8070SciHandler<PIN_SA, PIN_F1> SciH(Console);

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
 * - XIN edges to XOUT edges are 8ns.
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
static constexpr auto xin_hi_ns = 110;
static constexpr auto xin_lo_ns = 105;
#endif

static inline void xin_hi() __attribute__((always_inline));
static inline void xin_hi() {
    digitalWriteFast(PIN_XIN, HIGH);
}

static inline void xin_lo() __attribute__((always_inline));
static inline void xin_lo() {
    digitalWriteFast(PIN_XIN, LOW);
}

void Pins::clock_cycle() const {
    xin_hi();
    delayNanoseconds(xin_hi_ns);
    xin_lo();
    if (_freeRunning)
        SciH.loop();
    delayNanoseconds(xin_lo_ns);
}

static inline uint8_t signal_breq() __attribute__((always_inline));
static inline uint8_t signal_breq() {
    return digitalReadFast(PIN_BREQ);
}

static inline uint8_t signal_wds() __attribute__((always_inline));
static inline uint8_t signal_wds() {
    return digitalReadFast(PIN_WDS);
}

static inline uint8_t signal_rds() __attribute__((always_inline));
static inline uint8_t signal_rds() {
    return digitalReadFast(PIN_RDS);
}

static void assert_sa() __attribute__((unused));
static void assert_sa() {
    digitalWriteFast(PIN_SA, LOW);
}

static void negate_sa() {
    digitalWriteFast(PIN_SA, HIGH);
}

static void assert_sb() __attribute__((unused));
static void assert_sb() {
    digitalWriteFast(PIN_SB, LOW);
}

static void negate_sb() {
    digitalWriteFast(PIN_SB, HIGH);
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
    negate_sa();
    negate_sb();
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
        PIN_F1,
        PIN_F2,
        PIN_F3,
};

void Pins::begin() {
    // Set directions.
    for (uint8_t i = 0; i < sizeof(BUS_PINS); i++)
        pinMode(BUS_PINS[i], INPUT_PULLUP);
    pinMode(PIN_XIN, OUTPUT);
    pinMode(PIN_BREQ, INPUT_PULLUP);
    pinMode(PIN_RDS, INPUT_PULLUP);
    pinMode(PIN_WDS, INPUT_PULLUP);
    pinMode(PIN_SA, OUTPUT);
    pinMode(PIN_SB, OUTPUT);
    pinMode(PIN_HOLD, OUTPUT);
    pinMode(PIN_RST, OUTPUT);
    pinMode(PIN_ENIN, OUTPUT);
    pinMode(PIN_ENOUT, INPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    assert_reset();
    xin_lo();
    _freeRunning = false;
    reset();

    setDeviceBase(Device::ACIA);
}

Signals &Pins::prepareCycle() {
    Signals &signals = Signals::currCycle();
    while (signal_breq() != LOW) {
        clock_cycle();
    }
    assert_enin();
    // #BREQ=LOW
    while (!signals.getDirection()) {
        clock_cycle();
    }
    signals.getAddr();
    return signals;
}

Signals &Pins::completeCycle(Signals &signals) {
    if (signals.write()) {
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
        busWrite(D, signals.data);
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
    for (auto i = 0; i < len; i++) {
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
    // #RST must remain low is 8 Tc.
    for (auto i = 0; i < 4 * 8 * 10; i++)
        clock_cycle();
    negate_reset();
    negate_enin();
    // The first instruction will be fetched within 13 Tc after #RST
    // has gone high.
    for (auto i = 0; i < 4 * 13; i++)
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
        cycle();
        if (user_sw() == LOW)
            Commands.halt(true);
    } else {
        idle();
    }
}

bool Pins::isInsnFetch(const Signals *c0, const Signals *c1, const Signals *c2,
        const Signals *c3, const Signals *c4) {
    if (c4->addr == c2->addr + 1 && c2->addr == c1->addr + 1) {
        if (c1->read() && c2->read()) {
            // 8 bit operation
            if (debug_cycles)
                cli.println(F("8 bit operation"));
            return true;
        }
    }
    if (c4->addr == c1->addr + 1 && c1->addr == c0->addr + 1) {
        if (c0->read() && c1->read()) {
            if (c3->addr == c2->addr + 1 && c2->read() == c3->read()) {
                // 16 bit operation
                if (debug_cycles)
                    cli.println(F("16 bit operation"));
                return true;
            }
            if (c3->addr == c2->addr && c2->read() && c3->write()) {
                // read-modify-write operation
                if (debug_cycles)
                    cli.println(F("read-modify-write operation"));
                return true;
            }
        }
    }
    if (c2->read() && (c2->data & ~0x1B) == 0x64 && c3->addr == c2->addr + 1 &&
            c3->read()) {
        const auto target = c3->addr + static_cast<int8_t>(c3->data);
        if (c4->addr == target + 1) {
            // branch operation
            if (debug_cycles)
                cli.println(F("branch operation"));
            return true;
        }
    }
    if (c1->read() && c1->data == 0x24 && c2->addr == c1->addr + 1 &&
            c3->addr == c1->addr + 2 && c2->read() && c3->read()) {
        const auto target = (static_cast<uint16_t>(c3->data) << 8) | c2->data;
        if (c4->addr == target + 1) {
            // jump operation
            if (debug_cycles)
                cli.println(F("jump operation"));
            return true;
        }
    }

    return false;
}

void Pins::suspend(bool show) {
    Signals::resetCycles();
    const Signals *c0 = &cycle();
    const Signals *c1 = &cycle();
    const Signals *c2 = &cycle();
    const Signals *c3 = &cycle();
    while (true) {
        Signals &signals = prepareCycle();
        if (signals.read() && !Acia.isSelected(signals.addr) &&
                isInsnFetch(c0, c1, c2, c3, &signals)) {
            Regs.save();
            break;
        }
        completeCycle(signals);
        c0 = c1;
        c1 = c2;
        c2 = c3;
        c3 = &signals;
    }
    if (show)
        Signals::printCycles(c3 + 1);
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

void Pins::step(bool show) {
    const auto insn = Memory.raw_read(Regs.nextIp());
    auto bus_cycles = Regs::busCycles(insn);
    if (bus_cycles == 0)
        return;
    auto ea = Regs.effectiveAddr(insn, Regs.nextIp());
    if (Memory::is_internal(ea))
        bus_cycles = Regs::insnLen(insn);
    Regs.restore(debug_cycles);
    Signals::resetCycles();
    if ((insn & ~1) == 0x2E) {
        // SSM instruction
        const Signals &ssm = cycle().debug('1');
        if (show || debug_cycles)
            ssm.print();
        for (auto n = 0; n < 256; n++) {
            Signals &signals = prepareCycle();
            if (signals.addr != ea)
                break;
            completeCycle(signals).debug('s');
            if (show || debug_cycles)
                signals.print();
            ea++;
        }
    } else {
        for (auto c = 0; c < bus_cycles; c++) {
            cycle().debug(c < 9 ? '1' + c : 'a' + c - 9);
        }
        if (show || debug_cycles)
            Signals::printCycles();
    }
    Regs.save(debug_cycles);
}

uint8_t Pins::allocateIrq() {
    static uint8_t irq = 0;
    return irq++;
}

void Pins::assertIrq(uint8_t irq) {
    _irq |= (1 << irq);
    if (_irq) {
        // #INTA is negative-edge sensed.
        assert_sa();
        delayMicroseconds(1);
        negate_sa();
    }
}

void Pins::negateIrq(uint8_t irq) {
    _irq &= ~(1 << irq);
    if (_irq == 0)
        negate_sa();
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
    cli.print(F("Bitbang (INS8070) "));
    if (serial == Device::BITBANG) {
        cli.printDec(baseAddr);
        cli.println(F(" bps at F1 and SA"));
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
