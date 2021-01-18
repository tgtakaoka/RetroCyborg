/*
 * Clock generator test for HD6303 running on Teensy 3.5
 */

#define LED_PIN 13   // PTC05
#define CLK_PIN 29   // PTB18/FTM2_CH0
#define CLK2_PIN 30  // PTB19/FTM2_CH1

class FTM {
public:
    FTM(const char *name, volatile uint32_t &sc, const uint8_t channel)
        : _base(&sc), _channel(channel), _name(name) {}
    const char *name() const { return _name; }
    bool hasChannel(const uint8_t channel) const { return channel < _channel; }

    volatile uint32_t &SC() const { return _base[0x00 / 4]; }
    volatile uint32_t &CNT() const { return _base[0x04 / 4]; }
    volatile uint32_t &MOD() const { return _base[0x08 / 4]; }
    volatile uint32_t &CSC(const uint8_t channel) const {
        return hasChannel(channel) ? _base[(0x0c + 8 * channel) / 4] : _dummy;
    }
    volatile uint32_t &CV(const uint8_t channel) const {
        return hasChannel(channel) ? _base[(0x10 + 8 * channel) / 4] : _dummy;
    }
    volatile uint32_t &CNTIN() const { return _base[0x4c / 4]; }
    volatile uint32_t &STATUS() const { return _base[0x50 / 4]; }
    volatile uint32_t &MODE() const { return _base[0x54 / 4]; }
    volatile uint32_t &SYNC() const { return _base[0x58 / 4]; }
    volatile uint32_t &OUTINIT() const { return _base[0x5c / 4]; }
    volatile uint32_t &OUTMASK() const { return _base[0x60 / 4]; }
    volatile uint32_t &COMBINE() const { return _base[0x64 / 4]; }
    volatile uint32_t &DEADTIME() const { return _base[0x68 / 4]; }
    volatile uint32_t &EXTTRIG() const { return _base[0x6c / 4]; }
    volatile uint32_t &POL() const { return _base[0x70 / 4]; }
    volatile uint32_t &FMS() const { return _base[0x74 / 4]; }
    volatile uint32_t &FILTER() const { return _base[0x78 / 4]; }
    volatile uint32_t &FLTCTRL() const { return _base[0x7c / 4]; }
    volatile uint32_t &QDCTRL() const { return _base[0x80 / 4]; }
    volatile uint32_t &CONF() const { return _base[0x84 / 4]; }
    volatile uint32_t &FLTPOL() const { return _base[0x88 / 4]; }
    volatile uint32_t &SYNCONF() const { return _base[0x8c / 4]; }
    volatile uint32_t &INVCTRL() const { return _base[0x90 / 4]; }
    volatile uint32_t &SWOCTRL() const { return _base[0x94 / 4]; }
    volatile uint32_t &PWMLOAD() const { return _base[0x98 / 4]; }

private:
    volatile uint32_t *const _base;
    const uint8_t _channel;
    const char *const _name;
    mutable volatile uint32_t _dummy;
};

const FTM FTM0("FTM0", FTM0_SC, 8);
const FTM FTM1("FTM1", FTM1_SC, 2);
const FTM FTM2("FTM2", FTM2_SC, 2);
const FTM FTM3("FTM3", FTM3_SC, 8);

static void newline() {
    Serial.println();
}

static void printText(const char *text) {
    Serial.print(text);
}

void printDec(uint32_t val) {
    Serial.print(val, DEC);
}

void printHex(uint32_t val, uint8_t digits) {
    uint32_t max = 0xF;
    for (uint8_t i = 1; val <= max && i < digits; i++) {
        Serial.print('0');
        max <<= 4;
        max |= 0xF;
    }
    Serial.print(val, HEX);
}

void printHex(uint32_t val) {
    printHex(val, sizeof(val) * 2);
}

void printHex(uint16_t val) {
    printHex(val, sizeof(val) * 2);
}

void printHex(uint8_t val) {
    printHex(val, sizeof(val) * 2);
}

static void dumpFTM_SC(const uint8_t SC) {
    static const char *const CLKS[] = {
            "disabled", "system", "fixed freq", "external"};
    printText(" CLKS=");
    printText(CLKS[(SC & FTM_SC_CLKS_MASK) >> 3]);
    printText(" PS=1/");
    printDec(1 << (SC & FTM_SC_PS_MASK));
    if (SC & FTM_SC_CPWMS)
        printText(" CPWMS");
    if (SC & FTM_SC_TOIE)
        printText(" TOIE");
    if (SC & FTM_SC_TOF)
        printText(" TOF");
}

