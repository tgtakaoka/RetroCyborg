/*
 * FQBN: MegaCoreX:megaavr:4809:pinout=nano_every,bootloader=no_bootloader
 */

#define USE_TCA
//#define USE_TCB

#define CLK_MHZ (F_CPU / 1000000L)
#define MPU_MHZ 2
#define CLK_PER_MPU (CLK_MHZ / MPU_MHZ)

#if defined(USE_TCA) && defined(NANO_EVERY_PINOUT)
/*
 * E_OSC: TCA0-WO3-PORTD(PD3)
 * Q_OSC: TCA0-WO0-PORTD(PD0)
 * E_CLK: CCL-LUT0, EVOUTF(PF2)
 *        IN0=CCLLUT0A(CHANNEL2:#INT)
 *        IN2=CCLLUT0B(CHANNEL3:E_OSC)
 * Q_CLK: CCL-LUT1, EVOUTB(PB2)
 *        IN0=CCLLUT1A(CHANNEL2:#INT)
 *        IN1=TCA0-WO0(Q_OSC)
 * #IOR_E: CCL-LUT2
 *        IN1=CCLLUT2A(CHANNEL4:#IOR)
 *        CLK=CCLLUT2B(CHANNEL3:E_OSC)
 * #INT: SEQUNCE1-OUT
 *        SEQSEL1: DFF(D Flip FLop)
 *        D: CCL-LUT2(#IOR_E)
 *        G: 1
 *        CLK: CCL-LUT2-IN2(E_OSC)
 * CHANNEL0: CCL-LUT0(E_CLK)
 * CHANNEL1: CCL-LUT1(Q_CLK)
 * CHANNEL2: CCL-LUT2(#INT)
 * CHANNEL3: PD3(E_OSC)
 * CHANNEL4: PF3(#IOR)
 * EVOUTB: PB2(Q_CLK)
 * EVOUTD: PD2(#INT)
 * EVOUTF: PF2(E_CLK)
 */
/* Timer A, Split Single-slope PWM mode */
/* Port */
#define IOR_PIN 19                    // PF3(=PA3)
#define IOR_C_PIN 23                  // PA3(=PF3)
#define E_OSC_PINCTRL PORTD.PIN0CTRL  // PD3
#define E_OSC_DIRSET PORTD.DIRSET     // PD3 (output)
#define E_OSC_bm (1 << 3)             // PD3
#define E_OSC_CHANNEL 3               // CHANNEL3(E_OSC)
#define Q_OSC_PINCTRL PORTD.PIN0CTRL  // PD0
#define Q_OSC_DIRSET PORTD.DIRSET     // PD0 (output)
#define Q_OSC_bm (1 << 0)             // PD0
#define E_CLK_PIN 18                  // PF2(EVOUTF)
#define Q_CLK_PIN 5                   // PB2(EVOUTB)
/* debug purpose */
#define E_OSC_PIN 14               // PD3(TCA0-WO3-PORTD)
#define Q_OSC_PIN 17               // PD0(TCA0-WO0-PORTD)
#define INT_PIN 15                 // PD2(EVOUTD)
#define DEBUG_PIN 1                // PC4
#define DEBUG_OUTTGL PORTC.OUTTGL  // PC4 (toggle)
#define DEBUG_bm (1 << 4)          // PC4

static inline void toggleDebug() {
    // DEBUG_OUTTGL = DEBUG_bm;
}

static inline void startOscillator() {
    // toggleDebug();
    TCA0.SPLIT.CTRLA = 0;
    TCA0.SPLIT.LCNT = (CLK_PER_MPU / 4) * 2;  // initialize Q_OSC at 1.0pi rad.
    TCA0.SPLIT.HCNT = (CLK_PER_MPU / 4) * 3;  // initialize E_OSC at 1.5pi rad.
    TCA0.SPLIT.CTRLB = TCA_SPLIT_LCMP0EN_bm |  // enable LCMP0(Q_OSC)
                       TCA_SPLIT_HCMP0EN_bm;   // enable HCMP0(E_OSC)
    TCA0.SPLIT.CTRLC = 0;  // set L to LCMP0OV(Q_OSC) HCMP0OV(E_OSC)
    // toggleDebug();
    CCL.LUT1CTRLA = CCL_ENABLE_bm;                 // enable Q_CLK
    EVSYS.USEREVOUTB = EVSYS_CHANNEL_CHANNEL1_gc;  // enable EVOUTB(Q_CLK)
    CCL.LUT0CTRLA = CCL_ENABLE_bm;                 // enable E_CLK
    EVSYS.USEREVOUTF = EVSYS_CHANNEL_CHANNEL0_gc;  // enable EVOUTF(E_CLK)
    // toggleDebug();
    TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV1_gc |  // clock=CLK_PER/1
                       TCA_SPLIT_ENABLE_bm;        // enable TCA0(Q_OSC/E_OSC)
    // toggleDebug();
}

