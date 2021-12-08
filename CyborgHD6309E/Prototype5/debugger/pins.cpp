/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */

#include "pins.h"

#include <Arduino.h>
#include <libcli.h>

#include "commands.h"
#include "digital_fast.h"
#include "mc6850.h"
#include "string_util.h"

#ifdef SPI_MAPPING
#include <SPI.h>
#endif

extern libcli::Cli &cli;

static inline void assert_reset() {
    digitalWrite(RESET, LOW);
}

static inline void negate_reset() {
    digitalWrite(RESET, HIGH);
}

static inline void assert_halt() {
    digitalWrite(HALT, LOW);
}

static inline void negate_halt() {
    digitalWrite(HALT, HIGH);
}

static inline void assert_irq() {
    digitalWrite(IRQ, LOW);
}

static inline void negate_irq() {
    digitalWrite(IRQ, HIGH);
}

static inline void negate_nmi() {
    digitalWrite(NMI, HIGH);
}

static inline bool write_bus_cycle() {
    return digitalRead(RD_WR) == LOW;
}

static inline bool is_running() {
#ifdef SIGNALS_BUS
    const uint8_t pins = busRead(SIGNALS);
    return (pins & (_BV(BA_PIN) | _BV(BS_PIN) | _BV(LIC_PIN))) == 0;
#else
    return digitalRead(BA) == LOW && digitalRead(BS) == LOW &&
           digitalRead(LIC) == LOW;
#endif
}

static inline bool valid_bus_cycle(const Signals *prev) {
#ifdef SIGNALS_BUS
    const uint8_t pins = busRead(SIGNALS);
    return (pins & (_BV(BA_PIN) | _BV(BS_PIN))) == 0 && prev &&
           prev->advancedValidMemoryAddress();
#else
    return digitalRead(BA) == LOW && digitalRead(BS) == LOW && prev &&
           prev->advancedValidMemoryAddress();
#endif
}

static inline void enable_ram() {
    digitalWrite(RAM_E, LOW);
}

static inline void disable_ram() {
    digitalWrite(RAM_E, HIGH);
}

static inline bool user_switch_asserted() {
    return digitalRead(USR_SW) == LOW;
}

static inline void turnon_led() {
    digitalWrite(USR_LED, HIGH);
}

static inline void turnoff_led() {
    digitalWrite(USR_LED, LOW);
}

static inline void toggle_led() __attribute__((unused));
static inline void toggle_led() {
    if (digitalRead(USR_LED)) {
        digitalWrite(USR_LED, LOW);
    } else {
        digitalWrite(USR_LED, HIGH);
    }
}

class Pins Pins;

class Mc6850 Acia(Console);

void Pins::Dbus::begin() {
    busMode(DB, INPUT_PULLUP);
}

void Pins::Dbus::setDbus(uint8_t dir, uint8_t data) {
    if (dir == OUTPUT && write_bus_cycle()) {
        cli.println(F("!! R/W is LOW"));
        return;
    }
    if (dir == OUTPUT || _capture) {
        disable_ram();
    } else {
        enable_ram();
    }
    if (dir == INPUT) {
        busMode(DB, INPUT);
    } else {
        busMode(DB, OUTPUT);
        busWrite(DB, data);
    }
}

void Pins::Dbus::input() {
    setDbus(INPUT, 0);
}

void Pins::Dbus::output() {
    if (_valid) {
        _valid = false;
        setDbus(OUTPUT, _data);
    } else {
        input();
    }
}

void Pins::Dbus::set(uint8_t data) {
    _data = data;
    _valid = true;
}

void Pins::Dbus::capture(bool enabled) {
    _capture = enabled;
}

void Pins::print() {
    Signals &signals = Signals::currCycle();
    signals.get();
    signals.print();
}

/*
 * Start clock oscillator from Q=L and E=L
 */
