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

#define HALT_PORT A
#define HALT_PIN 7

#define IRQ_PORT A
#define IRQ_PIN 6

#define BS_PORT A
#define BS_PIN 0

#define BA_PORT A
#define BA_PIN 1

#define LIC_PORT A
#define LIC_PIN 4

#define AVMA_PORT A
#define AVMA_PIN 3

#define RESET_PORT A
#define RESET_PIN 5

#define STEP_PORT F
#define STEP_PIN 2

#define ACK_PORT F
#define ACK_PIN 1

#define INT_PORT F
#define INT_PIN 0
#define INT_INTERRUPT digitalPinToInterrupt(26)

#define USR_SW_PORT F
#define USR_SW_PIN 5
#define USR_SW_INTERRUPT digitalPinToInterrupt(31)

#define USR_LED_PORT F
#define USR_LED_PIN 4

#define RD_WR_PORT A
#define RD_WR_PIN 2

#define RAM_E_PORT F
#define RAM_E_PIN 3

#define ADR0_PORT C
#define ADR0_PIN 4

#define ADR1_PORT C
#define ADR1_PIN 5

#define DB_PORT D
#define DB_BUS 0xff

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