static inline void stopOscillator() {
    // toggleDebug();
    TCA0.SPLIT.CTRLA = 0;
    // toggleDebug();
    digitalWriteFast(Q_CLK_PIN, HIGH);
    EVSYS.USEREVOUTB = EVSYS_CHANNEL_OFF_gc;  // disable EVOUTB(Q_CLK)
    CCL.LUT1CTRLA = 0;                        // disable Q_CLK
    // toggleDebug();
    digitalWriteFast(E_CLK_PIN, HIGH);
    EVSYS.USEREVOUTF = EVSYS_CHANNEL_OFF_gc;  // disable EVOUTF(E_CLK)
    CCL.LUT0CTRLA = 0;                        // disable E_CLK
    // toggleDebug();
}

static void setupPORTMUX() {
    PORTMUX.TCAROUTEA = PORTMUX_TCA0_PORTD_gc;  // TCA0 on PD0(TCA0-WO0:Q_OSC)
                                                // PD3(TCA0-WO3:E_OSC)
    Q_OSC_PINCTRL = PORT_ISC_INTDISABLE_gc;     // Q_OSC: enable input buffer
    Q_OSC_DIRSET = Q_OSC_bm;                    // Q_OSC: output
    E_OSC_PINCTRL = PORT_ISC_INTDISABLE_gc;     // E_OSC: enable input buffer
    E_OSC_DIRSET = E_OSC_bm;                    // E_OSC: output
    PORTMUX.EVSYSROUTEA &= ~PORTMUX_EVOUT1_bm;  // EVOUTB on PB2(Q_CLK)
    PORTMUX.EVSYSROUTEA &= ~PORTMUX_EVOUT3_bm;  // EVOUTD on PD2(#INT)
    PORTMUX.EVSYSROUTEA &= ~PORTMUX_EVOUT5_bm;  // EVOUTF on PF2(E_CLK)
}

