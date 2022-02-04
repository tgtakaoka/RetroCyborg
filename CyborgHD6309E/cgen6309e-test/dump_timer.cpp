#include "dump_timer.h"

void DumpTimer::printText(const char *text, const char *next) {
    _console.print(text);
    if (next)
        _console.print(next);
}

void DumpTimer::printHex(uint16_t val, uint8_t digits) {
    uint16_t max = 0xF;
    for (uint8_t i = 1; val <= max && i < digits; i++) {
        _console.print('0');
        max <<= 4;
        max |= 0xF;
    }
    _console.print(val, HEX);
}

void DumpTimer::dumpTCA_CMP(
        const char *name, uint8_t n, bool CMPEN, bool CMPOV, uint16_t CMP) {
    printText(name);
    printDec(n);
    printText("=");
    printDec(CMP);
    printText(" OUT=", CMPOV ? "H" : "L");
    printText(CMPEN ? " ENABLED" : " DISABLED");
    newline();
}

void DumpTimer::dumpTCA(uint8_t n) {
    TCA_t &t = (&TCA0)[n];
    const uint8_t CTRLA = t.SINGLE.CTRLA;
    const uint8_t CTRLB = t.SINGLE.CTRLB;
    const uint8_t CTRLC = t.SINGLE.CTRLC;
    const uint8_t CTRLD = t.SINGLE.CTRLD;
    const uint16_t CNT = t.SINGLE.CNT;
    const uint16_t PER = t.SINGLE.PER;
    const bool SINGLE = (CTRLD & TCA_SINGLE_SPLITM_bm) == 0;
    printText("TCA");
    printDec(n);
    printText(":", SINGLE ? "SINGLE" : "SPLIT");
    printText(CTRLA & TCA_SINGLE_ENABLE_bm ? " ENABLED" : " DISABLED");
    const uint8_t CLKSEL =
            (CTRLA & TCA_SINGLE_CLKSEL_gm) >> TCA_SINGLE_CLKSEL_gp;
    static const uint16_t CLKDIV[] = {1, 2, 4, 8, 16, 64, 256, 1024};
    printText(" CLKSEL=1/");
    printDec(CLKDIV[CLKSEL]);
    if (SINGLE) {
        const bool ALUPD = (CTRLB & TCA_SINGLE_ALUPD_bm) != 0;
        const uint8_t WGMODE = (CTRLB & TCA_SINGLE_WGMODE_gm);
        printText("  ");
        if (ALUPD)
            printText("ALUPD ");
        static const char *const MODES[] = {
                "NORMAL",
                "FRQ",
                "(2)",
                "SINGLESLOPE",
                "(4)",
                "DSTOP",
                "DSBOTH",
                "DSBOTTOM",
        };
        printText(" WGMODE=", MODES[WGMODE]);
        printText(" CNT=");
        printDec(CNT);
        printText(" PER=");
        printDec(PER);
        newline();
        for (int i = 0; i < 3; i++) {
            const bool CMPEN = (CTRLB & (1 << (TCA_SINGLE_CMP0EN_bp + i))) != 0;
            const bool CMPOV = (CTRLC & (1 << (TCA_SINGLE_CMP0OV_bp + i))) != 0;
            const uint16_t CMP = (&t.SINGLE.CMP0)[i];
            dumpTCA_CMP("  CMP", i, CMPEN, CMPOV, CMP);
        }
    } else {
        const uint8_t HCNT = CNT >> 8;
        const uint8_t LCNT = CNT;
        const uint8_t HPER = PER >> 8;
        const uint8_t LPER = PER;
        printText("  HCNT=");
        printDec(HCNT);
        printText(" HPER=");
        printDec(HPER);
        newline();
        for (int i = 0; i < 3; i++) {
            const bool HCMPEN =
                    (CTRLB & (1 << (TCA_SPLIT_HCMP0EN_bp + i))) != 0;
            const bool HCMPOV =
                    (CTRLC & (1 << (TCA_SPLIT_HCMP0OV_bp + i))) != 0;
            const uint8_t HCMP = (&t.SPLIT.HCMP0)[i * 2];
            dumpTCA_CMP("  HCMP", i, HCMPEN, HCMPOV, HCMP);
        }
        printText("  LCNT=");
        printDec(LCNT);
        printText(" LPER=");
        printDec(LPER);
        newline();
        for (int i = 0; i < 3; i++) {
            const bool LCMPEN =
                    (CTRLB & (1 << (TCA_SPLIT_LCMP0EN_bp + i))) != 0;
            const bool LCMPOV =
                    (CTRLC & (1 << (TCA_SPLIT_LCMP0OV_bp + i))) != 0;
            const uint8_t LCMP = (&t.SPLIT.LCMP0)[i * 2];
            dumpTCA_CMP("  LCMP", i, LCMPEN, LCMPOV, LCMP);
        }
    }
}

void DumpTimer::dumpTCB(uint8_t n) {
    TCB_t &t = (&TCB0)[n];
    const uint8_t CTRLA = t.CTRLA;
    const uint8_t CTRLB = t.CTRLB;
    const uint16_t CNT = t.CNT;
    const uint16_t CCMP = t.CCMP;
    printText("TCB");
    printDec(n);
    printText(":", (CTRLA & TCB_ENABLE_bm) ? " ENABLED" : " DISABLED");
    const uint8_t CLKSEL = (CTRLA & TCB_CLKSEL_gm) >> TCB_CLKSEL_gp;
    static const char *const CLKSRC[] = {"1/1", "1/2", "CLKTCA", "(3)"};
    printText(" div=", CLKSRC[CLKSEL]);
    const uint8_t CNTMODE = CTRLB & TCB_CNTMODE_gm;
    static const char *const MODES[] = {
            "INT", "TIMEOUT", "CAPT", "FRQ", "PW", "FRQPW", "SINGLE", "PWM8"};
    printText(" CNTMODE=", MODES[CNTMODE]);
    printText("  CNT=");
    printDec(CNT);
    printText((t.STATUS & TCB_RUN_bm) ? " RUN" : " STOP");
    newline();
    if (CNTMODE == TCB_CNTMODE_PWM8_gc) {
        const uint8_t CCMPH = CCMP >> 8;
        const uint8_t CCMPL = CCMP;
        printText("  CCMPH=");
        printDec(CCMPH);
        printText(" CCMPL=");
        printDec(CCMPL);
    } else {
        printText("  CCMP=");
        printDec(CCMP);
    }
    printText(CTRLB & TCB_CCMPEN_bm ? " ENABLED" : " DISABLED");
    printText(" CCMPINIT=", (CTRLB & TCB_CCMPINIT_bm) ? "H" : "L");
    newline();
}
