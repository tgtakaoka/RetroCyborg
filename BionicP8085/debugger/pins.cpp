#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "i8085_sio_handler.h"
#include "i8251.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli cli;
class Pins Pins;
I8251 Usart(Console);
I8085SioHandler Sio(Console);

static constexpr bool debug_cycles = false;

/**
 * P8085 bus cycle.
 *        __    __    __    __    __    __    __    __    __    __
 *    X1 |  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |
 *       __       _____       _____       _____       _____       _
 *   CLK   |____T|1    |____T|2    |____T|3    |____T|4    |____T|1
 *
 * - P8085AH X1 can be maximum 6 MHz (167ns), thus CLK can be maximum
 *   3 MHz (333ns). (P8085AH-2 X1 is 10 MHz, P8085AH-1 X1 is 12 MHz)
 */

#if defined(ARDUINO_TEENSY41)
static constexpr auto x1_cycle_ns = 64;
static constexpr auto clk_hi_x1_ns = 54;
static constexpr auto clk_lo_x1_ns = 54;
static constexpr auto clk_hi_ns = 44;
static constexpr auto clk_lo_ns = 24;
static constexpr auto t1_lo_ns = 80;
static constexpr auto t2_lo_ns = 20;
#endif

static inline void x1_hi() {
    digitalWriteFast(PIN_X1, HIGH);
}

static inline void x1_lo() {
    digitalWriteFast(PIN_X1, LOW);
}

static inline void x1_cycle() {
    x1_hi();
    delayNanoseconds(x1_cycle_ns);
    x1_lo();
    delayNanoseconds(x1_cycle_ns);
}

static inline auto signal_clk() {
    return digitalReadFast(PIN_CLK);
}

void Pins::clk_hi() const {
    x1_hi();
    delayNanoseconds(clk_hi_x1_ns);
    if (_freeRunning)
        Sio.loop();
    x1_lo();
}

void Pins::clk_lo() const {
    x1_hi();
    delayNanoseconds(clk_lo_x1_ns);
    x1_lo();
}

void Pins::clk_cycle() const {
    clk_hi();
    delayNanoseconds(clk_hi_ns);
    clk_lo();
    delayNanoseconds(clk_lo_ns);
}

static inline void assert_trap() {
    digitalWriteFast(PIN_TRAP, HIGH);
}

static inline void negate_trap() {
    digitalWriteFast(PIN_TRAP, LOW);
}

static inline void assert_intr() {
    digitalWriteFast(PIN_INTR, HIGH);
}

static inline void negate_intr() {
    digitalWriteFast(PIN_INTR, LOW);
}

static inline auto signal_ale() {
    return digitalReadFast(PIN_ALE);
}

static inline auto signal_rd() {
    return digitalReadFast(PIN_RD);
}

static inline auto signal_wr() {
    return digitalReadFast(PIN_WR);
}

static inline auto signal_inta() {
    return digitalReadFast(PIN_INTA);
}

static inline void assert_ready() {
    digitalWriteFast(PIN_READY, HIGH);
}

static inline void negate_ready() {
    digitalWriteFast(PIN_READY, LOW);
}

static inline auto signal_ready() {
    return digitalReadFast(PIN_READY);
}

static inline void assert_hold() {
    digitalWriteFast(PIN_HOLD, HIGH);
}

static inline void negate_hold() {
    digitalWriteFast(PIN_HOLD, LOW);
}

static void assert_reset() {
    // Drive RESET condition
    x1_lo();
    digitalWriteFast(PIN_RESET, LOW);
    negate_hold();
    negate_trap();
    negate_intr();
    assert_ready();
    digitalWriteFast(PIN_RST55, LOW);
    digitalWriteFast(PIN_RST65, LOW);
    digitalWriteFast(PIN_RST75, LOW);
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
        PIN_S0,
        PIN_S1,
};

