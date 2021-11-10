#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "mc6850.h"
#include "regs.h"
#include "sci_handler.h"
#include "string_util.h"

extern libcli::Cli &cli;

class Pins Pins;
Mc6850 Acia(Console);
SciHandler<PIN_SCIRXD, PIN_SCITXD> SciH(Console);

static constexpr bool debug_cycles = false;

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
    pinMode(PIN_CLOCK, OUTPUT);
    pinMode(PIN_E, INPUT);
    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    assert_reset();
    clock_lo();
    _freeRunning = false;

    setIoDevice(SerialDevice::DEV_SCI, 0x10);
}

Signals &Pins::cycle() {
    Signals &signals = Signals::currCycle();
    // MC6803/HD6303 clock E is CLK/4, so we toggle CLK 4 times
    clock_lo();
    clock_hi();
    //
    clock_lo();
    signals.readAddr();
    clock_hi();
    //
    clock_lo();
    clock_hi();
    //
    clock_lo();
    clock_hi();
    signals.get();

    // DO the transaction here
    if (signals.rw == HIGH) {
        if (SciH.isSelected(signals.addr)) {
            signals.debug('s').data = SciH.read(signals.addr);
        } else if (Acia.isSelected(signals.addr)) {
            signals.debug('a').data = Acia.read(signals.addr);
        } else if (signals.readRam()) {
            signals.debug('m').data = Memory.raw_read(signals.addr);
        } else {
            ;  // inject data from signals.data
            signals.debug('i');
        }
        busWrite(AD, signals.data);
        // change data bus to output
        busMode(AD, OUTPUT);
    } else {
        signals.readData();
        if (SciH.isSelected(signals.addr)) {
            SciH.write(signals.debug('s').data, signals.addr);
        } else if (Acia.isSelected(signals.addr)) {
            Acia.write(signals.debug('a').data, signals.addr);
        } else if (signals.writeRam()) {
            Memory.raw_write(signals.addr, signals.debug('m').data);
        } else {
            ;  // capture data to signals.data
            signals.debug('c');
        }
    }
    // Set clock low to handle hold times and tristate data bus.
    clock_lo();
    busMode(AD, INPUT);

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
        Signals::inject(inst[i]).debug('i');
        cycle();
    }
    uint8_t cap = 0;
    if (buf) {
        while (cap < max) {
            Signals::capture().debug('c');
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
    while (clock_e() == HIGH)
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
#if defined(BIONIC_HD6303)
    cycle().debug('e');
#endif
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
        Signals &signals = Signals::capture();
        if (cycle().rw == LOW) {
            writes++;
        } else {
            writes = 0;
        }
        if (writes)
            signals.debug('0' + writes);
    }
    negate_nmi();
    // Capture registers pushed onto stack.
    const Signals *end = &Signals::currCycle() - writes;
    Regs.capture(end);
#if defined(BIONIC_HD6303)
    ;  // No irrelevant bus cycle is necessary.
#else
    cycle().debug('n');
#endif
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
        Signals::resetCycles().debug('N');
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
    Signals::resetCycles().debug('s');
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

Pins::SerialDevice Pins::getIoDevice(uint16_t &baseAddr) {
    if (_ioDevice == SerialDevice::DEV_SCI) {
        baseAddr = 0x10;
    } else {
        baseAddr = Acia.baseAddr();
    }
    return _ioDevice;
}

void Pins::setIoDevice(SerialDevice device, uint16_t baseAddr) {
    _ioDevice = device;
    if (device == SerialDevice::DEV_SCI) {
        Acia.enable(false, 0);
        SciH.enable(true);
    } else {
        SciH.enable(false);
        Acia.enable(true, baseAddr);
    }
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
