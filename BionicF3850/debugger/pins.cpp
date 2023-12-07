#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "i8251.h"
#include "inst.h"
#include "memory.h"
#include "regs.h"
#include "signals.h"
#include "string_util.h"
#include "tlcs90_uart_handler.h"

extern libcli::Cli cli;

class Pins Pins;
I8251 Usart(Console);
Tlcs90UartHandler UartH(Console);

static constexpr bool debug_cycles = false;

// clang-format off
/*
 * TMP90C802 Bus cycle
 *          __    __    __    __    __    __
 *     X1 _|  |__|  |__|  |__|  |__|  |__|  |__|
 *                  \___________\           \___
 *    CLK __________/            \__________/
 *        ___ _______________ _______________ __
 * A0~A15 ___X_______________X_______________X__
 *        _______             __________________
 *   #RD         \___________/
 *       ________________________             __
 *   #WR                         \___________/
 *                        ___         _______
 *   DATA ---------------<___>-------<_______>--
 *        ______________________________________
 *  W#AIT _____________/ \_____________/ \______
 */
// clang-format on

#if defined(ARDUINO_TEENSY41)
static constexpr auto x1_hi_ns = 74;
static constexpr auto x1_lo_ns = 74;
#endif

static inline void x1_hi() __attribute__((always_inline));
static inline void x1_hi() {
    digitalWriteFast(PIN_X1, HIGH);
}

static inline void x1_lo() __attribute__((always_inline));
static inline void x1_lo() {
    digitalWriteFast(PIN_X1, LOW);
}

void Pins::x1_cycle() const {
    x1_lo();
    delayNanoseconds(x1_lo_ns);
    if (_freeRunning)
        UartH.loop();
    x1_hi();
    delayNanoseconds(x1_hi_ns);
}

void Pins::x1_cycle_hi() const {
    x1_lo();
    delayNanoseconds(x1_lo_ns);
    if (_freeRunning)
        UartH.loop();
    x1_hi();
}

static void toggle_debug() __attribute__((unused));
static void toggle_debug() {
    digitalToggleFast(PIN_RXD);
}

static void assert_debug() __attribute__((unused));
static void assert_debug() {
    digitalWriteFast(PIN_RXD, HIGH);
}

static void negate_debug() __attribute__((unused));
static void negate_debug() {
    digitalWriteFast(PIN_RXD, LOW);
}

static inline uint8_t signal_clk() {
    return digitalReadFast(PIN_CLK);
}

static inline uint8_t signal_rd() {
    return digitalReadFast(PIN_RD);
}

static inline uint8_t signal_wr() {
    return digitalReadFast(PIN_WR);
}

static void assert_nmi() __attribute__((unused));
static void assert_nmi() {
    digitalWriteFast(PIN_NMI, LOW);
}

static void negate_nmi() {
    digitalWriteFast(PIN_NMI, HIGH);
}

static void assert_int0() __attribute__((unused));
static void assert_int0() {
    digitalWriteFast(PIN_INT0, HIGH);
}

static void negate_int0() {
    digitalWriteFast(PIN_INT0, LOW);
}

static void assert_int1() __attribute__((unused));
static void assert_int1() {
    digitalWriteFast(PIN_INT1, HIGH);
}

static void negate_int1() {
    digitalWriteFast(PIN_INT1, LOW);
}

static void assert_wait() __attribute__((unused));
static void assert_wait() {
    digitalWriteFast(PIN_WAIT, LOW);
}

static void negate_wait() {
    digitalWriteFast(PIN_WAIT, HIGH);
}

static void assert_reset() {
    negate_wait();
    negate_nmi();
    negate_int0();
    negate_int1();
    digitalWriteFast(PIN_RESET, LOW);
}

