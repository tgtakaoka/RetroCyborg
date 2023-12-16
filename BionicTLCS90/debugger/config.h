#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define VERSION_TEXT "0.1"
#define Console Serial

#define USART_BASE_ADDR 0xFFF0

#if defined(ARDUINO_TEENSY41)
#define PORT_DB 6     /* GPIO6 */
#define DB_gp 16      /* P6.16-P6.23 */
#define DB_gm 0xFF    /* P6.16-P6.23 */
#define DB_vp 0       /* D0-D7 */
#define PIN_DB0 19    /* P6.16 */
#define PIN_DB1 18    /* P6.17 */
#define PIN_DB2 14    /* P6.18 */
#define PIN_DB3 15    /* P6.19 */
#define PIN_DB4 40    /* P6.20 */
#define PIN_DB5 41    /* P6.21 */
#define PIN_DB6 17    /* P6.22 */
#define PIN_DB7 16    /* P6.23 */
#define PORT_ADL 6    /* GPIO6 */
#define ADL_gp 24     /* P6.24-P6.31 */
#define ADL_gm 0xFF   /* P6.24-P6.31 */
#define ADL_vp 0      /* A0-A7 */
#define PIN_AD0 22    /* P6.24 */
#define PIN_AD1 23    /* P6.25 */
#define PIN_AD2 20    /* P6.26 */
#define PIN_AD3 21    /* P6.27 */
#define PIN_AD4 38    /* P6.28 */
#define PIN_AD5 39    /* P6.29 */
#define PIN_AD6 26    /* P6.30 */
#define PIN_AD7 27    /* P6.31 */
#define PORT_ADM 7    /* GPIO7 */
#define ADM_gp 0      /* P7.00-P7.03 */
#define ADM_gm 0xF    /* P7.00-P7.03 */
#define ADM_vp 8      /* A8-A11 */
#define PIN_AD8 10    /* P7.00 */
#define PIN_AD9 12    /* P7.01 */
#define PIN_AD10 11   /* P7.02 */
#define PIN_AD11 13   /* P7.03 */
#define PORT_ADH 7    /* GPIO7 */
#define ADH_gp 16     /* P7.16-P7.19 */
#define ADH_gm 0xF    /* P7.16-P7.19 */
#define ADH_vp 12     /* A12-A14 */
#define PIN_AD12 8    /* P7.16 */
#define PIN_AD13 7    /* P7.17 */
#define PIN_AD14 36   /* P7.18 */
#define PIN_AD15 37   /* P6.19 */
#define PIN_INT1 2    /* P9.04 */
#define PIN_INT0 4    /* P9.06 */
#define PIN_NMI 33    /* P9.07 */
#define PIN_TXD 0     /* P6.03 */
#define PIN_RXD 1     /* P6.04 */
#define PIN_X1 5      /* P9.08 */
#define PIN_CLK 29    /* P9.31 */
#define PIN_RD 6      /* P7.10 */
#define PIN_WR 9      /* P7.11 */
#define PIN_WAIT 32   /* P7.12 */
#define PIN_RESET 28  /* P8.18 */
#define PIN_RTS 31    /* P8.22 */
#define PIN_USRSW 24  /* P6.12 */
#define PIN_USRLED 25 /* P6.13 */
#endif

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
