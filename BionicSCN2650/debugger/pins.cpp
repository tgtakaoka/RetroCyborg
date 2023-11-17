#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "i8251.h"
#include "regs.h"
#include "signals.h"
#include "string_util.h"

extern libcli::Cli cli;

class Pins Pins;
I8251 Usart(Console);

static constexpr bool debug_cycles = false;

// clang-format off
/*
 * SCN25650 Bus cycle
 *            _____       _____       _____       _____       _____       _____       __
 *  CLOCK ___|    T|0____|    T|1____|    T|2____|    T|0____|    T|1____|    T|2____|
 *                        \__________________\                 \_________________\
 *  OPREQ ________________|                  |_________________|                 |_______
 *        _______________ /__________________\__ _____________ /__________________\_ ____
 * A0~A14 _______________X______________________X_____________X_____________________X____
 *  M/#IO _______________                     |  _________________________________|_
 *   #R/W                \____________________|_/                                 | \____
 *        ______________________________ _____ ___________________________________ ______
 *   DATA ______________________________X__in_X________________________out________X______
 *        ________________________    ________________________________    _______________
 * #OPACK                         \__/                                \__/
 *        ____________________________________________________       _______       ______
 *   #WRP                                                     \_____/       \_____/
 */
// clang-format on

#if defined(ARDUINO_TEENSY41)
static constexpr auto clock_hi_ns = 380;
static constexpr auto clock_lo_ns = 380;
static constexpr auto clock_hi_t0p = 250;
static constexpr auto clock_hi_t0c = 340;
static constexpr auto clock_hi_t1 = 100;
static constexpr auto clock_hi_t2w = 360;
static constexpr auto clock_hi_idle = 120;
static constexpr auto clock_lo_t0 = 390;
static constexpr auto clock_lo_t1w = 350;
static constexpr auto clock_lo_t1r = 320;
#endif

static inline void clock_hi() __attribute__((always_inline));
static inline void clock_hi() {
    digitalWriteFast(PIN_CLOCK, HIGH);
}

static inline void clock_lo() __attribute__((always_inline));
static inline void clock_lo() {
    digitalWriteFast(PIN_CLOCK, LOW);
}

void clock_cycle() {
    clock_lo();
    delayNanoseconds(clock_lo_ns);
    clock_hi();
    delayNanoseconds(clock_hi_ns);
}

static void toggle_debug() __attribute__((unused));
static void toggle_debug() {
    digitalToggleFast(PIN_SENSE);
}

static void assert_debug() __attribute__((unused));
static void assert_debug() {
    digitalWriteFast(PIN_SENSE, HIGH);
}

static void negate_debug() __attribute__((unused));
static void negate_debug() {
    digitalWriteFast(PIN_SENSE, LOW);
}

static inline uint8_t signal_clock() {
    return digitalReadFast(PIN_CLOCK);
}

static inline uint8_t signal_opreq() {
    return digitalReadFast(PIN_OPREQ);
}

static inline uint8_t signal_intack() {
    return digitalReadFast(PIN_INTACK);
}

static void assert_intreq() {
    digitalWriteFast(PIN_INTREQ, LOW);
}

static void negate_intreq() {
    digitalWriteFast(PIN_INTREQ, HIGH);
}

static void assert_adren() {
    digitalWriteFast(PIN_ADREN, LOW);
}

static void assert_dbusen() {
    digitalWriteFast(PIN_DBUSEN, LOW);
}

static void assert_reset() {
    digitalWriteFast(PIN_RESET, HIGH);
    negate_intreq();
}

static void negate_reset() {
    digitalWriteFast(PIN_RESET, LOW);
}

static void negate_pause() {
    digitalWriteFast(PIN_PAUSE, HIGH);
}

static void assert_opack() {
    digitalWriteFast(PIN_OPACK, LOW);
}

