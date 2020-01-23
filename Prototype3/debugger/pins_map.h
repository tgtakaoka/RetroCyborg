/* -*- mode: c++; c-basic-offset: 2; tab-width: 2; -*- */
#ifndef __pins_map_h__
#define __pins_map_h__

#if defined(ARDUINO_AVR_ATmega32) || defined(ARDUINO_AVR_ATmega1284)

#define HALT_PORT  B
#define HALT_PIN   PB0

#define IRQ_PORT   B
#define IRQ_PIN    PB1

#define BS_PORT    C
#define BS_PIN     PC7

#define BA_PORT    C
#define BA_PIN     PC5

#define LIC_PORT   D
#define LIC_PIN    PD5

#define AVMA_PORT  C
#define AVMA_PIN   PC4

#define RESET_PORT B
#define RESET_PIN  PB2

#define STEP_PORT  D
#define STEP_PIN   PD7

#define ACK_PORT   D
#define ACK_PIN    PD6

#define INT_PORT   D
#define INT_PIN    PD3
#define INT_INTERRUPT digitalPinToInterrupt(11)

#define USR_SW_PORT D
#define USR_SW_PIN  PD2
#define USR_SW_INTERRUPT digitalPinToInterrupt(10)

#define USR_LED_PORT D
#define USR_LED_PIN  PD4

#define RD_WR_PORT B
#define RD_WR_PIN  PB3

#define RAM_E_PORT C
#define RAM_E_PIN  PC6

#define ADR0_PORT  C
#define ADR0_PIN   PC3

#define ADR1_PORT  C
#define ADR1_PIN   PC2

#define DB_PORT    A

#endif

#define __concat2__(a,b) a##b

#define __PIN__(name)  name##_PIN
#define __PORT__(name) name##_PORT
#define __PORT_DDR__(port) __concat2__(DDR,port)
#define __PORT_OUT__(port) __concat2__(PORT,port)
#define __PORT_IN__(port) __concat2__(PIN,port)
#define DDR(name) __PORT_DDR__(__PORT__(name))
#define PORT(name) __PORT_OUT__(__PORT__(name))
#define PIN(name) __PORT_IN__(__PORT__(name))

#endif
