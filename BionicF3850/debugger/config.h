#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define VERSION_TEXT "0.1"
#define Console Serial

#define USART_BASE_ADDR 0xF0

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
#define PORT_IO0 6    /* GPIO6 */
#define IO0_gp 24     /* P6.24-P6.31 */
#define IO0_gm 0xFF   /* P6.24-P6.31 */
#define IO0_vp 0      /* IO00-IO07 */
#define PIN_IO00 22   /* P6.24 */
#define PIN_IO01 23   /* P6.25 */
#define PIN_IO02 20   /* P6.26 */
#define PIN_IO03 21   /* P6.27 */
#define PIN_IO04 38   /* P6.28 */
#define PIN_IO05 39   /* P6.29 */
#define PIN_IO06 26   /* P6.30 */
#define PIN_IO07 27   /* P6.31 */
#define PORT_IO1L 7   /* GPIO7 */
#define IO1L_gp 0     /* P7.01-P7.03 */
#define IO1L_gm 0xF   /* P7.01-P7.03 */
#define IO1L_vp 0     /* IO10-IO13 */
#define PIN_IO1L0 10  /* P7.0 */
#define PIN_IO1L1 12  /* P7.1 */
#define PIN_IO1L2 11  /* P7.2 */
#define PIN_IO1L3 13  /* P6.3 */
#define PORT_IO1H 7   /* GPIO7 */
#define IO1H_gp 16    /* P7.16-P7.19 */
#define IO1H_gm 0xF   /* P7.16-P7.19 */
#define IO1H_vp 4     /* IO14-IO17 */
#define PIN_IO1H4 8   /* P7.16 */
#define PIN_IO1H5 7   /* P7.17 */
#define PIN_IO1H6 36  /* P7.18 */
#define PIN_IO1H7 37  /* P6.19 */
#define PORT_ROMC 9   /* GPIO9 */
#define ROMC_gp 4     /* P9.04-P9.08 */
#define ROMC_gm 0x1F  /* P9.04-P9.08 */
#define ROMC_vp 0     /* ROMC0-ROMC4 */
#define PIN_ROMC0 2   /* P9.04 */
#define PIN_ROMC1 3   /* P9.05 */
#define PIN_ROMC2 4   /* P9.66 */
#define PIN_ROMC3 33  /* P9.07 */
#define PIN_ROMC4 5   /* P9.08 */
#define PIN_XTLY 29   /* P9.31 */
#define PIN_PHI 6     /* P7.10 */
#define PIN_WRITE 9   /* P7.11 */
#define PIN_EXTRES 28 /* P8.18 */
#define PIN_ICB 31    /* P8.22 */
#define PIN_INTREQ 30 /* P8.23 */
#define PIN_USRSW 35  /* P7.28 */
#define PIN_USRLED 34 /* P7.29 */
#endif

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