static void negate_opack() {
    digitalWriteFast(PIN_OPACK, HIGH);
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

void Pins::begin() {
    // Set directions.
    busMode(DBUS, INPUT_PULLDOWN);
    busMode(ADRL, INPUT);
    busMode(ADRM, INPUT);
    busMode(ADRH, INPUT);
    pinMode(PIN_OPREQ, INPUT);
    pinMode(PIN_MIO, INPUT);
    pinMode(PIN_RW, INPUT);
    pinMode(PIN_WRP, INPUT);
    pinMode(PIN_INTACK, INPUT);
    pinMode(PIN_RUNWAIT, INPUT);
    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_CLOCK, OUTPUT);
    pinMode(PIN_PAUSE, OUTPUT);
    pinMode(PIN_OPACK, OUTPUT);
    pinMode(PIN_INTREQ, OUTPUT);
    pinMode(PIN_ADREN, OUTPUT);
    pinMode(PIN_DBUSEN, OUTPUT);
    pinMode(PIN_FLAG, INPUT);
    pinMode(PIN_SENSE, OUTPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    assert_reset();
    assert_adren();
    assert_dbusen();
    negate_opack();
    turn_off_led();

    _freeRunning = false;
    clock_hi();
    reset();

    setDeviceBase(Device::USART);

    _rom_begin = 1;
    _rom_end = 0;  // no ROM area
}

void Pins::reset(bool show) {
    negate_pause();
    assert_reset();
    // RESET must remain high for at least 3 cycles
    for (auto i = 0; i < 5; ++i)
        clock_cycle();
    negate_reset();

    Regs.save(show);

    Usart.reset();
}

Signals &Pins::prepareCycle() {
    // T0H
    goto entry;
    do {
        delayNanoseconds(clock_hi_t0p);
    entry:
        clock_lo();  // T0L
        delayNanoseconds(clock_lo_t0);
        clock_hi();  // T1H
        delayNanoseconds(100);
    } while (signal_opreq() == LOW);
    // T1H, OPREQ=H
    return Signals::currCycle().getAddr();
}

Signals &Pins::completeCycle(Signals &signals) {
    // T1H, OPREQ=H
    delayNanoseconds(clock_hi_t1);
    clock_lo();  // T1L
    if (signals.write()) {
        signals.getData();
        assert_opack();
        delayNanoseconds(clock_lo_t1w);
        clock_hi();  // T2H
        if (signals.ioAccess()) {
            if (signals.addr & 0x2000) {
                signals.addr &= 0xFF;
                if (Usart.isSelected(signals.addr)) {
                    Usart.write(signals.debug('u').data, signals.addr);
                } else {
                    signals.debug('E');
                }
            } else {
                signals.debug((signals.addr & 0x4000) ? 'D' : 'C');
            }
        } else if (signals.writeRam()) {
            if (signals.addr <= _rom_end && signals.addr >= _rom_begin) {
                // ROM area, ignore write from CPU;
            } else {
                Memory.write(signals.addr, signals.debug('m').data);
            }
        } else {
            ;  // capture data to signals.data
        }
        delayNanoseconds(clock_hi_t2w);
    } else {
        if (signals.ioAccess()) {
            if (signal_intack() != LOW) {
                signals.debug('V').data = Usart.intrVec();
            } else if (signals.addr & 0x2000) {
                signals.addr &= 0xFF;
                if (Usart.isSelected(signals.addr)) {
                    signals.debug('u').data = Usart.read(signals.addr);
                } else {
                    signals.debug('E');
                }
            } else {
                signals.debug((signals.addr & 0x4000) ? 'D' : 'C');
            }
        } else if (signals.readRam()) {
            signals.debug('m').data = Memory.read(signals.addr);
        } else {
            ;  // inject data from signals.data
        }
        signals.outData();
        assert_opack();
        delayNanoseconds(clock_lo_t1r);
        clock_hi();  // T2H
        delayNanoseconds(clock_hi_ns);
    }
    negate_opack();
    clock_lo();  // T2L
    Signals::nextCycle();
    busMode(DBUS, INPUT_PULLDOWN);
    delayNanoseconds(clock_lo_ns);
    clock_hi();  // T0H
    delayNanoseconds(clock_hi_t0c);
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
    uint8_t inj = 0;
    uint8_t cap = 0;
    while (inj < len || cap < max) {
        auto &signals = prepareCycle();
        if (cap < max)
            signals.capture();
        if (inj < len)
            signals.inject(inst[inj]);
        completeCycle(signals);
        if (inj == 0 && addr)
            *addr = signals.addr;
        if (signals.read()) {
            signals.debug('i');  // injected
            if (inj < len)
                inj++;
        } else {
            signals.debug('c');  // captured
            if (buf && cap < max)
                buf[cap++] = signals.data;
        }
    }
    return cap;
}

void Pins::idle() {
    // CPU stops at OPREQ=H, #OPACK=H
}

void Pins::loop() {
    if (_freeRunning) {
        Usart.loop();
        if (!rawStep() || user_sw() == LOW) {
            Commands.halt(true);
            return;
        }
    } else {
        clock_lo();
        delayNanoseconds(clock_lo_ns);
        clock_hi();
        delayNanoseconds(clock_hi_idle);
    }
}

void Pins::run() {
    Regs.restore(debug_cycles);
    // Reset cycles for dump valid bus cycles at HALT.
    Signals::resetCycles();
    _freeRunning = true;
    turn_on_led();
}

void Pins::halt(bool show) {
    if (_freeRunning) {
        if (show)
            Signals::disassembleCycles();
        Regs.save(debug_cycles);
        turn_off_led();
        _freeRunning = false;
    }
}

bool Pins::rawStep() {
    delayNanoseconds(clock_hi_ns);
    auto *signals = &prepareCycle();
    const auto insn = Memory.read(signals->addr);
    const auto len = Regs::insnLen(insn);
    if (insn == 0x40 || len == 0) {
        // HALT or unknown instruction. Just return.
        return false;
    }

    auto busCycles = len + Regs::busCycles(insn);
    if (Regs::hasIndirect(insn)) {
        if (Memory.read(signals->addr + 1) & 0x80)
            busCycles += 2;
    }
    signals->markFetch();
    for (auto i = 0; i < busCycles; ++i) {
        completeCycle(*signals);
        signals = &prepareCycle();
    }
    return true;
}

void Pins::step(bool show) {
    Regs.restore(debug_cycles);
    Signals::resetCycles();
    rawStep();
    if (show)
        Signals::printCycles();
    Regs.save(debug_cycles);
}

void Pins::assertIntr() {
    assert_intreq();
}

void Pins::negateIntr() {
    negate_intreq();
}

static const char TEXT_USART[] PROGMEM = "USART";

Pins::Device Pins::parseDevice(const char *name) const {
    if (strcasecmp_P(name, TEXT_USART) == 0)
        return Device::USART;
    return Device::NONE;
}

void Pins::getDeviceName(Pins::Device dev, char *name) const {
    *name = 0;
    if (dev == Device::USART)
        strcpy_P(name, TEXT_USART);
}

void Pins::setDeviceBase(Pins::Device dev, bool hasValue, uint16_t base) {
    switch (dev) {
    case Device::USART:
        setSerialDevice(Device::USART, hasValue ? base : USART_BASE_ADDR);
        break;
    default:
        break;
    }
}

void Pins::printDevices() const {
    cli.println();
    uint16_t baseAddr;
    const auto serial = getSerialDevice(baseAddr);
    cli.print(F("USART (i8251) "));
    if (serial == Device::USART) {
        cli.print(F("at H'"));
        cli.printHex(baseAddr & 0xFF, 2);
        cli.println('\'');
    } else {
        cli.println(F("disabled"));
    }
}

Pins::Device Pins::getSerialDevice(uint16_t &baseAddr) const {
    if (_serialDevice == Device::USART) {
        baseAddr = Usart.baseAddr();
    }
    return _serialDevice;
}

void Pins::setSerialDevice(Pins::Device device, uint16_t baseAddr) {
    _serialDevice = device;
    Usart.enable(device == Device::USART, baseAddr);
}

void Pins::setRomArea(uint16_t begin, uint16_t end) {
    _rom_begin = begin;
    _rom_end = end;
}

void Pins::printRomArea() const {
    cli.print(F("ROM area: "));
    if (_rom_begin <= _rom_end) {
        cli.printHex(_rom_begin, 4);
        cli.print('-');
        cli.printlnHex(_rom_end, 4);
    } else {
        cli.println(F("none"));
    }
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
