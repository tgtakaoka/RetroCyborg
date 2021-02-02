/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define VERSION_TEXT F("* CyborgHD6309E Prototype5 2.2.0")

#define IO_BASE_ADDR 0xDF00

#if defined(__AVR_ATmega4809__) && defined(MEGACOREX_DEFAULT_40PIN_PINOUT)
/**
 * Arduino IDE settings
 * FQBN: MegaCoreX:megaavr:4809:clock=internal_16MHz,pinout=40pin_standard,resetpin=reset,bootloader=uart1_default
 * Core: https://github.com/MCUdude/MegaCoreX
 * Board: ATmega4809
 * Clock: Internal 16MHz
 * Pinout: 40 pin standard
 * Reset pin: Reset
 * Bootloader: Optiboot (UART1 default pins)
 * Programmer: Atmel mEDBG (ATmega32u4)
 */

#define Console Serial1
#define CONSOLE_BAUD 115200

/*
 * E_OSC: TCA0-WO3-PORTB(PB3)
 * Q_OSC: TCA0-WO0-PORTB(PB0)
 * E_CLK: CCL-LUT0, EVOUTF(PF2)
 *        IN0=CCLLUT0A(CHANNEL2:#INT)
 *        IN2=CCLLUT0B(CHANNEL1:E_OSC)
 * Q_CLK: CCL-LUT1, EVOUTC(PC2)
 *        IN0=CCLLUT1A(CHANNEL2:#INT)
 *        IN2=CCLLUT1B(CHANNEL0:Q_OSC)
 * #IOR_E: CCL-LUT2
 *        IN1=CCLLUT2A(CHANNEL4:#IOR)
 *        CLK=CCLLUT2B(CHANNEL1:E_OSC)
 * #INT: SEQUNCE1-OUT
 *        SEQSEL1: DFF(D Flip FLop)
 *        D: CCL-LUT2(#IOR_E)
 *        G: 1
 *        CLK: CCL-LUT2-IN2(E_OSC)
 * CHANNEL0: PB0(Q_OSC)
 * CHANNEL1: PB3(E_OSC)
 * CHANNEL2: CCL-LUT2(#INT)
 * CHANNEL3: CCL-LUT0(E_CLK)
 * CHANNEL4: PF3(#IOR)
 * CHANNEL5: CCL-LUT1(Q_CLK)
 * EVOUTC: PC2(Q_CLK)
 * EVOUTF: PF2(E_CLK)
 */

/* Timer A, Split Single-slope PWM mode */
#define CLK_MHZ (F_CPU / 1000000L)
#define MPU_MHZ 2
#define CLK_PER_MPU (CLK_MHZ / MPU_MHZ)

// PB0(internal): TCA0-WO0-PORTB
#define Q_OSC_PINCTRL PORTB.PIN0CTRL
#define Q_OSC_DIRSET PORTB.DIRSET
#define Q_OSC_bm (1 << 0)

// PB3(internal): TCA0-WO3-PORTB
#define E_OSC_PINCTRL PORTB.PIN3CTRL
#define E_OSC_DIRSET PORTB.DIRSET
#define E_OSC_bm (1 << 3)
#define E_OSC_CHANNEL 1

// PA: HD6309E data bus, bidirectional
#define DB_PORT A
#define DB_BUS 0xff

// PC0: Debugger serial TXD, output
#define TXD_PORT C
#define TXD_PIN 0

// PC1: Debugger serial RXD, input
#define RXD_PORT C
#define RXD_PIN 1

// PC2: HD6309E Q, output
#define Q_CLK_PORT C
#define Q_CLK_PIN 2

// PC3: RAM enable, output
#define RAM_E_PORT C
#define RAM_E_PIN 3

// PC4: HD6309E A0, input
#define ADR0_PORT C
#define ADR0_PIN 4

// PC5: HD6309E A1, input
#define ADR1_PORT C
#define ADR1_PIN 5

// PD: HD6309E signals port
#define SIGNALS_PORT D
#define SIGNALS_BUS 0xff

// PD0: HD6309E #RESET, output
#define RESET_PORT SIGNALS_PORT
#define RESET_PIN 0

// PD1: HD6309E LIC, input
#define LIC_PORT SIGNALS_PORT
#define LIC_PIN 1

// PD2: HD6309E R/W, input
#define RD_WR_PORT SIGNALS_PORT
#define RD_WR_PIN 2

// PD3: HD6309E AVMA, input
#define AVMA_PORT SIGNALS_PORT
#define AVMA_PIN 3

// PD4: HD6309E BS, input
#define BS_PORT SIGNALS_PORT
#define BS_PIN 4

// PD5: HD6309E BA, input
#define BA_PORT SIGNALS_PORT
#define BA_PIN 5

// PD6: HD6309E #HALT, output
#define HALT_PORT SIGNALS_PORT
#define HALT_PIN 6

// PD7: HD6309E BUSY, input
#define BUSY_PORT SIGNALS_PORT
#define BUSY_PIN 7

// SPI use Alternate 2 pin mapping.
#define SPI_MAPPING 2

// PE0: Micro SD card, SPI MOSI
#define MOSI_PORT E
#define MOSI_PIN 0

// PE1: Micro SD card, SPI MISO
#define MISO_PORT E
#define MISO_PIN 1

// PE2: Micro SD card, SPI SCK
#define SCK_PORT E
#define SCK_PIN 2

// PE3: Micro SD card, SPI Chip Select
#define SS_PORT E
#define SS_PIN 3
#define SD_CS_PIN PIN_PE3

// PF0: HD6309E #IRQ, output
#define IRQ_PORT F
#define IRQ_PIN 0

// PF1: HD6309E #NMI, output
#define NMI_PORT F
#define NMI_PIN 1

// PF2: HD6309E E, output
#define E_CLK_PORT F
#define E_CLK_PIN 2

// PF3: 74HC688 #IOR, input
#define IOR_PORT F
#define IOR_PIN 3

// PF4: User LED, output
#define USR_LED_PORT F
#define USR_LED_PIN 4

// PF5: User Switch, input
#define USR_SW_PORT F
#define USR_SW_PIN 5
#define USR_SW_INTERRUPT digitalPinToInterrupt(PIN_PF5)

#else

#error "Unknown Arduino board config"

#endif // MEGACOREX_DEFAULT_40PIN_PINOUT

#endif
