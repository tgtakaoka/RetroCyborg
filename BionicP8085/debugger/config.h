#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define VERSION_TEXT "0.1"
#define Console Serial

#define USART_BASE_ADDR 0x00

#if defined(ARDUINO_TEENSY41)
#define PORT_AD 6     /* GPIO6 */
#define AD_gp 16      /* P6.16-P6.23 */
#define AD_gm 0xFF    /* P6.16-P6.23 */
#define AD_vp 0       /* AD0-AD7 */
#define PIN_AD0 19    /* P6.16 */
#define PIN_AD1 18    /* P6.17 */
#define PIN_AD2 14    /* P6.18 */
#define PIN_AD3 15    /* P6.19 */
#define PIN_AD4 40    /* P6.20 */
#define PIN_AD5 41    /* P6.21 */
#define PIN_AD6 17    /* P6.22 */
#define PIN_AD7 16    /* P6.23 */
#define PORT_AH 6     /* GPIO6 */
#define AH_gp 24      /* P6.24-P6.31 */
#define AH_gm 0xFF    /* P6.24-P6.31 */
#define AH_vp 8       /* A8-A15 */
#define PIN_AH8 22    /* P6.24 */
#define PIN_AH9 23    /* P6.25 */
#define PIN_AH10 20   /* P6.26 */
#define PIN_AH11 21   /* P6.27 */
#define PIN_AH12 38   /* P6.28 */
#define PIN_AH13 39   /* P6.29 */
#define PIN_AH14 26   /* P6.30 */
#define PIN_AH15 27   /* P6.31 */
#define PIN_RST55 10  /* P7.00 */
#define PIN_RST65 12  /* P7.01 */
#define PIN_RST75 11  /* P7.02 */
#define PORT_S 7      /* GPIO7 */
#define S_gp 16       /* P7.16-P7.19 */
#define S_gm 0x3      /* P7.16-P7.19 */
#define S_vp 0        /* S0-S1 */
#define PIN_S0 8      /* P7.16 */
#define PIN_S1 7      /* P7.17 */
#define PIN_HLDA 36   /* P7.18 */
#define PIN_HOLD 37   /* P7.19 */
#define PIN_ALE 2     /* P9.04 */
#define PIN_INTR 4    /* P9.06 */
#define PIN_TRAP 33   /* P9.06 */
#define PIN_SOD 0     /* P6.03 */
#define PIN_SID 1     /* P6.02 */
#define PIN_X1 5      /* P9.08 */
#define PIN_CLK 29    /* P9.09 */
#define PIN_RD 6      /* P7.10 */
#define PIN_WR 9      /* P7.11 */
#define PIN_READY 32  /* P7.12 */
#define PIN_RESET 28  /* P8.18 */
#define PIN_INTA 31   /* P8.22 */
#define PIN_IOM 30    /* P8.23 */
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
