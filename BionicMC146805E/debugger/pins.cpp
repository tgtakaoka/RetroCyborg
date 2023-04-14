#include "pins.h"

#include <libcli.h>

#include "commands.h"
#include "config.h"
#include "digital_fast.h"
#include "mc6850.h"
#include "regs.h"
#include "string_util.h"

extern libcli::Cli cli;
class Pins Pins;

Mc6850 Acia(Console);

static constexpr bool debug_cycles = false;

/**
 * MC146805E bus cycle.
 *         __    __    __    __    __    __    __    _
 * OSC1 __|c1|__|c2|__|c3|__|c4|__|c5|__|c1|__|c2|__|
 *       /    \_____A     \             /    _____A
 *   AS ______|     |_______________________|     |___
 *                  \      ___________D           \
 *   DS __________________|           |_______________
 *                   L_____________________________L
 *   LI _____________|                             |__
 *      _                               ______________
 *  R/W  |_____________________________|
 *
 * - OSC1 falling-edge to AS edges takes 70ns.
 * - OSC1 falling-edge to DS edges takes 70ns.
 * - OSC1 falling-edge to LI rising-edge takes 130ns.
 * - AS falling-edge to LI rising-edge takes 60ns.
 * - R/W is valid before rising edge of c1.
 */

#if defined(ARDUINO_TEENSY35)
static constexpr auto osc1_hi_ns = 100;
static constexpr auto osc1_lo_ns = 100;
static constexpr auto delay_osc1_asds_ns = 100;
static constexpr auto raw_c1_lo_ns = 0;
static constexpr auto c1_lo_ns = 0;
static constexpr auto c1_hi_ns = 80;
static constexpr auto c2_lo_ns = 40;
static constexpr auto c2_hi_ns = 0;
static constexpr auto c3_lo_ns = 30;
static constexpr auto c3_hi_ns = 0;
static constexpr auto c4_read_lo_ns = 0;
static constexpr auto c4_read_hi_ns = 0;
static constexpr auto c5_read_lo_ns = 0;
static constexpr auto c4_write_lo_ns = 0;
static constexpr auto c4_write_hi_ns = 0;
static constexpr auto c5_write_lo_ns = 0;
static constexpr auto c5_hi_ns = 0;
#endif
#if defined(ARDUINO_TEENSY41)
static constexpr auto osc1_hi_ns = 84;
static constexpr auto osc1_lo_ns = 76;
static constexpr auto delay_osc1_asds_ns = 100;
static constexpr auto raw_c1_lo_ns = 10;
static constexpr auto c1_lo_ns = 0;
static constexpr auto c1_hi_ns = 84;
static constexpr auto c2_lo_ns = 64;
static constexpr auto c2_hi_ns = 58;
static constexpr auto c3_lo_ns = 58;
static constexpr auto c3_hi_ns = 80;
static constexpr auto c4_read_lo_ns = 76;
static constexpr auto c4_read_hi_ns = 34;
static constexpr auto c5_read_lo_ns = 8;
static constexpr auto c4_write_lo_ns = 64;
static constexpr auto c4_write_hi_ns = 12;
static constexpr auto c5_write_lo_ns = 52;
static constexpr auto c5_hi_ns = 52;
#endif

static inline void osc1_hi() __attribute__((always_inline));
static inline void osc1_hi() {
    digitalWriteFast(PIN_OSC1, HIGH);
}

static inline void osc1_lo() __attribute__((always_inline));
static inline void osc1_lo() {
    digitalWriteFast(PIN_OSC1, LOW);
}

static inline void clock_cycle() __attribute__((always_inline));
static inline void clock_cycle() {
    osc1_hi();
    delayNanoseconds(osc1_hi_ns);
    osc1_lo();
    delayNanoseconds(osc1_lo_ns);
}