static inline void startOscillator() {
    TCA0.SPLIT.CTRLA = 0;
    TCA0.SPLIT.LCNT = (CLK_PER_MPU / 4) * 2;   // initialize Q_OSC at 1.0pi rad.
    TCA0.SPLIT.HCNT = (CLK_PER_MPU / 4) * 3;   // initialize E_OSC at 1.5pi rad.
    TCA0.SPLIT.CTRLB = TCA_SPLIT_LCMP0EN_bm |  // enable LCMP0(Q_OSC)
                       TCA_SPLIT_HCMP0EN_bm;   // enable HCMP0(E_OSC)
    TCA0.SPLIT.CTRLC = 0;  // set L to LCMP0OV(Q_OSC) HCMP0OV(E_OSC)

    CCL.LUT1CTRLA = CCL_ENABLE_bm;                 // enable Q_CLK
    EVSYS.USEREVOUTC = EVSYS_CHANNEL_CHANNEL5_gc;  // enable EVOUTC(Q_CLK)
    CCL.LUT0CTRLA = CCL_ENABLE_bm;                 // enable E_CLK
    EVSYS.USEREVOUTF = EVSYS_CHANNEL_CHANNEL3_gc;  // enable EVOUTF(E_CLK)

    TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV1_gc |  // clock=CLK_PER/1
                       TCA_SPLIT_ENABLE_bm;        // enable TCA0(Q_OSC/E_OSC)
}

/*
 * Stop clock oscillator at Q=H and E=H
 */
static inline void stopOscillator() {
    TCA0.SPLIT.CTRLA = 0;

    digitalWrite(Q_CLK, HIGH);
    EVSYS.USEREVOUTC = EVSYS_CHANNEL_OFF_gc;  // disable EVOUTC(Q_CLK)
    CCL.LUT1CTRLA = 0;                        // disable Q_CLK

    digitalWrite(E_CLK, HIGH);
    EVSYS.USEREVOUTF = EVSYS_CHANNEL_OFF_gc;  // disable EVOUTF(E_CLK)
    CCL.LUT0CTRLA = 0;                        // disable E_CLK
}

static void setupPORTMUX() {
    PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTB_gc;  // TCA0 on PB[0:5]
    Q_OSC_PINCTRL = PORT_ISC_INTDISABLE_gc;     // Q_OSC: enable input buffer
    Q_OSC_DIRSET = Q_OSC_bm;                    // Q_OSC: output
    E_OSC_PINCTRL = PORT_ISC_INTDISABLE_gc;     // E_OSC: enable input buffer
    E_OSC_DIRSET = E_OSC_bm;                    // E_OSC: output
    PORTMUX.EVSYSROUTEA &= ~PORTMUX_EVOUT2_bm;  // EVOUTC on PC2(Q_CLK)
    PORTMUX.EVSYSROUTEA &= ~PORTMUX_EVOUT5_bm;  // EVOUTF on PF2(E_CLK)
}

static void setupEVSYS() {
    EVSYS.USERCCLLUT0A = EVSYS_CHANNEL_CHANNEL2_gc;  // CCLLUT0A=#INT
    EVSYS.USERCCLLUT0B = EVSYS_CHANNEL_CHANNEL1_gc;  // CCLLUT0B=E_OSC
    EVSYS.USERCCLLUT1A = EVSYS_CHANNEL_CHANNEL2_gc;  // CCLLUT1A=#INT
    EVSYS.USERCCLLUT1B = EVSYS_CHANNEL_CHANNEL0_gc;  // CCLLUT1B=Q_OSC
    EVSYS.USERCCLLUT2A = EVSYS_CHANNEL_CHANNEL4_gc;  // CCLLUT2A=#IOR
    EVSYS.USERCCLLUT2B = EVSYS_CHANNEL_CHANNEL1_gc;  // CCLLUT2B=E_OSC
    EVSYS.USEREVOUTC = EVSYS_CHANNEL_CHANNEL5_gc;    // EVOUTC=Q_CLK
    EVSYS.USEREVOUTF = EVSYS_CHANNEL_CHANNEL3_gc;    // EVOUTF=E_CLK

    EVSYS.CHANNEL0 = EVSYS_GENERATOR_PORT1_PIN0_gc;  // CHANNEL0=PB0(Q_OSC)
    EVSYS.CHANNEL1 = EVSYS_GENERATOR_PORT1_PIN3_gc;  // CHANNEL1=PB3(E_OSC)
    EVSYS.CHANNEL2 = EVSYS_GENERATOR_CCL_LUT2_gc;    // CHANNEL2=CCLLUT2(#INT)
    EVSYS.CHANNEL3 = EVSYS_GENERATOR_CCL_LUT0_gc;    // CHANNEL3=CCLLUT0(E_CLK)
    EVSYS.CHANNEL4 = EVSYS_GENERATOR_PORT1_PIN3_gc;  // CHANNEL4=PF3(#IOR)
    EVSYS.CHANNEL5 = EVSYS_GENERATOR_CCL_LUT1_gc;    // CHANNEL5=CCLLUT1(Q_CLK)
}

