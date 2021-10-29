#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "mc6850.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli &cli;
class Pins Pins;

uint8_t RAM[0x2000];

Mc6850 Acia(Console);

static inline void clock_hi() {
    digitalWriteFast(PIN_CLOCK, HIGH);
    delayMicroseconds(1);
}

static inline void clock_lo() {
    digitalWriteFast(PIN_CLOCK, LOW);
    delayMicroseconds(1);
}

static void clock_cycle() {
    clock_hi();
    clock_lo();
}

static void assert_irq() {
    digitalWriteFast(PIN_IRQ, LOW);
}

static void negate_irq() {
    digitalWriteFast(PIN_IRQ, HIGH);
}

static void assert_reset() {
    // Drive RESET condition
    digitalWriteFast(PIN_RESET, LOW);
    negate_irq();
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
        PIN_B0,
        PIN_B1,
        PIN_B2,
        PIN_B3,
        PIN_B4,
        PIN_B5,
        PIN_B6,
        PIN_B7,
        PIN_AH8,
        PIN_AH9,
        PIN_AH10,
        PIN_AH11,
        PIN_AH12,
};

void Pins::begin() {
    // Set directions.
    for (uint8_t i = 0; i < sizeof(BUS_PINS); i++)
        pinMode(BUS_PINS[i], INPUT);
    busMode(DB, INPUT);
    busMode(AH, INPUT);

    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_CLOCK, OUTPUT);
    pinMode(PIN_AS, INPUT);
    pinMode(PIN_DS, INPUT);
    pinMode(PIN_RW, INPUT);
    pinMode(PIN_LI, INPUT);
    pinMode(PIN_IRQ, OUTPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    clock_lo();
    assert_reset();
    // Toggle clock to put MC146805E in reset
    for (int i = 0; i < 1920; i++)
        clock_cycle();
    _freeRunning = false;

    setIoDevice(SerialDevice::DEV_ACIA, ioBaseAddress());
}

Signals &Pins::cycle() {
    Signals &signals = Signals::currCycle();
    // MC146805E bus cycle is CLK/5, so we toggle CLK 5 times
    clock_hi();
    clock_lo();
    //
    clock_hi();
    signals.readAddr();
    clock_lo();
    //
    clock_hi();
    clock_lo();
    //
    clock_hi();
    signals.get();
    clock_lo();

    // DO the transaction here
    if (signals.rw == HIGH) {
        if (Acia.isSelected(signals.addr)) {
            signals.debug('a').data = Acia.read(signals.addr);
        } else if (signals.readRam()) {
            signals.debug('m').data = RAM[signals.addr];
        } else {
            ;  // inject data from signals.data
            signals.debug('i');
        }
        busWrite(DB, signals.data);
        // change data bus to output
        busMode(DB, OUTPUT);
    } else {
        signals.readData();
        if (Acia.isSelected(signals.addr)) {
            Acia.write(signals.debug('A').data, signals.addr);
        } else if (signals.writeRam()) {
            RAM[signals.addr] = signals.debug('M').data;
        } else {
            ;  // capture data to signals.data
            signals.debug('C');
        }
    }
    //
    clock_hi();
    clock_lo();
    busMode(DB, INPUT);

    Signals::nextCycle();
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
        Signals::currCycle().inject(inst[i]).debug('i');
        cycle();
    }
    uint8_t cap = 0;
    if (buf) {
        while (cap < max) {
            Signals::currCycle().capture().debug('c');
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
    // Synchronize clock output to DS.
    while (digitalReadFast(PIN_DS) == LOW)
        clock_cycle();
    while (digitalReadFast(PIN_DS) == HIGH)
        clock_cycle();
    cycle();
    Signals::resetCycles();
    cycle().debug('R');
    negate_reset();
    cycle().debug('r');
    cycle().debug('r');
    // Read Reset vector
    cycle().debug('V');
    cycle().debug('v');
    cycle().debug('d');
    if (show)
        Signals::printCycles();
    Regs.save(show);
}

void Pins::loop() {
    if (_freeRunning) {
        Acia.loop();
        cycle();
        if (user_sw() == LOW)
            Commands.halt();
    }
}

void Pins::halt(bool show) {
    if (_freeRunning) {
        _freeRunning = false;
        Signals::resetCycles();
        while (true) {
            Signals &signals = cycle();
            if (signals.li == HIGH) {
                const uint8_t insn = RAM[signals.debug('0').addr];
                const uint8_t cycles = Regs.cycles(insn);
                for (uint8_t c = 1; c < cycles; c++) {
                    cycle().debug(c < 10 ? '0' + c : 'a' + c - 10);
                }
                break;
            }
        }
        if (show)
            Signals::printCycles();
        turn_off_led();
        Regs.save(show);
    }
}

void Pins::run() {
    Regs.restore();
    _freeRunning = true;
    turn_on_led();
}

void Pins::step(bool show) {
    const uint8_t insn = RAM[Regs.pc];
    const uint8_t cycles = Regs.cycles(insn);
    Regs.restore(show);
    Signals::resetCycles();
    for (uint8_t c = 0; c < cycles; c++) {
        cycle().debug(c < 10 ? '0' + c : 'a' + c - 10);
    }
    if (show)
        Signals::printCycles();
    Regs.save(show);
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

Pins::SerialDevice Pins::getIoDevice(uint16_t &baseAddr) {
    baseAddr = Acia.baseAddr();
    return _ioDevice;
}

void Pins::setIoDevice(SerialDevice device, uint16_t baseAddr) {
    _ioDevice = device;
    Acia.enable(true, baseAddr);
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