static void dumpFTM_mode(uint8_t CSC, bool DECAPEN, bool COMBINE, bool CPWMS) {
    static const char *const edge[] = {"disabled", "rising", "falling", "both"};
    static const char *const output[] = {"disabled", "toggle", "clear", "set"};
    static const char *const match[] = {"disabled", "set", "clear", "set"};
    static const char *const combine[] = {
            "disabled", "set-clear", "clear-set", "set-clear"};
    const uint8_t EDGE = CSC & (FTM_CSC_ELSB | FTM_CSC_ELSA);
    const uint8_t LEVEL = EDGE;
    const uint8_t MODE = CSC & (FTM_CSC_MSB | FTM_CSC_MSA);
    if (DECAPEN) {
        printText(" DECAPEN ");
        printText(MODE & FTM_CSC_MSA ? "continuous " : "one-shot ");
        printText(" EDGE=");
        printText(edge[EDGE >> 2]);
    } else if (COMBINE) {
        printText(" COMBINE");
        printText(" MATCH=");
        printText(combine[EDGE >> 2]);
    } else if (CPWMS) {
        printText(" CPWMS");
        printText(" MATCH=");
        printText(match[EDGE >> 2]);
    } else {
        switch (MODE) {
        case 0:
            printText(" CAPTURE");
            printText(" EDGE=");
            printText(edge[EDGE >> 2]);
            break;
        case FTM_CSC_MSA:
            printText(" COMPARE");
            printText(" OUTPUT=");
            printText(output[LEVEL >> 2]);
            break;
        default:
            printText(" PWM");
            printText(" MATCH=");
            printText(match[EDGE >> 2]);
            break;
        }
    }
    if (CSC & FTM_CSC_DMA)
        printText(" DMA");
    if (CSC & FTM_CSC_CHIE)
        printText(" CHIE");
    if (CSC & FTM_CSC_CHF)
        printText(" CHF");
}

static void dumpFTM(const FTM &ftm) {
    printText(ftm.name());
    newline();
    printText("  SC:");
    dumpFTM_SC(ftm.SC());
    const uint8_t DTPS = ftm.DEADTIME() >> 6;
    printText(" DTPS=1/");
    printText(DTPS < 2 ? "1" : (DTPS == 2 ? "4" : "16"));
    printText(" DTVAL=");
    printDec(ftm.DEADTIME() & FTM_DEADTIME_DTVAL_MASK);
    newline();
    printText("  CNTIN=");
    printDec(ftm.CNTIN());
    printText(" MOD=");
    printDec(ftm.MOD());
    printText(" CNT=");
    printDec(ftm.CNT());
    newline();
    for (uint8_t channel = 0; channel < 8; channel++) {
        if (ftm.hasChannel(channel)) {
            printText("  C");
            printDec(channel);
            printText(": V=");
            printDec(ftm.CV(channel));
            printText(" INIT=");
            printText((ftm.OUTINIT() >> channel) & 1 ? "1" : "0");
            printText(" MASK=");
            printText((ftm.OUTMASK() >> channel) & 1 ? "1" : "0");
            const uint8_t COMBINE = (ftm.COMBINE() >> (channel / 2) * 8) & 0xFF;
            dumpFTM_mode(ftm.CSC(channel), COMBINE & FTM_COMBINE_DECAPEN0,
                    COMBINE & FTM_COMBINE_COMBINE0, ftm.SC() & FTM_SC_CPWMS);
            if (COMBINE & FTM_COMBINE_FAULTEN0)
                printText(" FAULTEN");
            if (COMBINE & FTM_COMBINE_SYNCEN0)
                printText(" SYNCEN");
            if (COMBINE & FTM_COMBINE_DTEN0)
                printText(" DTEN");
            if (COMBINE & FTM_COMBINE_DECAP0)
                printText(" DECAP");
            if (COMBINE & FTM_COMBINE_COMP0)
                printText(" COMP");
            /*
            const uint16_t SWOCTRL = ftm.SWOCTRL();
            if (SWOCTRL & (1 << channel)) {
                printText(" CHOC");
                printText(" CHOCV=");
                printText(SWOCTRL & (1 << (channel + 8)) ? "1" : "0");
            }
            */
            newline();
        }
    }
    printText("  PWMLOAD=");
    printHex((uint16_t)ftm.PWMLOAD());
    printText(" STATUS=");
    printHex((uint8_t)ftm.STATUS());
    printText(" POL=");
    printHex((uint8_t)ftm.POL());
    printText(" MODE=");
    printHex((uint8_t)ftm.MODE());
    printText(" SYNC=");
    printHex((uint8_t)ftm.SYNC());
    printText(" SYNCONF=");
    printHex(ftm.SYNCONF(), 6);
    newline();
}