static void negate_reset() {
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

void Pins::begin() {
    // Set directions.
    busMode(DB, INPUT_PULLDOWN);
    busMode(ADL, INPUT);
    busMode(ADM, INPUT);
    busMode(ADH, INPUT);
    pinMode(PIN_RD, INPUT);
    pinMode(PIN_WR, INPUT);
    pinMode(PIN_CLK, INPUT);
    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_X1, OUTPUT);
    pinMode(PIN_NMI, OUTPUT);
    pinMode(PIN_INT0, OUTPUT);
    // pinMode(PIN_INT1, OUTPUT);
    pinMode(PIN_WAIT, OUTPUT);
    pinMode(PIN_TXD, INPUT);
    pinMode(PIN_RXD, OUTPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    assert_reset();
    turn_off_led();

    _freeRunning = false;
    x1_hi();
    reset();

    setDeviceBase(Device::USART);

    _rom_begin = 1;
    _rom_end = 0;   // no ROM area
    _breakNum = 0;  // no break point
}

void Pins::reset(bool show) {
    Signals::resetCycles();
    assert_reset();
    // #RESET input must be maintained at the "0" level for at least
    // #10 systemn clock cycles (10 stated; 2usec at 10MHz).
    for (auto i = 0; i < 10 * 2 || signal_clk() == LOW; ++i)
        x1_cycle();
    negate_reset();
    while (true) {
        x1_lo();
        delayNanoseconds(x1_lo_ns - 10);
        if (signal_clk() == LOW)
            break;
        x1_hi();
        delayNanoseconds(x1_hi_ns);
    }

    _writes = 0;
    Regs.reset(show);
    Regs.save(show);

    Usart.reset();
    UartH.reset();
}

Signals &Pins::prepareCycle() {
    if (signal_rd() && signal_wr()) {
        while (signal_clk() == LOW)
            x1_cycle();
        x1_cycle_hi();
    }
    return Signals::currCycle().getAddr();
}

Signals &Pins::completeCycle(Signals &signals) {
    negate_wait();
    if (signals.read()) {
        _writes = 0;
        x1_cycle_hi();
        if (signals.readRam()) {
            signals.debug('m').data = Memory.read(signals.addr);
        } else {
            ;  // inject data from signals.data
        }
        signals.outData();
        Signals::nextCycle();
    } else if (signals.write()) {
        ++_writes;
        signals.getData();
        x1_cycle_hi();
        if (signals.writeRam()) {
            if (signals.addr <= _rom_end && signals.addr >= _rom_begin) {
                // ROM area, ignore write from CPU;
            } else {
                Memory.write(signals.addr, signals.debug('m').data);
            }
        } else {
            ;  // capture data to signals.data
        }
        Signals::nextCycle();
    } else {
        x1_cycle();
    }
    x1_cycle_hi();
    busMode(DB, INPUT_PULLDOWN);
    return signals;
}

void Pins::dummyCycles() {
    while (true) {
        auto &signals = prepareCycle();
        if (signals.read() || signals.write())
            break;
        completeCycle(signals);
    }
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
    negate_wait();
    uint8_t inj = 0;
    uint8_t cap = 0;
    while (inj < len || cap < max) {
        auto &signals = prepareCycle();
        if (cap < max)
            signals.capture();
        if (inj < len)
            signals.inject(inst[inj]);
        completeCycle(signals);
        if (signals.read()) {
            signals.debug('i');  // injected
            if (inj == 0 && addr)
                *addr = signals.addr;
            if (inj < len)
                inj++;
        } else if (signals.write()) {
            signals.debug('c');  // captured
            if (buf && cap < max)
                buf[cap++] = signals.data;
        }
    }
    dummyCycles();
    assert_wait();
    return cap;
}

void Pins::idle() {
    // CPU stops at #WAIT=LOW
    x1_cycle();
    x1_cycle();
    x1_cycle();
    x1_cycle_hi();
}

void Pins::loop() {
    if (_freeRunning) {
        Usart.loop();
        completeCycle(prepareCycle());
        if (_writes == 4) {
            const auto &signals = prepareCycle();
            if (signals.addr == 0x0010) {
                // SWI; break point or hlat to system (A=0)
                const auto &frame = signals.prev(4);
                const auto pc = (frame.data << 8) + frame.next().data;
                const auto a = frame.next(2).data;
                const auto breakPoint = isBreakPoint(pc - 1);
                if (breakPoint || a == 0) {
                    restoreBreakInsns();
                    Signals::disassembleCycles(signals.prev(7));
                    Regs.breakPoint(frame);
                    _freeRunning = false;
                    Commands.halt(false);
                    return;
                }
            }
        } else if (user_sw() == LOW) {
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
    suspend(false, true);  // step over possible break point
    saveBreakInsns();
    Regs.restore(debug_cycles);
    Signals::resetCycles();
    _freeRunning = true;
    turn_on_led();
}

void Pins::halt(bool show) {
    if (_freeRunning) {
        suspend(show, true);
    }
    turn_off_led();
    _freeRunning = false;
}

void Pins::suspend(bool show, bool nmi) {
    negate_wait();
    auto assertNmi = nmi;
    while (_writes < 4) {
        auto &signals = prepareCycle();
        completeCycle(signals);
        // Interrupt; n:d:d:1:d:W:W:W:W:1
        if (assertNmi) {
            assertNmi = false;
            assert_nmi();
            _writes = 0;
        }
    }
    const auto &stop = Signals::currCycle();
    negate_nmi();
    dummyCycles();
    if (show)
        Signals::disassembleCycles(nmi ? stop.prev(6) : stop);
    Regs.interrupt(stop.prev(4), nmi);
}

void Pins::step(bool show) {
    const auto opc = Memory.read(Regs.nextIp());
    Inst inst;
    if (opc == Inst::HALT || !inst.get(Regs.nextIp())) {
        // HALT or unknown instruction. Just return.
        return;
    }
    Regs.restore(debug_cycles);
    Signals::resetCycles();
    suspend(show, opc != Inst::SWI);
}

void Pins::assertIntr(IntrName intr) {
    if (intr == INTR_INT0)
        assert_int0();
    else if (intr == INTR_INT1)
        assert_int1();
}

void Pins::negateIntr(IntrName intr) {
    if (intr == INTR_INT0)
        negate_int0();
    else if (intr == INTR_INT1)
        negate_int1();
}

static const char TEXT_USART[] PROGMEM = "USART";
static const char TEXT_UART[] PROGMEM = "UART";

Pins::Device Pins::parseDevice(const char *name) const {
    if (strcasecmp_P(name, TEXT_USART) == 0)
        return Device::USART;
    if (strcasecmp_P(name, TEXT_UART) == 0)
        return Device::UART;
    return Device::NONE;
}

void Pins::getDeviceName(Pins::Device dev, char *name) const {
    *name = 0;
    if (dev == Device::USART)
        strcpy_P(name, TEXT_USART);
    if (dev == Device::UART)
        strcpy_P(name, TEXT_UART);
}

void Pins::setDeviceBase(Pins::Device dev, bool hasValue, uint16_t base) {
    switch (dev) {
    case Device::USART:
        setSerialDevice(Device::USART, hasValue ? base : USART_BASE_ADDR);
        break;
    case Device::UART:
        setSerialDevice(Device::UART, hasValue ? base : 0xFFEB);
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
    cli.print(F("UART (TLCS90) "));
    if (serial == Device::UART) {
        cli.print(F("at "));
        cli.printDec(baseAddr);
        cli.println(F(" bps"));
    } else {
        cli.println(F("disabled"));
    }
}

Pins::Device Pins::getSerialDevice(uint16_t &baseAddr) const {
    if (_serialDevice == Device::USART) {
        baseAddr = Usart.baseAddr();
    }
    if (_serialDevice == Device::UART) {
        baseAddr = UartH.baudrate();
    }
    return _serialDevice;
}

void Pins::setSerialDevice(Pins::Device device, uint16_t baseAddr) {
    _serialDevice = device;
    Usart.enable(device == Device::USART, baseAddr);
    UartH.enable(device == Device::UART);
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
        Memory.write(_breakPoints[i], Inst::SWI);
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
