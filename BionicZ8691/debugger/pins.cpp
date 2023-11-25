#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "i8251.h"
#include "regs.h"
#include "signals.h"
#include "string_util.h"
#include "z8_sio_handler.h"

extern libcli::Cli cli;

class Pins Pins;
I8251 Usart(Console);
Z8SioHandler SioH(Console);

static constexpr bool debug_cycles = false;
static constexpr bool debug_step = false;

/**
 * Z8 External Bus cycle
 *         __    __    __    __    __    __    __    __    __    __
 * XTAL1 _|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__
 *        \     \     \                       \     \     \     \
 *       __       _________________________________________       ____
 *   #AS   |_____|                                         |_____|
 *       _____________                          ______________________
 *   #DS              |________________________|____|
 *          _______________________________________         __________
 *  R/#W --<_______________________________________>-------<__________
 *          _________                                       __________
 *  ADDR --<_________>-------------------------------------<__________
 *                     ________________________
 *  DATA -------------<________________________>----|-----------------
 *
 */

#if defined(ARDUINO_TEENSY41)
static constexpr auto xtal1_hi_ns = 18;
static constexpr auto xtal1_lo_ns = 18;
#endif

static inline void xtal1_hi() __attribute__((always_inline));
static inline void xtal1_hi() {
    // XTAL1 is negated
    digitalWriteFast(PIN_XTAL1, LOW);
}

static inline void xtal1_lo() __attribute__((always_inline));
static inline void xtal1_lo() {
    // XTAL1 is negated
    digitalWriteFast(PIN_XTAL1, HIGH);
}

void Pins::xtal1_cycle() const {
    xtal1_hi();
    delayNanoseconds(xtal1_hi_ns);
    xtal1_lo();
    delayNanoseconds(xtal1_lo_ns);
    if (_freeRunning)
        SioH.loop();
}

static inline uint8_t signal_as() __attribute__((always_inline));
static inline uint8_t signal_as() {
    return digitalReadFast(PIN_AS);
}

static inline uint8_t signal_ds() __attribute__((always_inline));
static inline uint8_t signal_ds() {
    return digitalReadFast(PIN_DS);
}

static void assert_irq0() {
    digitalWriteFast(PIN_IRQ0, LOW);
}

static void negate_irq0() {
    digitalWriteFast(PIN_IRQ0, HIGH);
}

static void assert_irq1() {
    digitalWriteFast(PIN_IRQ1, LOW);
}

static void negate_irq1() {
    digitalWriteFast(PIN_IRQ1, HIGH);
}

static void assert_irq2() {
    digitalWriteFast(PIN_IRQ2, LOW);
}

static void negate_irq2() {
    digitalWriteFast(PIN_IRQ2, HIGH);
}

static void toggle_debug() __attribute__((unused));
static void toggle_debug() {
    digitalToggleFast(PIN_IRQ2);
}

static void toggle_irq1() __attribute__((unused));
static void toggle_irq1() {
    digitalToggleFast(PIN_IRQ1);
}