void Pins::begin() {
    // Set directions.
    for (uint8_t i = 0; i < sizeof(BUS_PINS); i++)
        pinMode(BUS_PINS[i], INPUT);
    pinMode(PIN_RST55, OUTPUT);
    pinMode(PIN_RST65, OUTPUT);
    pinMode(PIN_RST75, OUTPUT);
    pinMode(PIN_INTR, OUTPUT);
    pinMode(PIN_TRAP, OUTPUT);
    pinMode(PIN_X1, OUTPUT);
    pinMode(PIN_ALE, INPUT);
    pinMode(PIN_IOM, INPUT_PULLUP);
    pinMode(PIN_HLDA, INPUT);
    pinMode(PIN_INTA, INPUT_PULLUP);
    pinMode(PIN_RD, INPUT_PULLUP);
    pinMode(PIN_WR, INPUT_PULLUP);
    pinMode(PIN_READY, OUTPUT);
    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_HOLD, OUTPUT);
    pinMode(PIN_SOD, INPUT_PULLUP);
    pinMode(PIN_SID, OUTPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    assert_reset();
    // Syncronize X1 and CLK.
    while (signal_clk() == LOW)
        x1_cycle();
    while (signal_clk() != LOW)
        x1_cycle();
    // Now CLK=L and X1=L
    _freeRunning = false;
    _inta = Regs::NOP;

    setDeviceBase(Device::USART);

    _rom_begin = 1;
    _rom_end = 0;  // no ROM area
}

Signals &Pins::cycleT1() {
    Signals &signals = Signals::currCycle();
    // Do T4/T5 if any, and confirm T1L by ALE=H
    while (signal_ale() == LOW)
        clk_cycle();
    // T1H
    clk_hi();
    while (signal_ale() == HIGH)
        ;
    signals.getAddress();
    // T2L
    clk_lo();
    return signals;
}

Signals &Pins::cycleT2() {
    Signals &signals = Signals::currCycle();
    delayNanoseconds(t2_lo_ns);
    clk_cycle();
    signals.getDirection();
    return signals;
}

Signals &Pins::cycleT2Ready() {
    negate_ready();
    return cycleT2();
}

Signals &Pins::cycleT2Wait(uint16_t pc) {
    assert_ready();
    return cycleT2().setAddress(pc);
}

Signals &Pins::cycleT3(Signals &signals) {
    // T3L
    if (signals.iom == LOW) {  // Memory access
        if (signals.rd == LOW) {
            if (signals.readRam()) {
                signals.debug('m').data = Memory.raw_read(signals.addr);
            } else {
                signals.debug('i');  // inject data from signals.data
            }
            busWrite(AD, signals.data);
            busMode(AD, OUTPUT);
        } else if (signals.wr == LOW) {
            signals.getData();
            if (signals.writeRam()) {
                if (signals.addr <= _rom_end && signals.addr >= _rom_begin) {
                    // ROM area, ignore write from CPU;
                } else {
                    Memory.write(signals.addr, signals.debug('m').data);
                }
            } else {
                signals.debug('c');  // capture data to signals.data
            }
        }
    } else {  // I/O access
        const uint8_t ioaddr = signals.addr;
        if (signals.rd == LOW) {
            if (Usart.isSelected(ioaddr)) {
                signals.debug('u').data = Usart.read(ioaddr);
            } else {
                signals.debug('U').data = 0;
            }
            busWrite(AD, signals.data);
            busMode(AD, OUTPUT);
        } else if (signals.wr == LOW) {
            signals.getData();
            if (Usart.isSelected(ioaddr)) {
                Usart.write(signals.debug('u').data, ioaddr);
            } else {
                signals.debug('U');
            }
        } else if (signals.inta == LOW) {
            signals.debug('a').data = _inta;
            busWrite(AD, signals.data);
            busMode(AD, OUTPUT);
            _inta = Regs::NOP;
        }
    }
    // T3H
    clk_hi();
    Signals::nextCycle();
    busMode(AD, INPUT);
    // T1L or T4L/T5L
    clk_lo();
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
        Signals &signals = (inj == 0) ? cycleT2Wait(Regs.nextIp()) : cycleT2();
        if (signals.rd == LOW) {
            Signals::inject(inst[inj++]);
        } else if (signals.wr == LOW) {
            Signals::capture();
        }
        cycleT3(signals);
        if (signals.wr == LOW && buf) {
            if (cap == 0 && addr)
                *addr = signals.addr;
            if (cap < max)
                buf[cap++] = signals.data;
        }
        delayNanoseconds(t1_lo_ns);
        cycleT1();
    }
    cycleT2Ready();
    return cap;
}

