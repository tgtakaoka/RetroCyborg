#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "i8251.h"
#include "memory.h"
#include "regs.h"
#include "signals.h"
#include "string_util.h"

#define DEBUG_BUS(e)
//#define DEBUG_BUS(e) e
#define DEBUG_STEP(e)
//#define DEBUG_STEP(e) e
#define DEBUG_EXEC(e)
//#define DEBUG_EXEC(e) e

extern libcli::Cli cli;

class Pins Pins;
I8251 Usart(Console);

static constexpr bool debug_cycles = false;

// clang-format off
/*
 * F3850 bus cycle
 *        _    __    __    __    __    __    __    __    __    __    __    __    __ 
 *   XTLY  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \
 *        _|    __    __    __    __    __    __    __    __    __    __    __    __
 *    PHI   \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  \__/  
 *           \_______\               \_______\                           \_______\  
 *  WRITE ___/        \______________/        \__________________________/        \_
 *        _______________ _______________________ __________________________________
 *   ROMC _______________X_______________________X__________________________________
 *        _______________________ _______
 *  DB RD _______________________X
 *        _________ _______________________ ___________________________________ ____
 *     DB _________X_|_____________________X_|_________________________________X_|__
 */
// clang-format on

#if defined(ARDUINO_TEENSY41)
namespace {
constexpr auto xtly_lo_ns = 224;
constexpr auto xtly_hi_ns = 224;
constexpr auto xtly_read_romc = 170;
constexpr auto xtly_nowrite_romc = 84;
constexpr auto xtly_idle_ns = 25;
}  // namespace
#endif

static inline void xtly_hi() __attribute__((always_inline));
static inline void xtly_hi() {
    digitalWriteFast(PIN_XTLY, HIGH);
}

static inline void xtly_lo() __attribute__((always_inline));
static inline void xtly_lo() {
    digitalWriteFast(PIN_XTLY, LOW);
}

static inline void xtly_cycle() __attribute__((always_inline));
static inline void xtly_cycle() {
    delayNanoseconds(xtly_hi_ns);
    xtly_lo();
    delayNanoseconds(xtly_lo_ns);
    xtly_hi();
}

static inline void xtly_cycle_lo() __attribute__((always_inline));
static inline void xtly_cycle_lo() {
    xtly_lo();
    delayNanoseconds(xtly_lo_ns);
    xtly_hi();
}

static void assert_debug() __attribute__((unused));
static void assert_debug() {
    digitalWriteFast(PIN_IO00, LOW);
}

static void negate_debug() __attribute__((unused));
static void negate_debug() {
    digitalWriteFast(PIN_IO00, HIGH);
}

static void toggle_debug() __attribute__((unused));
static void toggle_debug() {
    digitalToggleFast(PIN_IO00);
}

static inline uint8_t signal_phi() {
    return digitalReadFast(PIN_PHI);
}

static inline uint8_t signal_write() {
    return digitalReadFast(PIN_WRITE);
}

static inline uint8_t signal_icb() __attribute__((unused));
static inline uint8_t signal_icb() {
    return digitalReadFast(PIN_ICB);
}

static void assert_intreq() __attribute__((unused));
static void assert_intreq() {
    digitalWriteFast(PIN_INTREQ, LOW);
}

static void negate_intreq() {
    digitalWriteFast(PIN_INTREQ, HIGH);
}

static void assert_extres() {
    negate_intreq();
    digitalWriteFast(PIN_EXTRES, LOW);
}

