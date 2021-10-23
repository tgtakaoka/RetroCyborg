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

//#define DEBUG_CYCLE
#if defined(DEBUG_CYCLE)
#define DO_DEBUG_CYCLE(s) s
#else
#define DO_DEBUG_CYCLE(s)
#endif
DO_DEBUG_CYCLE(bool debug_cycle; uint32_t count);

uint8_t RAM[0x10000];

Mc6850 Acia(Console);
SciHandler<PIN_SCIRXD, PIN_SCITXD> SciH(Console);

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
}

static void negate_nmi() {
    digitalWriteFast(PIN_NMI, HIGH);
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

    // Toggle reset to put MC6803 in reset
    clock_cycle();
    clock_cycle();
    clock_cycle();
    clock_cycle();
}

static void inject_mode(uint8_t mode) {
    pinMode(PIN_PC2, OUTPUT);
    digitalWriteFast(PIN_PC2, (mode & 4) ? HIGH : LOW);
    pinMode(PIN_PC1, OUTPUT);
    digitalWriteFast(PIN_PC1, (mode & 2) ? HIGH : LOW);
    pinMode(PIN_PC0, OUTPUT);
    digitalWriteFast(PIN_PC0, (mode & 1) ? HIGH : LOW);
}

static void negate_reset() {
    // Release RESET conditions
    digitalWriteFast(PIN_RESET, HIGH);
}

static void release_mode() {
    pinMode(PIN_PC2, INPUT_PULLUP);
    pinMode(PIN_PC1, INPUT_PULLUP);
    pinMode(PIN_PC0, INPUT_PULLUP);
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
    busMode(DB, INPUT);
    busMode(AH, INPUT);

    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_CLOCK, OUTPUT);
    pinMode(PIN_E, INPUT);
    pinMode(PIN_AS, INPUT);
    pinMode(PIN_RW, INPUT);
    pinMode(PIN_IRQ1, OUTPUT);
    pinMode(PIN_PC0, INPUT_PULLUP);
    pinMode(PIN_PC1, INPUT_PULLUP);
    pinMode(PIN_PC2, INPUT_PULLUP);
    // #NMI is connected to P21/PC1 for LILBUG trace.
    pinMode(PIN_NMI, OUTPUT_OPENDRAIN);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    assert_reset();
    clock_lo();
    _freeRunning = false;

    Acia.enable(false, ioBaseAddress());
    SciH.enable(true);
}

Signals &Pins::cycle() {
    Signals &signals = Signals::currCycle();
    // MC6803 clock E is CLK/4, so we toggle CLK 4 times
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
    signals.get().debug('R');

    // DO the transaction here
    DO_DEBUG_CYCLE(char rw; char state);
    if (signals.rw == HIGH) {
        DO_DEBUG_CYCLE(rw = 'R');
        // change data bus to output
        busMode(DB, OUTPUT);
        if (SciH.isSelected(signals.addr)) {
            signals.data = SciH.read(signals.addr);
            DO_DEBUG_CYCLE(state = 'S');
        } else if (Acia.isSelected(signals.addr)) {
            signals.data = Acia.read(signals.addr);
            DO_DEBUG_CYCLE(state = 'A');
        } else if (signals.readRam()) {
            signals.data = RAM[signals.addr];
            DO_DEBUG_CYCLE(state = 'M');
        } else {
            ;  // inject data from signals.data
            DO_DEBUG_CYCLE(state = 'I');
        }
        busWrite(DB, signals.data);
    } else {
        DO_DEBUG_CYCLE(rw = 'W');
        signals.readData().debug('W');
        if (SciH.isSelected(signals.addr)) {
            SciH.write(signals.data, signals.addr);
            DO_DEBUG_CYCLE(state = 'S');
        } else if (Acia.isSelected(signals.addr)) {
            Acia.write(signals.data, signals.addr);
            DO_DEBUG_CYCLE(state = 'A');
        } else if (signals.writeRam()) {
            RAM[signals.addr] = signals.data;
            DO_DEBUG_CYCLE(state = 'M');
        } else {
            ;  // capture data to signals.data
            DO_DEBUG_CYCLE(state = 'C');
        }
    }
    DO_DEBUG_CYCLE(do {
        if (debug_cycle) {
            cli.printDec(count++, 6);
            cli.print('@');
            cli.printHex(signals.addr, 4);
            cli.print(rw);
            cli.print(state);
            cli.printHex(signals.data, 2);
            cli.println();
        }
    } while (0));

    // Set clock low to handle hold times and tristate data bus.
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
    inject_mode(MPU_MODE);
    // Synchronize clock output and E clock input.
    while (clock_e() == LOW)
        clock_cycle();
    while (clock_e() == HIGH)
        clock_cycle();
    for (int i = 0; i < 24; i++)
        cycle();
    Signals::resetCycles();
    cycle().debug('R');
    negate_reset();
    cycle().debug('r');
    release_mode();
    // Read Reset vector
    cycle().debug('V');
    cycle().debug('v');
    if (show)
        Signals::printCycles();
    Regs.save(show);

    SciH.reset();
}

void Pins::loop() {
    if (_freeRunning) {
        Acia.loop();
        SciH.loop();
        cycle();
        if (user_sw() == LOW)
            Commands.halt();
    } else {
        DO_DEBUG_CYCLE(debug_cycle = false);
        Signals::resetCycles();
        // Inject "BRA *"
        Signals::currCycle().inject(0x20);
        cycle();
        Signals::currCycle().inject(0xFE);
        cycle();
        Signals::currCycle().inject(0);
        cycle();
        DO_DEBUG_CYCLE(debug_cycle = false);
    }
}

void Pins::halt(bool show) {
    if (_freeRunning) {
        Signals::resetCycles();
        uint8_t needFlush = 0;
        uint8_t writes = 0;
        assert_nmi();
        while (writes < 7) {
            Signals signals = Signals::currCycle().capture();
            if (cycle().rw == LOW) {
                writes++;
            } else {
                needFlush = Signals::flushCycles(needFlush);
                writes = 0;
            }
            signals.debug('0' + writes);
        }
        negate_nmi();
        Regs.set(&Signals::currCycle() - writes);
        cycle().debug('n');
        // Inject the current PC as NMI vector.
        Signals::currCycle().inject(Regs.pc >> 8);
        cycle().debug('V');
        Signals::currCycle().inject(Regs.pc);
        cycle().debug('v');
        if (show)
            Signals::printCycles();
        _freeRunning = false;
        turn_off_led();
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
        SciH.loop();
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
        assert_irq1();
}

void Pins::negateIrq(uint8_t irq) {
    _irq &= ~(1 << irq);
    if (_irq == 0)
        negate_irq1();
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
