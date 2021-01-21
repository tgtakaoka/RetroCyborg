/*
 * FQBN: MegaCoreX:megaavr:4809:pinout=nano_every,bootloader=no_bootloader
 */

#define PER_MHZ (F_CPU / 1000000L)
#define CLK_MHZ 8  // 4MHz or 8MHz

#define LED_PIN 13   // PE2
#define CLK_PIN 3    // PF5
#define IORQ_PIN 2   // PA0
#define MEMRQ_PIN 7  // PA1
#define WAIT_PIN 19  // PA3

static void dumpSingleTCA(
        const char *name, int n, bool enabled, bool out, uint16_t cmp) {
    Serial.print(name);
    Serial.print(n);
    Serial.print(": ");
    Serial.print(enabled ? "enabled" : "disabled");
    Serial.print(" out=");
    Serial.print(out ? "H" : "L");
    Serial.print(" cmp=");
    Serial.println(cmp, HEX);
}

static void dumpSplitTCA(
        const char *name, int n, bool enabled, bool out, uint8_t cmp) {
    Serial.print(name);
    Serial.print(n);
    Serial.print("=");
    Serial.print(cmp, HEX);
    Serial.print(enabled ? " enabled" : " disabled");
    Serial.print(" out=");
    Serial.println(out ? "H" : "L");
}

static void dumpTCA(uint8_t n) {
    Serial.print("TCA");
    Serial.print(n);
    Serial.print(':');
    TCA_t &t = (&TCA0)[n];
    const bool singleMode = (t.SINGLE.CTRLA & TCA_SINGLE_SPLITM_bm) == 0;
    Serial.print(singleMode ? "single" : "split");
    Serial.print(
            (t.SINGLE.CTRLA & TCA_SINGLE_ENABLE_bm) ? " enabled" : " disabled");
    const uint8_t clksel =
            (t.SINGLE.CTRLA & TCA_SINGLE_CLKSEL_gm) >> TCA_SINGLE_CLKSEL_gp;
    const uint16_t div =
            (clksel < 5) ? (1 << clksel) : (1 << (clksel + clksel - 4));
    Serial.print(" div=1/");
    Serial.println(div, DEC);
    if (singleMode) {
        const bool autoLockUpdate = (t.SINGLE.CTRLB & TCA_SINGLE_ALUPD_bm) != 0;
        const uint8_t wgMode = (t.SINGLE.CTRLB & TCA_SINGLE_WGMODE_gm);
        Serial.print("  ");
        if (autoLockUpdate)
            Serial.print("Auto-Lock Update ");
        static const char *const modes[] = {
                "NORMAL",
                "FRQ",
                "(2)",
                "SINGLESLOPE",
                "(4)",
                "DSTOP",
                "DSBOTH",
                "DSBOTTOM",
        };
        Serial.print("mode=");
        Serial.println(modes[wgMode]);
        Serial.print(" CNT=");
        Serial.print(t.SPLIT.HCNT, HEX);
        Serial.print(" PER=");
        Serial.println(t.SPLIT.HPER, HEX);
        for (int i = 0; i < 3; i++) {
            const bool enabled = (t.SINGLE.CTRLB & (1 << (4 + i))) != 0;
            const bool out = (t.SINGLE.CTRLC & (1 << (4 + i))) != 0;
            const uint8_t cmp = *(&t.SINGLE.CMP0 + (i * 2));
            dumpSingleTCA("    CMP", i, enabled, out, cmp);
        }
    } else {
        Serial.print("  HCNT=");
        Serial.print(t.SPLIT.HCNT, HEX);
        Serial.print(" HPER=");
        Serial.println(t.SPLIT.HPER, HEX);
        for (int i = 0; i < 3; i++) {
            const bool enabled = (t.SPLIT.CTRLB & (1 << (4 + i))) != 0;
            const bool out = (t.SPLIT.CTRLC & (1 << (4 + i))) != 0;
            const uint8_t cmp = *(&t.SPLIT.HCMP0 + (i * 2));
            dumpSplitTCA("    HCMP", i, enabled, out, cmp);
        }
        Serial.print("  LCNT=");
        Serial.print(t.SPLIT.LCNT, HEX);
        Serial.print(" LPER=");
        Serial.println(t.SPLIT.LPER, HEX);
        for (int i = 0; i < 3; i++) {
            const bool enabled = (t.SPLIT.CTRLB & (1 << i)) != 0;
            const bool out = (t.SPLIT.CTRLC & (1 << i)) != 0;
            const uint8_t cmp = *(&t.SPLIT.LCMP0 + (i * 2));
            dumpSplitTCA("    LCMP", i, enabled, out, cmp);
        }
    }
}