static void setupEVSYS() {
    EVSYS.USERCCLLUT0A = EVSYS_CHANNEL_CHANNEL2_gc;  // CCLLUT0A=#INT
    EVSYS.USERCCLLUT0B = EVSYS_CHANNEL_CHANNEL3_gc;  // CCLLUT0B=E_OSC
    EVSYS.USERCCLLUT1A = EVSYS_CHANNEL_CHANNEL2_gc;  // CCLLUT1A=#INT
    EVSYS.USERCCLLUT2A = EVSYS_CHANNEL_CHANNEL4_gc;  // CCLLUT2A=#IOR
    EVSYS.USERCCLLUT2B = EVSYS_CHANNEL_CHANNEL3_gc;  // CCLLUT2B=E_OSC
    EVSYS.USEREVOUTB = EVSYS_CHANNEL_CHANNEL1_gc;    // EVOUTB=Q_CLK
    EVSYS.USEREVOUTD = EVSYS_CHANNEL_CHANNEL2_gc;    // EVOUTD=#INT
    EVSYS.USEREVOUTF = EVSYS_CHANNEL_CHANNEL0_gc;    // EVOUTF=E_CLK

    EVSYS.CHANNEL0 = EVSYS_GENERATOR_CCL_LUT0_gc;    // CHANNEL0=CCLLUT0(E_CLK)
    EVSYS.CHANNEL1 = EVSYS_GENERATOR_CCL_LUT1_gc;    // CHANNEL1=CCLLUT1(Q_CLK)
    EVSYS.CHANNEL2 = EVSYS_GENERATOR_CCL_LUT2_gc;    // CHANNEL2=CCLLUT2(#INT)
    EVSYS.CHANNEL3 = EVSYS_GENERATOR_PORT1_PIN3_gc;  // CHANNEL3=PD3(E_OSC)
    EVSYS.CHANNEL4 = EVSYS_GENERATOR_PORT1_PIN3_gc;  // CHANNEL4=PF3(#IOR)
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
    CCL.LUT1CTRLB = CCL_INSEL0_TCA0_gc |   // IN0=TCA0-WO0(Q_OSC)
                    CCL_INSEL1_EVENTA_gc;  // IN1=CCLLUT1A(#INT)
    CCL.LUT1CTRLC = CCL_INSEL2_MASK_gc;    // IN2=MASK
    CCL.TRUTH1 = 0b10111011;               // OUT=(#INT=H)Q_OSC+(#INT=L)H
    CCL.LUT1CTRLA = CCL_ENABLE_bm;         // enable LUT1
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
#endif

#if defined(USE_TCB) && defined(NANO_EVERY_PINOUT)
/*
 * E_OSC: TCB2-WO(PC0), EVOUTE(PE2)
 * Q_OSC: TCB1-WO-ALT(PF5)
 * E_CLK: CCL-LUT0, EVOUTF(PF2)
 *        IN0=CCLLUT0A(CHANNEL2:#INT)
 *        IN2=TCB2-WO(E_OSC)
 * Q_CLK: CCL-LUT1, EVOUTC(PC2)
 *        IN0=CCLLUT1A(CHANNEL2:#INT)
 *        IN1=TCB1-WO(Q_OSC)
 * #IOR_E: CCL-LUT2
 *        IN1=CCLLUT2A(CHANNEL4:#IOR)
 *        CLK=CCLLUT2B(E_OSC)
 * #INT: SEQUNCE1-OUT
 *        SEQSEL1: DFF(D Flip FLop)
 *        D: CCL-LUT2(#IOR_E)
 *        G: 1
 *        CLK: CCL-LUT2-IN2(E_OSC)
 * CHANNEL0: CCL-LUT0(E_CLK)
 * CHANNEL1: CCL-LUT1(Q_CLK)
 * CHANNEL2: CCL-LUT2(#INT)
 * CHANNEL3: PE2(E_OSC)
 * CHANNEL4: PF3(#IOR)
 * EVOUTB: PB2(Q_CLK)
 * EVOUTD: PD2(#INT)
 * EVOUTE: PE2(E_OSC)
 * EVOUTF: PF2(E_CLK)
 */
/* Timer B, 8-bit PWM mode */
#define E_TCB TCB2
#define Q_TCB TCB1
/* Port */
#define IOR_PIN 19                    // PF3=(PA3)
#define IOR_C_PIN 23                  // PA3=(PF3)
#define E_OSC_PINCTRL PORTE.PIN0CTRL  // PE2(D13)
#define E_OSC_DIRSET PORTE.DIRSET     // PE2 (output)
#define E_OSC_bm (1 << 2)             // PE2
#define E_OSC_CHANNEL 3               // CHANNEL3(E_OSC)
#define Q_OSC_PINCTRL PORTF.PIN0CTRL  // PF5(D3)
#define Q_OSC_DIRSET PORTF.DIRSET     // PF5 (output)
#define Q_OSC_bm (1 << 5)             // PF5
#define E_CLK_PIN 18                  // PF2(EVOUTF)
#define Q_CLK_PIN 5                   // PB2(EVOUTB)
/* debug purpose */
#define E_OSC_PIN 13               // PE2(EVOUTE)
#define Q_OSC_PIN 3                // PF5:TCB1-WO-ALT1
#define INT_PIN 15                 // PD2(EVOUTD)
#define DEBUG_PIN 1                // PC4
#define DEBUG_OUTTGL PORTC.OUTTGL  // PC4 (toggle)
#define DEBUG_bm (1 << 4)          // PC4

static inline void toggleDebug() {
    // DEBUG_OUTTGL = DEBUG_bm;
}

static inline void startOscillator() {
    // toggleDebug();
    Q_TCB.CTRLA = 0;
    E_TCB.CTRLA = 0;
    Q_TCB.CNT = 1;
    E_TCB.CNT = 1;
    Q_TCB.CTRLB = TCB_CNTMODE_PWM8_gc | TCB_CCMPEN_bm;
    E_TCB.CTRLB = TCB_CNTMODE_PWM8_gc | TCB_CCMPEN_bm;
    // toggleDebug();
    CCL.LUT1CTRLA = CCL_ENABLE_bm;                 // enable Q_CLK
    EVSYS.USEREVOUTB = EVSYS_CHANNEL_CHANNEL1_gc;  // enable EVOUTB(Q_CLK)
    CCL.LUT0CTRLA = CCL_ENABLE_bm;                 // enable E_CLK
    EVSYS.USEREVOUTF = EVSYS_CHANNEL_CHANNEL0_gc;  // enable EVOUTF(E_CLK)
    // toggleDebug();
    Q_TCB.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm;
    asm volatile("nop");
    E_TCB.CTRLA = TCB_CLKSEL_CLKDIV1_gc | TCB_ENABLE_bm;
    // toggleDebug();
}

static inline void stopOscillator() {
    // toggleDebug();
    Q_TCB.CTRLB = 0;  // disable Q_OSC
    E_TCB.CTRLB = 0;  // disable E_OSC
    // toggleDebug();
    digitalWriteFast(Q_CLK_PIN, HIGH);
    EVSYS.USEREVOUTB = EVSYS_CHANNEL_OFF_gc;  // disable EVOUTB(Q_CLK)
    CCL.LUT1CTRLA = 0;                        // disable Q_CLK
    // toggleDebug();
    digitalWriteFast(E_CLK_PIN, HIGH);
    EVSYS.USEREVOUTF = EVSYS_CHANNEL_OFF_gc;  // disable EVOUTF(E_CLK)
    CCL.LUT0CTRLA = 0;                        // disable E_CLK
    // toggleDebug();
}

static void setupPORTMUX() {
    PORTMUX.TCBROUTEA |= PORTMUX_TCB1_bm;       // TCB1 on PF5(Q_OSC)
    Q_OSC_PINCTRL = PORT_ISC_INTDISABLE_gc;     // Q_OSC: enable input buffer
    Q_OSC_DIRSET = Q_OSC_bm;                    // Q_OSC: output
    PORTMUX.TCBROUTEA &= ~PORTMUX_TCB2_bm;      // TCB2 on PC0(E_OSC)
    E_OSC_PINCTRL = PORT_ISC_INTDISABLE_gc;     // E_OSC: enable input buffer
    E_OSC_DIRSET = E_OSC_bm;                    // E_OSC: output
    PORTMUX.EVSYSROUTEA &= ~PORTMUX_EVOUT1_bm;  // EVOUTB on PB2(Q_CLK)
    PORTMUX.EVSYSROUTEA &= ~PORTMUX_EVOUT3_bm;  // EVOUTD on PD2(#INT)
    PORTMUX.EVSYSROUTEA &= ~PORTMUX_EVOUT4_bm;  // EVOUTE on PE2(E_OSC)
    PORTMUX.EVSYSROUTEA &= ~PORTMUX_EVOUT5_bm;  // EVOUTF on PF2(E_CLK)
}

static void setupEVSYS() {
    EVSYS.USERCCLLUT0A = EVSYS_CHANNEL_CHANNEL2_gc;  // CCLLUT0A=#INT
    EVSYS.USERCCLLUT1A = EVSYS_CHANNEL_CHANNEL2_gc;  // CCLLUT1A=#INT
    EVSYS.USERCCLLUT2A = EVSYS_CHANNEL_CHANNEL4_gc;  // CCLLUT2A=#IOR
    EVSYS.USERCCLLUT2B = EVSYS_CHANNEL_CHANNEL3_gc;  // CCLLUT2B=E_OSC
    EVSYS.USEREVOUTB = EVSYS_CHANNEL_CHANNEL1_gc;    // EVOUTB=Q_CLK
    EVSYS.USEREVOUTD = EVSYS_CHANNEL_CHANNEL2_gc;    // EVOUTD=#INT
    EVSYS.USEREVOUTE = EVSYS_CHANNEL_CHANNEL3_gc;    // EVOUTE=E_OSC
    EVSYS.USEREVOUTF = EVSYS_CHANNEL_CHANNEL0_gc;    // EVOUTF=E_CLK

    EVSYS.CHANNEL0 = EVSYS_GENERATOR_CCL_LUT0_gc;    // CHANNEL0=CCLLUT0(E_CLK)
    EVSYS.CHANNEL1 = EVSYS_GENERATOR_CCL_LUT1_gc;    // CHANNEL1=CCLLUT1(Q_CLK)
    EVSYS.CHANNEL2 = EVSYS_GENERATOR_CCL_LUT2_gc;    // CHANNEL2=CCLLUT2(#INT)
    EVSYS.CHANNEL3 = EVSYS_GENERATOR_PORT0_PIN0_gc;  // CHANNEL3=PC0(E_OSC)
    EVSYS.CHANNEL4 = EVSYS_GENERATOR_PORT1_PIN3_gc;  // CHANNEL4=PF3(#IOR)
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
    CCL.LUT0CTRLC = CCL_INSEL2_TCB2_gc;     // IN2=TCB2-WO(E_OSC)
    CCL.TRUTH0 = 0b11110101;                // OUT=(#INT=H)E_OSC+(#INT=L)H
    CCL.LUT0CTRLA = CCL_ENABLE_bm;          // enable LUT0
    // LUT1: OUT=Q_CLK
    CCL.LUT1CTRLB = CCL_INSEL0_EVENTA_gc |  // IN0=CCLLUT1A(#INT)
                    CCL_INSEL1_TCB1_gc;     // IN1=TCB1-WO(Q_OSC)
    CCL.LUT1CTRLC = CCL_INSEL2_MASK_gc;     // IN2=MASK
    CCL.TRUTH1 = 0b11011101;                // OUT=(#INT=H)Q_OSC+(#INT=L)H
    CCL.LUT1CTRLA = CCL_OUTEN_bm |          // OUT=PC6(Q_CLK)
                    CCL_ENABLE_bm;          // enable LUT1
    // Enable CCL
    CCL.CTRLA = CCL_ENABLE_bm;
}

static void setupTimer() {
    Q_TCB.CTRLA = 0;
    Q_TCB.CCMPL = CLK_PER_MPU - 1;  // CLK cycle-1
    Q_TCB.CCMPH = CLK_PER_MPU / 2;  // CLK cycle/2

    E_TCB.CTRLA = 0;
    E_TCB.CCMPL = CLK_PER_MPU - 1;  // CLK cycle-1
    E_TCB.CCMPH = CLK_PER_MPU / 2;  // CLK cycle/2
}
#endif

static inline void fallingQ() {
    digitalWriteFast(Q_CLK_PIN, LOW);
    asm volatile("nop");
}

static inline void fallingE() {
    digitalWriteFast(E_CLK_PIN, LOW);
    asm volatile("nop");
}

static inline void sampleIOR() {
    // Strobe CCL.LUT2 clock to latch #IOR to #INT.
    EVSYS.STROBE = (1 << E_OSC_CHANNEL);  // Strobe E_OSC Channel
}

static void ioRequest() {
    stopOscillator();
    toggleDebug();
    delayMicroseconds(1);
    fallingQ();
    toggleDebug();
    // Do something
    delayMicroseconds(1);
    // Leave interrupt
    fallingE();
    toggleDebug();

    digitalWriteFast(IOR_C_PIN, HIGH);

    sampleIOR();
    startOscillator();
}

ISR(CCL_CCL_vect) {
    toggleDebug();
    const uint8_t flags = CCL.INTFLAGS;
    if (flags & CCL_INT2_bm) {  // check interrupt from LUT2
        CCL.INTFLAGS = CCL_INT2_bm;
        ioRequest();
    } else {
        CCL.INTFLAGS = flags;  // clear all flags
    }
    toggleDebug();
}

#include "dump_timer.h"

void setup() {
    pinMode(DEBUG_PIN, OUTPUT);
    Serial.begin(115200);

    setupPORTMUX();
    setupEVSYS();
    setupCCL();
    setupTimer();

    digitalWriteFast(IOR_C_PIN, HIGH);
    pinMode(IOR_C_PIN, OUTPUT);
    pinMode(IOR_PIN, INPUT_PULLUP);

    pinMode(E_CLK_PIN, OUTPUT);
    pinMode(Q_CLK_PIN, OUTPUT);

    pinMode(E_OSC_PIN, OUTPUT);
    pinMode(Q_OSC_PIN, OUTPUT);
    pinMode(INT_PIN, OUTPUT);

    startOscillator();
}
void loop() {
    delay(500);
    digitalWriteFast(IOR_C_PIN, LOW);
}