static void setupCCL() {
    CCL.CTRLA = 0;  // disable CCL to configure
    // LUT2/LUT3: D FlipFlop OUT=#INT
    CCL.SEQCTRL1 = CCL_SEQSEL1_DFF_gc;       // D Flip Flop
    CCL.LUT2CTRLB = CCL_INSEL0_MASK_gc |     // IN0=MASK
                    CCL_INSEL1_EVENTA_gc;    // IN1=CCLLUT2A(#IOR)
    CCL.LUT2CTRLC = CCL_INSEL2_EVENTB_gc;    // IN2=CCLLUT2B(E_OSC)
    CCL.TRUTH2 = 0b11001100;                 // D=#IOR
    CCL.LUT2CTRLA = CCL_CLKSRC_IN2_gc |      // CLK=IN2(E_OSC)
                    CCL_ENABLE_bm;           // enable LUT2
    CCL.LUT3CTRLB = CCL_INSEL0_MASK_gc |     // IN0=MASK
                    CCL_INSEL1_MASK_gc;      // IN1=MASK
    CCL.LUT3CTRLC = CCL_INSEL2_MASK_gc;      // IN2=MASK
    CCL.TRUTH3 = 0b11111111;                 // G=H
    CCL.LUT3CTRLA = CCL_ENABLE_bm;           // enable LUT3
    CCL.INTCTRL0 = CCL_INTMODE2_FALLING_gc;  // enable falling edge LUT2(#INT)
    // LUT0: OUT=E_CLK
    CCL.LUT0CTRLB = CCL_INSEL0_EVENTA_gc |  // IN0=CCLLUT0A(#INT)
                    CCL_INSEL1_MASK_gc;     // IN1=MASK
    CCL.LUT0CTRLC = CCL_INSEL2_EVENTB_gc;   // IN2=CCLLUT0B(E_OSC)
    CCL.TRUTH0 = 0b11110101;                // OUT=(#INT=H)E_OSC+(#INT=L)H
    CCL.LUT0CTRLA = CCL_ENABLE_bm;          // enable LUT0
    // LUT1: OUT=Q_CLK
    CCL.LUT1CTRLB = CCL_INSEL0_EVENTA_gc |  // IN0=CCLLUT1A(#INT)
                    CCL_INSEL1_MASK_gc;     // IN1=MASK
    CCL.LUT1CTRLC = CCL_INSEL2_EVENTB_gc;   // IN2=CCLLUT1B(Q_OSC)
    CCL.TRUTH1 = 0b11110101;                // OUT=(#INT=H)Q_OSC+(#INT=L)H
    CCL.LUT1CTRLA = CCL_ENABLE_bm;          // enable LUT1
    // Enable CCL
    CCL.CTRLA = CCL_ENABLE_bm;
}

static void setupTimer() {
    TCA0.SPLIT.CTRLESET = TCA_SPLIT_CMD_RESET_gc |  // reset timer/counter
                          3;                        // reset both low/high
    TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm;         // SPLIT mode
    TCA0.SPLIT.LPER = CLK_PER_MPU - 1;              // CLK cycle-1
    TCA0.SPLIT.HPER = CLK_PER_MPU - 1;              // CLK cycle-1
    TCA0.SPLIT.LCMP0 = CLK_PER_MPU / 2;             // CLK cycle/2
    TCA0.SPLIT.HCMP0 = CLK_PER_MPU / 2;             // CLK cycle/2
}

static inline void raisingQ() {
    digitalWrite(Q_CLK, HIGH);
    asm volatile("nop");
}

static inline void raisingE() {
    digitalWrite(E_CLK, HIGH);
    asm volatile("nop");
}

static inline void fallingQ() {
    digitalWrite(Q_CLK, LOW);
    asm volatile("nop");
}

static inline void fallingE() {
    digitalWrite(E_CLK, LOW);
    asm volatile("nop");
}

/*
 * Restart clock oscillator from Q-H and E=H
 */
static void restartEQ() {
    fallingQ();
    fallingE();
    startOscillator();
}