static uint8_t signal_ds() {
    return digitalReadFast(PIN_DS);
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
    busMode(B, INPUT);
    busMode(AH, INPUT);

    pinMode(PIN_AS, INPUT);
    pinMode(PIN_RW, INPUT);
    pinMode(PIN_IRQ, OUTPUT);
    pinMode(PIN_TIMER, OUTPUT);
    pinMode(PIN_OSC1, OUTPUT);
    pinMode(PIN_DS, INPUT);
    pinMode(PIN_LI, INPUT);
    pinMode(PIN_RESET, OUTPUT);
    pinMode(PIN_USRSW, INPUT_PULLUP);
    pinMode(PIN_USRLED, OUTPUT);
    turn_off_led();

    assert_reset();
    // Toggle clock to put MC146805E in reset
    for (uint16_t i = 0; i < 1920 * 5; i++)
        clock_cycle();
    _freeRunning = false;

    setDeviceBase(Device::ACIA);
}

Signals &Pins::prepareCycle() {
    // MC146805E bus cycle is CLK/5, so we toggle CLK 5 times
    // c1
    delayNanoseconds(c1_lo_ns);
    Signals &signals = Signals::currCycle();
    busMode(B, INPUT);  // To ensure 160ns data hold time after DS-falling edge.
    osc1_hi();
    delayNanoseconds(c1_hi_ns);
    osc1_lo();  // AS->LOW
    // c2
    delayNanoseconds(c2_lo_ns);
    signals.getDirection();
    osc1_hi();
    delayNanoseconds(c2_hi_ns);
    signals.getAddr1();
    osc1_lo();  // AS->LOW
    // c3
    signals.getAddr2();
    delayNanoseconds(c3_lo_ns);
    osc1_hi();
    delayNanoseconds(c3_hi_ns);
    signals.getLoadInstruction();
    osc1_lo();  // DS->HIGH

    return signals;
}

Signals &Pins::completeCycle(Signals &signals) {
    if (signals.rw == HIGH) {
        // c4
        delayNanoseconds(c4_read_lo_ns);
        osc1_hi();
        if (Acia.isSelected(signals.addr)) {
            signals.debug('a').data = Acia.read(signals.addr);
        } else if (signals.readRam()) {
            signals.debug('m').data = Memory.raw_read(signals.addr);
        } else {
            ;  // inject data from signals.data
        }
        delayNanoseconds(c4_read_hi_ns);
        // c5
        osc1_lo();  // DS=HIGH
        busWrite(B, signals.data);
        // change data bus to output
        busMode(B, OUTPUT);
        delayNanoseconds(c5_read_lo_ns);
    } else {
        // c4
        delayNanoseconds(c4_write_lo_ns);
        osc1_hi();
        signals.getData();
        if (Acia.isSelected(signals.addr)) {
            Acia.write(signals.debug('a').data, signals.addr);
        } else if (signals.writeRam()) {
            Memory.raw_write(signals.addr, signals.debug('m').data);
        } else {
            ;  // capture data to signals.data
        }
        delayNanoseconds(c4_write_hi_ns);
        // c5
        osc1_lo();  // DS=HIGH
        delayNanoseconds(c5_write_lo_ns);
    }
    osc1_hi();
    Signals::nextCycle();
    delayNanoseconds(c5_hi_ns);
    // Data hold time will be done in next cycle().
    osc1_lo();  // DS->LOW

    return signals;
}

Signals &Pins::cycle() {
    return completeCycle(prepareCycle());
}

Signals &Pins::raw_cycle() {
    delayNanoseconds(raw_c1_lo_ns);
    return cycle();
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
        raw_cycle();
    }
    uint8_t cap = 0;
    if (buf) {
        while (cap < max) {
            Signals::capture();
            const Signals &signals = raw_cycle();
            if (cap == 0 && addr)
                *addr = signals.addr;
            buf[cap++] = signals.data;
        }
    }
    return cap;
}

