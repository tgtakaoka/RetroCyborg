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
Mc6850 Acia(Console);

static constexpr bool debug_cycles = false;

static inline void clock_hi() {
    digitalWriteFast(PIN_EXTAL, HIGH);
    delayMicroseconds(1);
}

static inline void clock_lo() {
    digitalWriteFast(PIN_EXTAL, LOW);
    delayMicroseconds(1);
}

static void clock_cycle() {
    clock_lo();
    clock_hi();
}

static uint8_t clock_e() {
    return digitalReadFast(PIN_E);
}

static void assert_nmi() {
    digitalWriteFast(PIN_NMI, LOW);
}

static void negate_nmi() {
    digitalWriteFast(PIN_NMI, HIGH);
}

static void assert_irq() {
    digitalWriteFast(PIN_IRQ, LOW);
}

static void negate_irq() {
    digitalWriteFast(PIN_IRQ, HIGH);
}

// TODO: Utilize #HALT to Pins.step()
static void assert_halt() __attribute__((unused));
static void assert_halt() {
    digitalWriteFast(PIN_HALT, LOW);
}

static void negate_halt() {
    digitalWriteFast(PIN_HALT, HIGH);
}

static void negate_mr() {
    digitalWriteFast(PIN_MR, HIGH);
}

static void assert_reset() {
    // Drive RESET condition
    digitalWriteFast(PIN_RESET, LOW);
    negate_halt();
    negate_nmi();
    negate_irq();
    negate_mr();
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
};

void Pins::begin() {
    // Set directions.
    for (uint8_t i = 0; i < sizeof(BUS_PINS); i++)
        pinMode(BUS_PINS[i], INPUT_PULLUP);
    busMode(D, INPUT);
    busMode(AL, INPUT);
#if defined(AM_vp)
    busMode(AM, INPUT);
#endif
    busMode(AH, INPUT);

    pinMode(PIN_HALT, OUTPUT);
    pinMode(PIN_RW, INPUT);
    pinMode(PIN_IRQ, OUTPUT);
    pinMode(PIN_NMI, OUTPUT);
    pinMode(PIN_EXTAL, OUTPUT);
    pinMode(PIN_E, INPUT);
    pinMode(PIN_VMA, INPUT);
    pinMode(PIN_RE, INPUT_PULLUP);
    pinMode(PIN_MR, OUTPUT);
    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_BA, INPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    assert_reset();
    clock_lo();
    _freeRunning = false;

    setIoDevice(SerialDevice::DEV_ACIA, ioBaseAddress());
}

Signals &Pins::cycle() {
    Signals &signals = Signals::currCycle();
    // MC6802/MC6808 clock E is CLK/4, so we toggle CLK 4 times
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
    signals.get();

    if (signals.vma == LOW) {
        signals.debug('-');
    } else if (signals.rw == HIGH) {
        if (Acia.isSelected(signals.addr)) {
            signals.debug('a').data = Acia.read(signals.addr);
        } else if (signals.readRam()) {
            signals.debug('m').data = Memory.raw_read(signals.addr);
        } else {
            ;  // inject data from signals.data
            signals.debug('i');
        }
        busWrite(D, signals.data);
        // change data bus to output
        busMode(D, OUTPUT);
        signals.readData();
    } else {
        signals.readData();
        if (Acia.isSelected(signals.addr)) {
            Acia.write(signals.debug('a').data, signals.addr);
        } else if (signals.writeRam()) {
            Memory.raw_write(signals.addr, signals.debug('m').data);
        } else {
            ;  // capture data to signals.data
            signals.debug('c');
        }
    }
    // Set clock low to handle hold times and tristate data bus.
    clock_hi();
    clock_lo();
    clock_hi();
    busMode(D, INPUT);

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
    // Reset vector should not point internal registers.
    const uint16_t reset_vec = Memory.raw_read_uint16(Memory::reset_vector);
    if (Memory::is_internal(reset_vec))
        Memory.raw_write_uint16(Memory::reset_vector, 0x0100);

    assert_reset();
    // Synchronize clock output and E clock input.
    while (clock_e() == LOW)
        clock_cycle();
    while (clock_e() == HIGH)
        clock_cycle();
    for (auto i = 0; i < 3; i++)
        cycle();
    Signals::resetCycles();
    cycle().debug('R');
    negate_reset();
    cycle().debug('r');
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
}

void Pins::idle() {
    // Inject "BRA *"
    Signals::inject(0x20);
    cycle();
    Signals::inject(0xFE);
    cycle();
    Signals::inject(0);
    cycle();
    Signals::inject(0);
    cycle();
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

void Pins::suspend(bool show) {
    uint8_t writes = 0;
    assert_nmi();
    // Wait for consequtive 7 writes which means registers saved onto stack.
    while (writes < 7) {
        Signals signals = Signals::capture();
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
    cycle().debug('n');
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