static void assert_reset() {
    digitalWriteFast(PIN_RESET, LOW);
    negate_irq0();
    negate_irq1();
    negate_irq2();
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

static const uint8_t BUS_PINS[] = {
        PIN_ADDR0,
        PIN_ADDR1,
        PIN_ADDR2,
        PIN_ADDR3,
        PIN_ADDR4,
        PIN_ADDR5,
        PIN_ADDR6,
        PIN_ADDR7,
        PIN_ADDR8,
        PIN_ADDR9,
        PIN_ADDR10,
        PIN_ADDR11,
        PIN_ADDR12,
        PIN_ADDR13,
        PIN_ADDR14,
        PIN_ADDR15,
};

void Pins::begin() {
    // Set directions.
    for (uint8_t i = 0; i < sizeof(BUS_PINS); i++)
        pinMode(BUS_PINS[i], INPUT_PULLDOWN);
    pinMode(PIN_XTAL1, OUTPUT);
    pinMode(PIN_AS, INPUT);
    pinMode(PIN_DS, INPUT);
    pinMode(PIN_RW, INPUT);
    pinMode(PIN_DM, INPUT);
    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_IRQ0, OUTPUT);
    pinMode(PIN_IRQ1, OUTPUT);
    pinMode(PIN_IRQ2, OUTPUT);
    pinMode(PIN_TXD, INPUT);
    pinMode(PIN_RXD, OUTPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    _freeRunning = false;
    xtal1_lo();
    reset();

    setDeviceBase(Device::USART);

    _rom_begin = 1;
    _rom_end = 0;  // no ROM area
}

Signals &Pins::prepareCycle() {
    Signals &signals = Signals::currCycle();
    // Wait for bus cycle
    while (signal_ds() != LOW) {
        if (signal_as() == LOW) {
            // #AS is asserted
            while (signal_as() == LOW) {
                xtal1_cycle();
            }
            // #AS is high
            // Read address and bus status
            signals.getAddr();
        } else {
            xtal1_cycle();
        }
    }
    // #DS is low
    return signals;
}

Signals &Pins::completeCycle(Signals &signals) {
    // #DS is low
    if (signals.write()) {
        _writes++;
        signals.getData();
        if (Usart.isSelected(signals.addr)) {
            Usart.write(signals.debug('u').data, signals.addr);
        } else if (signals.writeRam()) {
            if (signals.addr <= _rom_end && signals.addr >= _rom_begin) {
                // ROM area, ignore write from CPU;
            } else {
                Memory.write(signals.addr, signals.debug('m').data);
            }
        } else {
            ;  // capture data to signals.data
        }
    } else {
        _writes = 0;
        if (Usart.isSelected(signals.addr)) {
            signals.debug('u').data = Usart.read(signals.addr);
        } else if (signals.readRam()) {
            signals.debug('m').data = Memory.read(signals.addr);
        } else {
            ;  // inject data from signals.data
        }
        signals.outData();
    }
    while (signal_ds() == LOW) {
        xtal1_cycle();
    }
    // #DS is high
    Signals::nextCycle();
    busMode(DATA, INPUT_PULLDOWN);
    return signals;
}

Signals &Pins::cycle() {
    return completeCycle(prepareCycle());
}

Signals &Pins::cycle(uint8_t insn) {
    return completeCycle(prepareCycle().inject(insn));
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
        if (cap < max)
            signals.capture();
        if (inj < len)
            signals.inject(inst[inj]);
        completeCycle(signals);
        if (signals.read()) {
            signals.debug('i');  // injected
            if (inj < len) {
                inj++;
            }
        } else {
            signals.debug('c');  // captured
            if (cap == 0 && addr)
                *addr = signals.addr;
            if (buf && cap < max) {
                buf[cap++] = signals.data;
            }
        }
    }
    return cap;
}

void Pins::reset(bool show) {
    assert_reset();
    // #RESET must remain low for a minimum of 16 clocks.
    for (auto i = 0; i < 16 * 2; i++)
        xtal1_cycle();
    negate_reset();
    // Wait for #AS pulse with #DS high
    while (signal_ds() == LOW) {
        while (signal_as() == LOW)
            xtal1_cycle();
        while (signal_as() != LOW)
            xtal1_cycle();
    }

    Regs.save(show);
    Regs.reset(show);

    Usart.reset();
    SioH.reset();
}

