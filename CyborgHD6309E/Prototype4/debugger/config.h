/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define VERSION_TEXT F("* CyborgHD6309E Prototype4 2.1.0")

#if defined(ARDUINO_AVR_ATmega4809) && defined(MEGACOREX_DEFAULT_40PIN_PINOUT)
/**
 * Arduino IDE settings
 * FQBN: MegaCoreX:megaavr:4809:clock=internal_20MHz,pinout=40pin_standard,resetpin=reset,bootloader=uart1_default
 * Core: https://github.com/MCUdude/MegaCoreX
 * Board: ATmega4809
 * Clock: Internal 20MHz
 * Pinout: 40 pin standard
 * Reset pin: Reset
 * Bootloader: Optiboot (UART1 default pins)
 * Programmer: Atmel mEDBG (ATmega32u4)
 */

#define Console Serial1
#define CONSOLE_BAUD 115200

// PA: HD6309E data bus, bidirectional
#define DB_PORT A
#define DB_BUS 0xff

// PC0: Debugger serial TXD, output
#define TXD_PORT C
#define TXD_PIN 0

// PC1: Debugger serial RXD, input
#define RXD_PORT C
#define RXD_PIN 1

// PC2: RAM enable, output
#define RAM_E_PORT C
#define RAM_E_PIN 2

// PC3: Clock Generator #ACK, output
#define ACK_PORT C
#define ACK_PIN 3

// PC4: HD6309E A0, input
#define ADR0_PORT C
#define ADR0_PIN 4

// PC5: HD6309E A1, input
#define ADR1_PORT C
#define ADR1_PIN 5

// PD: HD6309E signals port
#define SIGNALS_PORT D
//#define SIGNALS_BUS 0xff

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

// PF2: Clock Generator #INT, input
#define INT_PORT F
#define INT_PIN 2
#define INT_INTERRUPT digitalPinToInterrupt(PIN_PF2)

// PF3: Clock Generator #STEP, output
#define STEP_PORT F
#define STEP_PIN 3

// PF4: User LED, output
#define USR_LED_PORT F
#define USR_LED_PIN 4

// PF5: User Switch, input
#define USR_SW_PORT F
#define USR_SW_PIN 5
#define USR_SW_INTERRUPT digitalPinToInterrupt(PIN_PF5)

#endif // MEGACOREX_DEFAULT_40PIN_PINOUT

#endif
