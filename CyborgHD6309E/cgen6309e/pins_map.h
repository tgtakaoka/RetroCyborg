/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __PINS_MAP_H__
#define __PINS_MAP_H__

#include <avr/io.h>

// 0xFFC0 ~ 0xFFC7
#define IO_ADR_BASE 0xFFC0

/* Bit pattern of ADRn of IO_ADR_BASE */
#define IOA(n) (((IO_ADR_BASE >> (n)) & 1) << ADR##n)
/* Bit position of ADRn */
#define BVA(n) _BV(ADR##n)

/* megaTinyCore + Onboard Atmel mEDBG (UNO WiFi Rev2) */
/* using microUPDI */

/**
 * Arduino IDE settings
 * FQBN: megaTinyCore:megaavr:atxy2:chip=412,clock=20internal,millis=disabled
 * Core: https://github.com/SpenceKonde/megaTinyCore Board: ATtiny412/402/212/202
 * Chip: ATtiny412
 * Clock speed: 20MHz
 * millis/micros: Disabled
 * Programmer: Atmel mEDBG (ATmega32u4)
 */
#if defined(__AVR_ATtiny412__) || defined(__AVR_ATtiny212__)

// PA6=74688Q 74688G=7430Q
#define IOR_PORT A
#define IOR_PIN 6

#define CLK_E_PORT A
#define CLK_E_PIN 7

#define CLK_Q_PORT A
#define CLK_Q_PIN 1

#define INT_PORT A
#define INT_PIN 3

#define ACK_PORT A
#define ACK_PIN 0 /* UPDI */

#define STEP_PORT A
#define STEP_PIN 2

#endif

/**
 * Arduino IDE settings
 * FQBN: megaTinyCore:megaavr:atxy4:chip=1614,clock=20internal,millis=disabled
 * Core: https://github.com/SpenceKonde/megaTinyCore
 * Board: ATtiny1614/1604/814/804/414/404/214/204
 * Chip: ATtiny1614
 * Clock speed: 20MHz
 * millis/micros: Disabled
 * Programmer: Atmel mEDBG (ATmega32u4)
 */
#if defined(__AVR_ATtiny1614__) || defined(__AVR_ATtiny814__) || \
        defined(__AVR_ATtiny414__) || defined(__AVR_ATtiny214__)

#define ADRH_PORT A
#define ADR7 7
#define ADR6 6
#define ADR5 5
#define ADR4 4
#define ADR3 3
#define ADRHI 2
// PA2=7430 8 input NAND (AD15:AD8), should be Low in I/O address selected.
// PA1=#INT normally High
// PA0=UPDI normally High (pull upped internally)
#define IO_ADRH                                                         \
    (IOA(7) | IOA(6) | IOA(5) | IOA(4) | IOA(3) | (0 << 2) | (1 << 1) | \
            (1 << 0))
#define ADRH_BUS \
    (BVA(7) | BVA(6) | BVA(5) | BVA(4) | BVA(3) | BVA(HI) | (1 << 1) | (1 << 0))

#define CLK_E_PORT B
#define CLK_E_PIN 2

#define CLK_Q_PORT B
#define CLK_Q_PIN 3

#define INT_PORT A
#define INT_PIN 1

#define ACK_PORT B
#define ACK_PIN 0

#define STEP_PORT B
#define STEP_PIN 1

#endif

/**
 * Arduino IDE settings
 * FQBN: megaTinyCore:megaavr:atxy6:chip=3216,clock=20internal,millis=disabled
 * Core: https://github.com/SpenceKonde/megaTinyCore
 * Board: ATtiny3216/1616/1606/816/806/416/406
 * Chip: ATtiny3216
 * Clock speed: 20MHz
 * millis/micros: Disabled
 * Reset pin: Reset
 * Programmer: Atmel mEDBG (ATmega32u4)
 */
#if defined(__AVR_ATtiny3216__) || defined(__AVR_ATtiny1616__) || \
        defined(__AVR_ATtint816__) || defined(__AVR_ATtiny416__)

#define ADRH_PORT A
#define ADR15 3
#define ADR14 2
#define ADR13 1
#define ADR12 7
#define ADR11 6
#define ADR10 5
#define ADR5 4
#define IO_ADRH \
    (IOA(15) | IOA(14) | IOA(13) | IOA(12) | IOA(11) | IOA(10) | IOA(5))
#define ADRH_BUS \
    (BVA(15) | BVA(14) | BVA(13) | BVA(12) | BVA(11) | BVA(10) | BVA(5))

#define ADRM_PORT B
#define ADR9 1
#define ADR8 2
#define ADR7 3
#define ADR6 4
#define ADR4 5
#define IO_ADRM (IOA(9) | IOA(8) | IOA(7) | IOA(6) | IOA(4))
#define ADRM_BUS (BVA(9) | BVA(8) | BVA(7) | BVA(6) | BVA(4))

#define CLK_E_PORT C
#define CLK_E_PIN 2

#define CLK_Q_PORT C
#define CLK_Q_PIN 3

#define INT_PORT C
#define INT_PIN 1

#define ACK_PORT C
#define ACK_PIN 0

#define STEP_PORT B
#define STEP_PIN 0

#endif

/**
 * Arduino IDE settings
 * FQBN: MiniCore:avr:328:clock=20MHz_external,LTO=Os_flto,bootloader=no_bootloader
 * Core: https://github.com/MCUdude/MightyCore
 * Board: ATmega328
 * Clock speed: External 20MHz
 * Compiler LTO: LTO Enabled
 * Variant: 328P/328PA
 * Bootloader: No bootloader
 * Programmer: Arduino as ISP (MiniCore)
 */
#if defined(ARDUINO_AVR_ATmega328)

#define ADRH_PORT D
#define ADR15 7
#define ADR14 6
#define ADR13 5
#define ADR12 4
#define ADR11 3
#define ADR10 2
#define ADR9 1
#define ADR5 0
#define IO_ADRH                                                           \
    (IOA(15) | IOA(14) | IOA(13) | IOA(12) | IOA(11) | IOA(10) | IOA(9) | \
            IOA(5))
#define ADRH_BUS                                                          \
    (BVA(15) | BVA(14) | BVA(13) | BVA(12) | BVA(11) | BVA(10) | BVA(9) | \
            BVA(5))

#define ADRM_PORT B
#define ADR8 0
#define ADR7 5
#define ADR6 1
#define ADR4 2
#define ADR3 3
#define ADR2 4
#define IO_ADRM (IOA(8) | IOA(7) | IOA(6) | IOA(4) | IOA(3) | IOA(2))
#define ADRM_BUS (BVA(8) | BVA(7) | BVA(6) | BVA(4) | BVA(3) | BVA(2))

#define CLK_E_PORT C
#define CLK_E_PIN 4

#define CLK_Q_PORT C
#define CLK_Q_PIN 5

#define INT_PORT C
#define INT_PIN 3

#define ACK_PORT C
#define ACK_PIN 2

#define STEP_PORT C
#define STEP_PIN 1

#endif

#endif