void Pins::idle() {
    static const uint8_t JR[] = {0x8B, 0xFE};
    // Inject JR $
    const auto &signals = Signals::currCycle();
    execInst(JR, sizeof(JR));
    Signals::resetTo(signals);
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

void Pins::halt(bool show) {
    if (_freeRunning) {
        if (show)
            Signals::disassembleCycles();
        Regs.save(debug_cycles);
        turn_off_led();
        _freeRunning = false;
    }
}

void Pins::run() {
    Regs.restore(debug_cycles);
    // Reset cycles for dump valid bus cycles at HALT.
    Signals::resetCycles();
    rawStep();  // step over possible break point
    saveBreakInsns();
    _freeRunning = true;
    turn_on_led();
}

bool Pins::rawStep() {
    auto *signals = &prepareCycle();
    if (signals->write()) {
        // interrupt acknowledge is ongoing
        // finsh saving PC and FLAGS
        while (signals->write()) {
            completeCycle(*signals).debug('I');
            signals = &prepareCycle();
        }
        // fetch vector
        completeCycle(*signals).debug('v');
        completeCycle(prepareCycle()).debug('v');
        signals = &prepareCycle();
    }
    const auto insn = Memory.read(signals->addr);
    const auto cycles = Regs::busCycles(insn);
    if (debug_step) {
        cli.print(F("@@ rawStep at "));
        cli.printHex(signals->addr, 4);
        cli.print(':');
        cli.printHex(insn, 2);
        cli.print(F(" cycles="));
        cli.printlnDec(cycles);
    }
    if (cycles == 0 || insn == 0x7F || insn == 0x6F) {
        if (cycles == 0) {
            cli.print(F("Illegal instruction "));
            cli.printHex(insn, 2);
        } else if (insn == 0x7F) {
            cli.print(F("HALT instruction "));
        } else if (insn == 0x6F) {
            cli.print(F("STOP instruction "));
        }
        cli.print(F(" at "));
        cli.printHex(signals->addr, 4);
        cli.println(F("; HALT"));
        cycle(0x8B);  // JR $
        cycle(0xFE);
        return false;
    }
    completeCycle(*signals).debug('1');
    for (auto c = 1; c < cycles; c++) {
        cycle().debug(c + '1');
        if (_writes == 3) {
            // interrupt is acknowledged, fetch vector
            cycle().debug('v');
            cycle().debug('v');
            return true;
        }
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

void Pins::assertIntr(IntrName intr) {
    switch (intr) {
    default:
        break;
    case INTR_IRQ0:
        assert_irq0();
        break;
    case INTR_IRQ1:
        assert_irq1();
        break;
    case INTR_IRQ2:
        assert_irq2();
        break;
    }
}

void Pins::negateIntr(IntrName intr) {
    switch (intr) {
    default:
        break;
    case INTR_IRQ0:
        negate_irq0();
        break;
    case INTR_IRQ1:
        negate_irq1();
        break;
    case INTR_IRQ2:
        negate_irq2();
        break;
    }
}

static const char TEXT_USART[] PROGMEM = "USART";
static const char TEXT_SIO[] PROGMEM = "SIO";

Pins::Device Pins::parseDevice(const char *name) const {
    if (strcasecmp_P(name, TEXT_USART) == 0)
        return Device::USART;
    if (strcasecmp_P(name, TEXT_SIO) == 0)
        return Device::SIO;
    return Device::NONE;
}

void Pins::getDeviceName(Pins::Device dev, char *name) const {
    *name = 0;
    if (dev == Device::USART)
        strcpy_P(name, TEXT_USART);
    if (dev == Device::SIO)
        strcpy_P(name, TEXT_SIO);
}

void Pins::setDeviceBase(Pins::Device dev, bool hasValue, uint16_t base) {
    switch (dev) {
    case Device::USART:
        setSerialDevice(Device::USART, hasValue ? base : USART_BASE_ADDR);
        break;
    case Device::SIO:
        setSerialDevice(Device::SIO, hasValue ? base : 0xF0);
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
        cli.print(F("at 0"));
        cli.printHex(baseAddr, 2);
        cli.println('H');
    } else {
        cli.println(F("disabled"));
    }
    cli.print(F("SIO (Z86C91) "));
    if (serial == Device::SIO) {
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
    if (_serialDevice == Device::SIO) {
        baseAddr = SioH.baudrate();
    }
    return _serialDevice;
}

void Pins::setSerialDevice(Pins::Device device, uint16_t baseAddr) {
    _serialDevice = device;
    Usart.enable(device == Device::USART, baseAddr);
    SioH.enable(device == Device::SIO);
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
        Regs.disassemble(_breakPoints[i], 1);
    }
}

void Pins::saveBreakInsns() {
    for (uint8_t i = 0; i < _breakNum; ++i) {
        _breakInsns[i] = Memory.read(_breakPoints[i]);
        Memory.write(_breakPoints[i], 0x7F);  // HALT
   }
}

void Pins::restoreBreakInsns() {
    for (uint8_t i = 0; i < _breakNum; ++i) {
        Memory.write(_breakPoints[i], _breakInsns[i]);
    }
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