static void sampleIOR() {
    // Strobe CCL.LUT2 clock to latch #IOR to #INT.
    EVSYS.STROBE = 1 << E_OSC_CHANNEL;  // Strobe E_OSC Channel
}

static void ioRequest() {
    static uint8_t data;
    const uint16_t ioAddr = Pins.ioRequestAddress();
    const bool ioWrite = Pins.ioRequestWrite();
    if (Acia.isSelected(ioAddr)) {
        if (ioWrite) {
            toggle_led();
            Acia.write(Pins.ioGetData(), ioAddr);
        } else {
            Pins.ioSetData(Acia.read(ioAddr));
        }
    } else {
        if (ioWrite) {
            data = Pins.ioGetData();
        } else {
            Pins.ioSetData(data);
        }
    }
}

ISR(CCL_CCL_vect) {
    const uint8_t flags = CCL.INTFLAGS;
    if (flags & CCL_INT2_bm) {  // check interrupt from LUT2
        CCL.INTFLAGS = CCL_INT2_bm;
        stopOscillator();
        fallingQ();
        ioRequest();
        Pins.acknowledgeIoRequest();
    } else {
        CCL.INTFLAGS = flags;  // clear all flags
    }
}

void Pins::reset(bool show) {
    assert_reset();
    negate_halt();
    startOscillator();
    delayMicroseconds(10);

    Signals::resetCycles();
    const Signals *prev = nullptr;
    stopOscillator();
    negate_reset();
    assert_halt();
    for (;;) {
        fallingQ();
        Signals &signals = cycle(prev);
        signals.debug('R');
        if (signals.halting())
            break;
        prev = &signals;
    }
    restartEQ();

    turnoff_led();
    _freeRunning = false;
    _stopRunning = false;
    if (show)
        Signals::printCycles();
}

/**
 * Advance one bus cycle.
 *
 * A bus cycle starts from Q=L and E=H and ends at Q=H and E=H.
 */
Signals &Pins::cycle(const Signals *prev) {
    // Setup data bus.
    if (valid_bus_cycle(prev) && !write_bus_cycle()) {
        _dbus.output();
    } else {
        _dbus.input();
    }
    Signals &signals = Signals::currCycle();
    signals.get();
    fallingE();
    // Cleanup data bus.
    _dbus.input();
    raisingQ();
    raisingE();

    Signals::nextCycle();
    return signals;
}

void Pins::stopRunning() {
    _stopRunning = true;
}

void Pins::loop() {
    if (_freeRunning) {
        Acia.loop();
        if (user_switch_asserted())
            stopRunning();
    }

    if (_stopRunning) {
        CCL.INTCTRL0 = CCL_INTMODE2_INTDISABLE_gc;
        _stopRunning = false;
        _freeRunning = false;
        Commands.halt(true);
    }
}

void Pins::run() {
    _freeRunning = true;
    CCL.INTCTRL0 = CCL_INTMODE2_FALLING_gc;
    turnon_led();
    negate_halt();
}

void Pins::halt(bool show) {
    Signals::resetCycles();
    const Signals *prev = nullptr;
    stopOscillator();
    assert_halt();
    for (;;) {
        fallingQ();
        Signals &signals = cycle(prev);
        signals.debug('H');
        if (signals.halting())
            break;
        prev = &signals;
    }
    restartEQ();

    turnoff_led();
    if (show)
        Signals::printCycles();
}

void Pins::setData(uint8_t data) {
    _dbus.set(data);
}

const Signals *Pins::unhalt() {
    Signals::resetCycles();
    const Signals *prev = nullptr;
    stopOscillator();
    negate_halt();
    fallingQ();
    assert_halt();
    for (;;) {
        if (is_running())
            break;
        Signals &signals = cycle(prev);
        signals.debug('U');
        fallingQ();
        prev = &signals;
    }
    return prev;
}

void Pins::execInst(const uint8_t *inst, uint8_t len, bool show) {
    execute(inst, len, nullptr, 0, false, false, show);
}

uint8_t Pins::captureReads(
        const uint8_t *inst, uint8_t len, uint8_t *capBuf, uint8_t max) {
    return execute(inst, len, capBuf, max, false, capBuf != nullptr, false);
}

uint8_t Pins::captureWrites(
        const uint8_t *inst, uint8_t len, uint8_t *capBuf, uint8_t max) {
    return execute(inst, len, capBuf, max, capBuf != nullptr, false, false);
}