const FTM &CLK = FTM2;

//#define TWO_PHASE

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

#define SYSTEM_MHZ 120
#define CLK_MHZ 4
#define CLK_MOD ((SYSTEM_MHZ / CLK_MHZ) / 2 - 1)
#define CLK_CV ((SYSTEM_MHZ / CLK_MHZ) / 4)
    // System Clock/1 120MHz
    CLK.SC() = FTM_SC_CLKS(0);

#ifdef TWO_PHASE
    CLK.MODE() = FTM_MODE_WPDIS | FTM_MODE_FTMEN;
    // FTM2_CH0/1: combine PWM mode (QUADEN=0 DECAPEN=0 COMBINE=1 CPWMS=0
    // MSB=1).
    CLK.COMBINE() = FTM_COMBINE_COMBINE0 | FTM_COMBINE_COMP0 |
                     FTM_COMBINE_SYNCEN0 | FTM_COMBINE_DTEN0;
    CLK.SYNCONF() = FTM_SYNCONF_SYNCMODE | FTM_SYNCONF_SWWRBUF |
                     FTM_SYNCONF_CNTINC | FTM_SYNCONF_SWSOC | FTM_SYNCONF_SWOC;
    CLK.DEADTIME() = FTM_DEADTIME_DTPS(0) | FTM_DEADTIME_DTVAL(1);
    CLK.CV(0) = CLK_CV;  // width=(CV(n+1)-CV(n))
    CLK.CV(1) = CLK_MOD;
    CLK.CSC(0) = FTM_CSC_MSB | FTM_CSC_ELSB;
    CLK.CSC(0) = FTM_CSC_MSB | FTM_CSC_ELSB;
    CLK.PWMLOAD() = FTM_PWMLOAD_CH0SEL | FTM_PWMLOAD_CH1SEL;
    CLK.CNTIN() = 0;
    CLK.CNT() = 0;
    CLK.MOD() = CLK_MOD;  // period=(MOD-CNTIN+1)
    CLK.SYNC() = FTM_SYNC_SWSYNC;
    CLK.SC() = FTM_SC_CLKS(1) | FTM_SC_PS(0);
    PORTB_PCR18 = PORT_PCR_MUX(3);
    PORTB_PCR19 = PORT_PCR_MUX(3);
#else
    CLK.MODE() = FTM_MODE_WPDIS;
    // FTM2_CH0: PWM mode (QUADEN=0 DECAPEN=0 COMBINE=0 CPWMS=0 MSB=1).
    CLK.COMBINE() = 0;
    CLK.CV(0) = CLK_CV;  // width=(CV-CNTIN)
    CLK.CSC(0) = FTM_CSC_MSB | FTM_CSC_ELSB;
    CLK.CNTIN() = 0;
    CLK.CNT() = 0;
    CLK.MOD() = CLK_MOD;  // period=(MOD-CNTIN+1)
    CLK.SC() = FTM_SC_CLKS(1) | FTM_SC_PS(0);
    PORTB_PCR18 = PORT_PCR_MUX(3);
#endif
}

void loop() {
    digitalWriteFast(LED_PIN, LOW);
#ifdef TWO_PHASE
    CLK.CV(0) = CLK_CV;
    CLK.CV(1) = CLK_CV;
    CLK.PWMLOAD() |= FTM_PWMLOAD_LDOK;
#else
    CLK.CV(0) = 0;
#endif
    if (Serial.dtr()) {
        printText("LED OFF");
        newline();
        dumpFTM(FTM2);
    }
    delay(1000);

    digitalWriteFast(LED_PIN, HIGH);
#ifdef TWO_PHASE
    CLK.CV(0) = CLK_CV;
    CLK.CV(1) = CLK_MOD;
    CLK.PWMLOAD() |= FTM_PWMLOAD_LDOK;
#else
    CLK.CV(0) = CLK_CV;
#endif
    if (Serial.dtr()) {
        printText("LED ON");
        newline();
        dumpFTM(FTM2);
    }
    delay(1000);
}
