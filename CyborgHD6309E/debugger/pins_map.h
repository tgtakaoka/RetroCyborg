/* -*- mode: c++; c-basic-offset: 4; tab-width: 4; -*- */
#ifndef __PINS_MAP_H__
#define __PINS_MAP_H__

#define __concat2__(a, b) a##b
#define __concat3__(a, b, c) a##b##c

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
#if defined(ARDUINO_AVR_ATmega4809)

#define Console Serial1
#define CONSOLE_BAUD 115200

#define SPI_MAPPING 2

// PA: HD6309E data bus, bidirectional
#define DB_PORT A
#define DB_BUS 0xff

// PC0: Debugger serial TXD, output
#define TXD_PORT C
#define TXD_PIN 0

// PC1: Debugger serial RXD, input
#define RXD_PORT C
#define RXD_PIN 1

// PC2: HD6309E A0, input
#define ADR0_PORT C
#define ADR0_PIN 2

// PC3: HD6309E A1, input
#define ADR1_PORT C
#define ADR1_PIN 3

// PC4: HD6309E #RESET, output
#define RESET_PORT C
#define RESET_PIN 4

// PC5: HD6309E LIC, input
#define LIC_PORT C
#define LIC_PIN 5

// PD0: #ACK, output
#define ACK_PORT D
#define ACK_PIN 0

// PD1: #STEP, output
#define STEP_PORT D
#define STEP_PIN 1

// PD2: #INT, input
#define INT_PORT D
#define INT_PIN 2
#define INT_INTERRUPT digitalPinToInterrupt(16)

// PD3: HD6309E AVMA, input
#define AVMA_PORT D
#define AVMA_PIN 3

// PD4: HD6309E BS, input
#define BS_PORT D
#define BS_PIN 4

// PD5: HD6309E BA, input
#define BA_PORT D
#define BA_PIN 5

// PD6: HD6309E #HALT, output
#define HALT_PORT D
#define HALT_PIN 6

// PD7: RAM enable, output
#define RAM_E_PORT D
#define RAM_E_PIN 7

// PE0: SPI MOSI
#define MOSI_PORT E
#define MOSI_PIN 0

// PE1: SPI MISO
#define MISO_PORT E
#define MISO_PIN 1

// PE2: SPI SCK
#define SCK_PORT E
#define SCK_PIN 2

// PE3: SPI Chip Select
#define SS_PORT E
#define SS_PIN 3
#define SD_CS_PIN 25            // PE3

// PF0: HD6309E #IRQ, output
#define IRQ_PORT F
#define IRQ_PIN 0

// PF1: HD6309E #NMI, output
#define NMI_PORT F
#define NMI_PIN 1

// PF2: HD6309E R/W, input
#define RD_WR_PORT F
#define RD_WR_PIN 2

// PF3: User Switch, input
#define USR_SW_PORT F
#define USR_SW_PIN 3
#define USR_SW_INTERRUPT digitalPinToInterrupt(29)

// PF4: User LED, output
#define USR_LED_PORT F
#define USR_LED_PIN 4

// PF5: Debugger Serial RTS, output
#define DBG_RTS_PORT F
#define DBG_RTS_PIN 5

#endif

/**
 * Arduino IDE settings
 * FQBN: MightyCore:avr:1284:clock=16MHz_external,variant=modelP,pinout=standard,bootloader=uart0,LTO=Os_flto
 * Core: https://github.com/MCUdude/MightyCore
 * Board: ATmega1284
 * Clock: External 16MHz
 * Compiler LTO: LTO Enabled
 * Variant: 1284P
 * Pinout: Standard pinout
 * Bootloader: Yes (UART0)
 * Programmer: Arduino as ISP (MightyCore)
 */
#if defined(ARDUINO_AVR_ATmega1284) || defined(ARDUINO_AVR_ATmega644)

#define Console Serial
#define CONSOLE_BAUD 115200

#define HALT_PORT B
#define HALT_PIN 0

#define IRQ_PORT B
#define IRQ_PIN 1

#define BS_PORT C
#define BS_PIN 7

#define BA_PORT C
#define BA_PIN 5

#define LIC_PORT D
#define LIC_PIN 5

#define AVMA_PORT C
#define AVMA_PIN 4

#define RESET_PORT B
#define RESET_PIN 2

#define STEP_PORT D
#define STEP_PIN 7

#define ACK_PORT D
#define ACK_PIN 6

#define INT_PORT D
#define INT_PIN 3
#define INT_INTERRUPT digitalPinToInterrupt(11)

#define USR_SW_PORT D
#define USR_SW_PIN 2
#define USR_SW_INTERRUPT digitalPinToInterrupt(10)

#define USR_LED_PORT D
#define USR_LED_PIN 4

#define RD_WR_PORT B
#define RD_WR_PIN 3

#define RAM_E_PORT C
#define RAM_E_PIN 6

#define ADR0_PORT C
#define ADR0_PIN 3

#define ADR1_PORT C
#define ADR1_PIN 2

#define DB_PORT A
#define DB_BUS 0xFF

#endif

#endif