static void dumpTCB(uint8_t n) {
    Serial.print("TCB");
    Serial.print(n);
    Serial.print(':');
    TCB_t &t = (&TCB0)[n];
    Serial.print((t.CTRLA & TCB_ENABLE_bm) ? " enabled" : " disabled");
    static const char *const clksel[] = {"1/1", "1/2", "clk_tca", "(3)"};
    const char *div = clksel[(t.CTRLA & TCB_CLKSEL_gm) >> TCB_CLKSEL_gp];
    Serial.print(" div=");
    Serial.print(div);
    const uint8_t cntMode = t.CTRLB & TCB_CNTMODE_gm;
    static const char *const cntModes[] = {
            "INT", "TIMEOUT", "CAPT", "FRQ", "PW", "FRQPW", "SINGLE", "PWM8"};
    Serial.print(" mode=");
    Serial.println(cntModes[cntMode]);
    Serial.print("  CNT=");
    Serial.print(t.CNT, HEX);
    Serial.println((t.STATUS & TCB_RUN_bm) ? " run" : " stop");
    Serial.print("  CCMP=");
    Serial.print(t.CCMP, HEX);
    Serial.print((t.CTRLB & TCB_CCMPEN_bm) ? " enabled" : " disabled");
    Serial.print(" init=");
    Serial.println((t.CTRLB & TCB_CCMPINIT_bm) ? "H" : "L");
}

static void dumpTimers() {
    dumpTCA(0);
    for (uint8_t n = 0; n < 4; n++)
        dumpTCB(n);
}

TCB_t &CLK = TCB1;

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    PORTMUX.TCBROUTEA |= PORTMUX_TCB1_bm;  // TCB1 on PF5
    pinMode(CLK_PIN, OUTPUT);
    digitalWriteFast(CLK_PIN, LOW);
    CLK.CTRLA = TCB_CLKSEL_CLKDIV1_gc; /* F_CPU/1 */
    CLK.CTRLB = TCB_CNTMODE_PWM8_gc | TCB_CCMPEN_bm;
    CLK.CCMPL = PER_MHZ / CLK_MHZ - 1;  // CLK cycle-1
    CLK.CCMPH = PER_MHZ / CLK_MHZ / 2;  // CLK cycle/2
    CLK.CTRLA |= TCB_ENABLE_bm;

    PORTMUX.CCLROUTEA &= ~PORTMUX_LUT0_bm;  // CCL LUT0 on PA[3]
    pinMode(IORQ_PIN, INPUT_PULLUP);
    pinMode(MEMRQ_PIN, INPUT_PULLUP);
    pinMode(WAIT_PIN, OUTPUT);

    dumpTimers();
}

void loop() {
    digitalWriteFast(LED_PIN, HIGH);
    digitalWriteFast(WAIT_PIN, LOW);
    CLK.CCMPH = 0;  // CLK off=L
    delay(1000);
    digitalWriteFast(LED_PIN, LOW);
    digitalWriteFast(WAIT_PIN, HIGH);
    CLK.CCMPH = PER_MHZ / CLK_MHZ / 2;  // CLK on
    delay(1000);
    dumpTimers();
}
