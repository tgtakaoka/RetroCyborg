/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __pins_map_h__
#define __pins_map_h__

#if defined(ARDUINO_AVR_ATmega32) || defined(ARDUINO_AVR_ATmega1284)

#define Console      Serial
#define CONSOLE_BAUD 115200

#define HALT_PORT  B
#define HALT_PIN   0

#define IRQ_PORT   B
#define IRQ_PIN    1

#define BS_PORT    C
#define BS_PIN     7

#define BA_PORT    C
#define BA_PIN     5

#define LIC_PORT   D
#define LIC_PIN    5

#define AVMA_PORT  C
#define AVMA_PIN   4

#define RESET_PORT B
#define RESET_PIN  2

#define STEP_PORT  D
#define STEP_PIN   7

#define ACK_PORT   D
#define ACK_PIN    6

#define INT_PORT   D
#define INT_PIN    3
#define INT_INTERRUPT digitalPinToInterrupt(11)

#define USR_SW_PORT D
#define USR_SW_PIN  2
#define USR_SW_INTERRUPT digitalPinToInterrupt(10)

#define USR_LED_PORT D
#define USR_LED_PIN  4

#define RD_WR_PORT B
#define RD_WR_PIN  3

#define RAM_E_PORT C
#define RAM_E_PIN  6

#define ADR0_PORT  C
#define ADR0_PIN   3

#define ADR1_PORT  C
#define ADR1_PIN   2

#define DB_PORT    A
#define DB_BUS     0xFF

#endif

#define __concat2__(a,b) a##b

#define __PIN__(name)  name##_PIN
#define __BUS__(name)  name##_BUS
#define __PORT__(name) name##_PORT
#define __PORT_DDR__(port) __concat2__(DDR,port)
#define __PORT_OUT__(port) __concat2__(PORT,port)
#define __PORT_IN__(port)  __concat2__(PIN,port)
#define PDIR(name) __PORT_DDR__(__PORT__(name))
#define POUT(name) __PORT_OUT__(__PORT__(name))
#define PIN(name)  __PORT_IN__(__PORT__(name))
#define PIN_m(name)   _BV(__PIN__(name))
#define BUS_gm(name)  __BUS__(name)

#endif
