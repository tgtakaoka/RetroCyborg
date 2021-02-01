/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define VERSION_TEXT F("* CyborgHD6309E Prototype3 2.1.0")

#if defined(ARDUINO_AVR_ATmega1284) && defined(STANDARD_PINOUT)
/**
 * Arduino IDE settings
 * FQBN: MightyCore:avr:1284:clock=16MHz_external,LTO=Os_flto,variant=modelP,pinout=standard,bootloader=uart0
 * Core: https://github.com/MCUdude/MightyCore
 * Board: ATmega1284
 * Clock: External 16 MHz
 * Compiler LTO: LTO enabled
 * Variantn: 1284P
 * Pinout: Standard input
 * Bootloader: Yes (UART0)
 * Programmer: Arduino as ISP (MightyCore)
 */

#define Console Serial
#define CONSOLE_BAUD 115200

// PA: HD6309E: data bus, bidirectional
#define DB_PORT A
#define DB_BUS 0xFF

// PB0: HD6309E #NMI, output
#define NMI_PORT B
#define NMI_PIN 0

// PB1: HD6309E #IRQ, output
#define IRQ_PORT B
#define IRQ_PIN 1

// PB2: HD6309E A0, input
#define ADR0_PORT B
#define ADR0_PIN 2

// PB3: HD6309E A1, input
#define ADR1_PORT B
#define ADR1_PIN 3

// PB4: Micro SD card, SPI Chip Select
#define SS_PORT B
#define SS_PIN 4
#define SD_CS_PIN PIN_PB4

// PB5: Micro SD card, SPI MOSI
#define MOSI_PORT B
#define MOSI_PIN 5

// PB6: Micro SD card, SPI MISO
#define MISO_PORT B
#define MISO_PIN 6

// PB7: Micro SD card, SPI SCK
#define SCK_PORT B
#define SCK_PIN 7

// PC: HD6309E signals port
#define SIGNALS_PORT C
//#define SIGNALS_BUS 0xff

// PC0: HD6309E #RESET, output
#define RESET_PORT SIGNALS_PORT
#define RESET_PIN 0

// PC1: HD6309E #HALT, output
#define HALT_PORT SIGNALS_PORT
#define HALT_PIN 1

// PC2: HD6309E R/W, input
#define RD_WR_PORT SIGNALS_PORT
#define RD_WR_PIN 2

// PC3: HD6309E BUSY, input
#define BUSY_PORT SIGNALS_PORT
#define BUSY_PIN 3

// PC4: HD6309E AVMA, input
#define AVMA_PORT SIGNALS_PORT
#define AVMA_PIN 4

// PC5: HD6309E BA, input
#define BA_PORT SIGNALS_PORT
#define BA_PIN 5

// PC6: HD6309E LIC, input
#define LIC_PORT SIGNALS_PORT
#define LIC_PIN 6

// PC7: HD6309E BS, input
#define BS_PORT SIGNALS_PORT
#define BS_PIN 7

// PD0: Debugger serial RXD, input
#define RXD_PORT D
#define RXD_PIN 0

// PD1: Debugger serial TXD, output
#define TXD_PORT D
#define TXD_PIN 1

// PD2: User Switch, input
#define USR_SW_PORT D
#define USR_SW_PIN 2
#define USR_SW_INTERRUPT digitalPinToInterrupt(PIN_PD2)

// PD3: Clock Generator #INT, input
#define INT_PORT D
#define INT_PIN 3
#define INT_INTERRUPT digitalPinToInterrupt(PIN_PD3)

// PD4: User LED, output
#define USR_LED_PORT D
#define USR_LED_PIN 4

// PD5: RAM enable, output
#define RAM_E_PORT D
#define RAM_E_PIN 5

// PD6: Clock Generator #ACK, output
#define ACK_PORT D
#define ACK_PIN 6

// PD7: Clock Generator #STEP, output
#define STEP_PORT D
#define STEP_PIN 7

#endif

#endif