uint8_t Pins::execute(const uint8_t *inst, uint8_t len, uint8_t *capBuf,
        uint8_t max, bool capWrite, bool capRead, bool show) {
    uint8_t cap = 0;
    const Signals *prev = unhalt();
    for (uint8_t i = 0; i < len;) {
        setData(inst[i]);
        Signals &signals = cycle(prev);
        if (signals.readCycle(prev)) {
            signals.debug('A' + i);
            i++;
        } else {
            signals.debug('-');
        }
        if (signals.halting())
            goto end;
        fallingQ();
        prev = &signals;
    }

    if (capWrite)
        _dbus.capture(true);
    for (;;) {
        Signals &signals = cycle(prev);
        if ((capWrite && signals.writeCycle(prev)) ||
                (capRead && signals.readCycle(prev))) {
            signals.debug('a' + cap);
            if (cap < max)
                capBuf[cap++] = signals.dbus();
        } else {
            signals.debug('-');
        }
        if (signals.halting())
            break;
        fallingQ();
        prev = &signals;
    }
    if (capWrite)
        _dbus.capture(false);
end:
    restartEQ();
    if (show)
        Signals::printCycles();
    return cap;
}

void Pins::step(bool show) {
    const Signals *prev = unhalt();
    for (;;) {
        Signals &signals = cycle(prev);
        signals.debug('S');
        if (signals.halting())
            break;
        fallingQ();
        prev = &signals;
    }
    restartEQ();
    if (show)
        Signals::printCycles();
}

void Pins::begin() {
    pinMode(RESET, OUTPUT);
    assert_reset();
    pinMode(HALT, OUTPUT);
    negate_halt();
    pinMode(NMI, OUTPUT);
    negate_nmi();
    pinMode(IRQ, OUTPUT);
    negate_irq();

    setupPORTMUX();
    setupEVSYS();
    setupCCL();
    setupTimer();

    pinMode(IOR, INPUT_PULLUP);
    pinMode(E_CLK, OUTPUT);
    pinMode(Q_CLK, OUTPUT);

    pinMode(BS, INPUT_PULLUP);
    pinMode(BA, INPUT_PULLUP);
    pinMode(LIC, INPUT_PULLUP);
    pinMode(AVMA, INPUT_PULLUP);
    pinMode(BUSY, INPUT_PULLUP);
    pinMode(RD_WR, INPUT_PULLUP);
    pinMode(RAM_E, OUTPUT);
    disable_ram();

    _dbus.begin();

    pinMode(ADR0, INPUT_PULLUP);
    pinMode(ADR1, INPUT_PULLUP);

    pinMode(USR_SW, INPUT_PULLUP);
    pinMode(USR_LED, OUTPUT);
    turnoff_led();

    Console.begin(CONSOLE_BAUD);
    cli.begin(Console);

#ifdef SPI_MAPPING
    SPI.swap(SPI_MAPPING);
#endif

    assert_halt();
    negate_reset();
    _freeRunning = false;
    _stopRunning = false;

    setIoDevice(SerialDevice::DEV_ACIA, ioBaseAddress());

    startOscillator();
}

uint8_t Pins::allocateIrq() {
    static uint8_t irq = 0;
    return irq++;
}

void Pins::assertIrq(const uint8_t irq) {
    _irq |= (1 << irq);
    if (_irq)
        assert_irq();
}

void Pins::negateIrq(const uint8_t irq) {
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

uint16_t Pins::ioRequestAddress() const {
    uint16_t addr = ioBaseAddress();
    if (digitalRead(ADR0) == HIGH)
        addr |= (1 << 0);
    if (digitalRead(ADR1) == HIGH)
        addr |= (1 << 1);
    return addr;
}

bool Pins::ioRequestWrite() const {
    return write_bus_cycle();
}

uint8_t Pins::ioGetData() {
    _dbus.capture(true);
    _dbus.input();
    const uint8_t data = busRead(DB);
    _dbus.capture(false);
    return data;
}

void Pins::ioSetData(uint8_t data) {
    _dbus.set(data);
    _dbus.output();
    asm volatile("nop");
    asm volatile("nop");
}

void Pins::acknowledgeIoRequest() {
    fallingE();
    asm volatile("nop");
    _dbus.input();
    sampleIOR();
    startOscillator();
}

int Pins::sdCardChipSelectPin() const {
    return SD_CS_PIN;
}