void Pins::reset(bool show) {
    // Reset vector pointing internal memory, we can't save register by SWI.
    const uint16_t reset_vec = Memory.raw_read_uint16(Memory::reset_vector);
    if (Memory::is_internal(reset_vec))
        Memory.raw_write_uint16(Memory::reset_vector, 0x0100);

    // Toggle clock to put MC146805E in reset
    for (uint16_t i = 0; i < 1920 * 5; i++)
        clock_cycle();

    assert_reset();
    // Synchronize clock output to DS.
    while (signal_ds() == LOW) {
        clock_cycle();
        delayNanoseconds(delay_osc1_asds_ns);
    }
    while (signal_ds() != LOW) {
        clock_cycle();
        delayNanoseconds(delay_osc1_asds_ns);
    }
    Signals::resetCycles();
    raw_cycle().debug('R');
    raw_cycle().debug('R');
    negate_reset();
    raw_cycle().debug('r');
    raw_cycle().debug('r');
    // Read Reset vector
    raw_cycle().debug('v');
    raw_cycle().debug('v');
    raw_cycle().debug('-');
    if (show)
        Signals::printCycles();

    // We should certainly inject SWI by pointing external address here.
    Regs.save(show);
    // Restore reset vector points to internal
    if (Memory::is_internal(reset_vec)) {
        Memory.raw_write_uint16(Memory::reset_vector, reset_vec);
        Regs.pc = reset_vec;
    }

    Acia.reset();
}

void Pins::idle() {
    // MC146805E is fully static, so we can stop clock safely.
}

void Pins::loop() {
    if (_freeRunning) {
        Acia.loop();
        Signals &signals = prepareCycle();
        if (signals.fetchInsn()) {
            const auto insn = Memory.raw_read(signals.addr);
            if ((insn & ~1) == 0x8E) {  // STOP/WAIT
                Commands.halt(true);
                return;
            }
        }
        if (user_sw() == LOW) {
            Commands.halt(true);
            return;
        }
        completeCycle(signals);
    } else {
        idle();
    }
}

void Pins::halt(bool show) {
    if (_freeRunning) {
        _freeRunning = false;
        while (true) {
            Signals &signals = Signals::currCycle();
            // Wait until Load Instruction signal asserted
            if (signals.fetchInsn()) {
                const auto insn = Memory.raw_read(signals.addr);
                if ((insn & ~1) == 0x8E) {  // STOP/WAIT
                    Signals::inject(0x20);  // BRA *
                    completeCycle(signals).debug('B');
                    Signals::inject(0xFE);
                    cycle().debug('B');
                    cycle().debug('B');
                    break;
                }
                rawStep(signals);
                break;
            }
            completeCycle(signals);
            prepareCycle();
        }
        if (show)
            Signals::disassembleCycles();
        Regs.save(debug_cycles);
        turn_off_led();
    }
}

void Pins::run() {
    Regs.restore();
    // Reset cycles for dump valid bus cycles at HALT.
    Signals::resetCycles();
    _freeRunning = true;
    turn_on_led();
}

void Pins::rawStep(Signals &signals) {
    const uint8_t insn = Memory.read(signals.addr);
    const uint8_t cycles = Regs.cycles(insn);
    completeCycle(signals).debug('1');
    for (uint8_t c = 1; c < cycles; c++) {
        raw_cycle().debug(c + '1');
    }
}

void Pins::step(bool show) {
    const uint8_t insn = Memory.read(Regs.pc);
    if ((insn & ~1) == 0x8E)  // STOP/WAIT
        return;
    Regs.restore(debug_cycles);
    Signals::resetCycles();
    rawStep(prepareCycle());
    if (show)
        Signals::printCycles();
    Regs.save(debug_cycles);
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

static const char TEXT_ACIA[] PROGMEM = "ACIA";

Pins::Device Pins::parseDevice(const char *name) const {
    if (strcasecmp_P(name, TEXT_ACIA) == 0)
        return Device::ACIA;
    return Device::NONE;
}

void Pins::getDeviceName(Pins::Device dev, char *name) const {
    *name = 0;
    if (dev == Device::ACIA)
        strcpy_P(name, TEXT_ACIA);
}

void Pins::setDeviceBase(Pins::Device dev, bool hasValue, uint16_t base) {
    switch (dev) {
    case Device::ACIA:
        setSerialDevice(Device::ACIA, hasValue ? base : ACIA_BASE_ADDR);
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
}

Pins::Device Pins::getSerialDevice(uint16_t &baseAddr) const {
    if (_serialDevice == Device::ACIA) {
        baseAddr = Acia.baseAddr();
    }
    return _serialDevice;
}

void Pins::setSerialDevice(Pins::Device device, uint16_t baseAddr) {
    _serialDevice = device;
    if (device == Device::ACIA) {
        Acia.enable(true, baseAddr);
    }
}

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