void Pins::reset(bool show) {
    Signals::resetCycles();
    assert_reset();
    // Syncronize X1 and CLK.
    while (signal_clk() == LOW)
        x1_cycle();
    while (signal_clk() != LOW)
        x1_cycle();
    // #RESET_IN should be kept low for a minimum of three clock
    // #periods to ensure proper synchronization of the CPU.
    for (auto i = 0; i < 3; i++)
        clk_cycle();

    _freeRunning = false;
    _inta = Regs::NOP;

    negate_reset();
    // #RESET_IN is sampled here falling transition of next CLK.
    clk_cycle();
    cycleT1();
    cycleT2Ready();

    Regs.save(show);

    Usart.reset();
    Sio.reset();
}

void Pins::idle() {
    cycleT2Ready();
}

void Pins::loop() {
    if (_freeRunning) {
        Usart.loop();
        cycleT1();
        cycleT3(cycleT2());
        if (user_sw() == LOW) {
            Commands.halt(true);
            Signals::resetCycles();
        }
    } else {
        Signals::inject(0x00);
        idle();
    }
}

void Pins::halt(bool show) {
    if (_freeRunning) {
        show |= debug_cycles;
        if (debug_cycles)
            cli.println(F("@@ halt"));
        while (true) {
            Signals &signals = cycleT1();
            if (signals.fetchInsn())
                break;
            cycleT3(cycleT2());
            delayNanoseconds(t1_lo_ns);
        }
        cycleT2Ready();
        if (show)
            Signals::disassembleCycles();
        Regs.save(debug_cycles);
        turn_off_led();
        _freeRunning = false;
    }
}

void Pins::run() {
    Regs.restore(debug_cycles);
    // Mark cycles for dumping valid bus cycles at HALT.
    Signals::resetCycles();
    cycleT3(cycleT2Wait(Regs.nextIp()));
    _freeRunning = true;
    turn_on_led();
}

void Pins::step(bool show) {
    Regs.restore(debug_cycles);
    if (debug_cycles)
        cli.println(F("@@ step"));
    if (show)
        Signals::resetCycles();
    char c = '1';
    cycleT3(cycleT2Wait(Regs.nextIp())).debug(c++);
    while (true) {
        Signals &signals = cycleT1();
        if (signals.fetchInsn())
            break;
        cycleT3(cycleT2());
        delayNanoseconds(t1_lo_ns);
    }
    cycleT2Ready();
    if (show)
        Signals::printCycles();
    Regs.save(debug_cycles);
}

uint8_t Pins::intrToInta(IntrName intr) {
    switch (intr) {
    default:
        return Regs::RST_0 | (uint8_t(intr) & 0x38);
    case INTR_RST55:
    case INTR_RST65:
    case INTR_RST75:
    case INTR_TRAP:
        return Regs::NOP;
    }
}

void Pins::assertIntr(IntrName intr) {
    switch (intr) {
    default:
        assert_intr();
        break;
    case INTR_RST55:
        digitalWriteFast(PIN_RST55, HIGH);
        break;
    case INTR_RST65:
        digitalWriteFast(PIN_RST65, HIGH);
        break;
    case INTR_RST75:
        digitalWriteFast(PIN_RST75, HIGH);
        break;
    case INTR_TRAP:
        assert_trap();
        break;
    case INTR_NONE:
        break;
    }
    _inta = intrToInta(intr);
}

void Pins::negateIntr(IntrName intr) {
    switch (intr) {
    default:
        negate_intr();
        break;
    case INTR_RST55:
        digitalWriteFast(PIN_RST55, LOW);
        break;
    case INTR_RST65:
        digitalWriteFast(PIN_RST65, LOW);
        break;
    case INTR_RST75:
        digitalWriteFast(PIN_RST75, LOW);
        break;
    case INTR_TRAP:
        negate_trap();
        break;
    case INTR_NONE:
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
        setSerialDevice(Device::SIO, hasValue ? base : 0);
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
    cli.print(F("SIO (i8085) "));
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
        baseAddr = Sio.baudrate();
    }
    return _serialDevice;
}

void Pins::setSerialDevice(Pins::Device device, uint16_t baseAddr) {
    _serialDevice = device;
    if (device == Device::USART) {
        Usart.enable(true, baseAddr);
        Sio.enable(false);
    }
    if (device == Device::SIO) {
        Usart.enable(false, 0);
        Sio.enable(true);
    }
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