static void negate_extres() {
    digitalWriteFast(PIN_EXTRES, HIGH);
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
    busMode(DB, INPUT_PULLDOWN);
    busMode(ROMC, INPUT);
    busMode(IO0, INPUT);
    busMode(IO1L, OUTPUT);
    busMode(IO1H, OUTPUT);
    pinMode(PIN_WRITE, INPUT);
    pinMode(PIN_PHI, INPUT);
    pinMode(PIN_EXTRES, OUTPUT);
    pinMode(PIN_XTLY, OUTPUT);
    pinMode(PIN_INTREQ, OUTPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    assert_extres();
    turn_off_led();

    negate_debug();
    pinMode(PIN_IO00, OUTPUT);

    _freeRunning = false;
    xtly_lo();
    reset();

    setDeviceBase(Device::USART);

    _rom_begin = 1;
    _rom_end = 0;   // no ROM area
    _breakNum = 0;  // no break point
}

void Pins::reset(bool show) {
    Signals::resetCycles();
    assert_extres();
    for (auto i = 0; i < 10 * 4; ++i)
        xtly_cycle();
    while (signal_write() == LOW)
        xtly_cycle();
    // WRITE=H
    negate_extres();
    delayNanoseconds(xtly_hi_ns);

    cycle();  // IDLE
    delayNanoseconds(xtly_idle_ns);
    cycle();  // RESET PC0

    Regs.save(show);

    Usart.reset();
}

Signals &Pins::cycle() {
    auto &signals = Signals::currCycle();
    xtly_cycle_lo();
    while (signal_write() != LOW)
        xtly_cycle();
    // WRITE=L
    DEBUG_BUS(toggle_debug());
    busMode(DB, INPUT_PULLDOWN);
    DEBUG_BUS(toggle_debug());
    xtly_cycle();
    xtly_cycle();
    const auto read = Regs.romc_read(signals.getRomc());
    if (read) {
        DEBUG_BUS(toggle_debug());
        signals.outData();
        DEBUG_BUS(toggle_debug());
    }
    delayNanoseconds(xtly_read_romc);
    xtly_cycle_lo();
    while (signal_write() == LOW)
        xtly_cycle();
    // WRITE=H
    if (read) {
        delayNanoseconds(xtly_nowrite_romc);
    } else {
        DEBUG_BUS(toggle_debug());
        Regs.romc_write(signals.getData());
        DEBUG_BUS(toggle_debug());
    }
    Signals::nextCycle();
    return signals;
}

void Pins::execInst(const uint8_t *inst, uint8_t len) {
    execute(inst, len, nullptr, 0);
}

uint8_t Pins::captureWrites(
        const uint8_t *inst, uint8_t len, uint8_t *buf, uint8_t max) {
    return execute(inst, len, buf, max);
}

uint8_t Pins::execute(
        const uint8_t *inst, uint8_t len, uint8_t *buf, uint8_t max) {
    uint8_t inj = 0;
    uint8_t cap = 0;
    DEBUG_EXEC(assert_debug());
    while (inj < len || cap < max) {
        auto &signals = Signals::currCycle();
        if (cap < max)
            signals.capture();
        if (inj < len)
            signals.inject(inst[inj]);
        cycle();
        if (signals.read()) {
            signals.debug('i');  // injected
            if (inj < len)
                inj++;
        } else if (signals.write()) {
            signals.debug('c');  // captured
            if (buf && cap < max)
                buf[cap++] = signals.data;
        }
    }
    DEBUG_EXEC(negate_debug());
    return cap;
}

void Pins::idle() {
    // Inject "BR $"
    Signals::currCycle().inject(Regs::BR);
    cycle();  // ROMC=0x00
    delayNanoseconds(xtly_idle_ns);
    Signals::currCycle().inject(0xFF);
    cycle();  // ROMC=0x1C
    delayNanoseconds(xtly_idle_ns);
    Signals::currCycle().inject(0xFF);
    cycle();  // ROMC=0x01
}

void Pins::loop() {
    if (_freeRunning) {
        Usart.loop();
        if (!rawStep() || user_sw() == LOW) {
            restoreBreakInsns();
            Commands.halt(true);
            return;
        }
    } else {
        idle();
    }
}

void Pins::run() {
    Regs.restore(debug_cycles);
    Signals::resetCycles();
    rawStep();  // step over possible break point
    saveBreakInsns();
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
    const auto opc = Memory.read(Regs.nextIp());
    const auto len = Regs::insnLen(opc);
    if (len == 0) {
        // Unknown instruction. Just return.
        return false;
    }
    DEBUG_STEP(toggle_debug());
    const auto busCycles = Regs::busCycles(opc) + len;
    for (auto i = 0; i < busCycles; ++i)
        cycle();
    DEBUG_STEP(toggle_debug());
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

void Pins::assertIntreq() {
    assert_intreq();
}

void Pins::negateIntreq() {
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
        cli.print(F("at "));
        cli.printHex(baseAddr, 4);
        cli.println('H');
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

bool Pins::setBreakPoint(uint16_t addr) {
    uint8_t i = 0;
    while (i < _breakNum) {
        if (_breakPoints[i] == addr)
            return true;
        ++i;
    }
    if (i < sizeof(_breakInsns)) {
        _breakPoints[i] = addr;
        ++_breakNum;
        return true;
    }
    return false;
}

bool Pins::clearBreakPoint(uint8_t index) {
    if (--index >= _breakNum)
        return false;
    for (uint8_t i = index + 1; i < _breakNum; ++i) {
        _breakPoints[index] = _breakPoints[i];
        ++index;
    }
    --_breakNum;
    return true;
}

void Pins::printBreakPoints() const {
    for (uint8_t i = 0; i < _breakNum; ++i) {
        cli.printDec(i + 1, -2);
        Memory.disassemble(_breakPoints[i], 1);
    }
}

void Pins::saveBreakInsns() {
    for (uint8_t i = 0; i < _breakNum; ++i) {
        _breakInsns[i] = Memory.read(_breakPoints[i]);
        Memory.write(_breakPoints[i], Regs::UNKN);
    }
}

void Pins::restoreBreakInsns() {
    for (uint8_t i = 0; i < _breakNum; ++i) {
        Memory.write(_breakPoints[i], _breakInsns[i]);
    }
}

bool Pins::isBreakPoint(uint16_t addr) const {
    for (uint8_t i = 0; i < _breakNum; ++i) {
        if (_breakPoints[i] == addr)
            return true;
    }
    return false;
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
