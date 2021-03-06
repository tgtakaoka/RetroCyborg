/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __PINS_MAP_H__
#define __PINS_MAP_H__

#define __concat2__(a, b) a##b
#define __concat3__(a, b, c) a##b##c

/**
 * Arduino IDE settings
 * FQBN:
 * MegaCoreX:megaavr:4809:clock=internal_20MHz,pinout=40pin_standard,resetpin=reset,bootloader=uart1_default
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

// PD: Z80 data bus, bidirectional
#define DB_PORT D
#define DB_BUS 0xff

// PC0: Debugger serial TXD, output
#define TXD_PORT C
#define TXD_PIN 0

// PC1: Debugger serial RXD, input
#define RXD_PORT C
#define RXD_PIN 1

// PC2: RAM enable, output
#define RAMEN_PORT C
#define RAMEN_PIN 2

// PC3: User Switch, input
#define USR_SW_PORT C
#define USR_SW_PIN 3
#define USR_SW_INTERRUPT digitalPinToInterrupt(11)

// PC4: User LED, output
#define USR_LED_PORT C
#define USR_LED_PIN 4

// PC5: Debugger Serial RTS, output
#define DBG_RTS_PORT C
#define DBG_RTS_PIN 5

// PA0: Z80 #IORQ, input pullup (CCL:LUT0.IN0)
#define IORQ_PORT A
#define IORQ_PIN 0

// PA1: Z80 #MREQ, input pullup (CCL:LUT0.IN1)
#define MREQ_PORT A
#define MREQ_PIN 1

// PA2: Z80 #M1, input pullup
#define M1_PORT A
#define M1_PIN 2

// PA3: Z80 #WAIT, output (CCL:LUT0.OUT)
#define WAIT_PORT A
#define WAIT_PIN 3

// PA4: Z80 #RD, input pullup
#define RD_PORT A
#define RD_PIN 4

// PA5: Z80 #WR, input pullup
#define WR_PORT A
#define WR_PIN 5

// PA6: Z80 #BUSREQ, output
#define BUSREQ_PORT A
#define BUSREQ_PIN 6

// PA7: Z80 #BUSACK, input pullup
#define BUSACK_PORT A
#define BUSACK_PIN 7

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

// PF0: Z80 #INT, output
#define INT_PORT F
#define INT_PIN 0

// PF1: Z80 #NMI, output
#define NMI_PORT F
#define NMI_PIN 1

// PF2: Z80 A0, input pullup
#define AD0_PORT F
#define AD0_PIN 2

// PF3: Z80 A1, input pullup
#define AD1_PORT F
#define AD1_PIN 3

// PF2: Z80 #RESET, output
#define RESET_PORT F
#define RESET_PIN 4

// PF2: Z80 CLK, output
#define CLK_PORT F
#define CLK_PIN 6

#endif  // ARDUINO_AVR_ATmega4809

#endif
