#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define VERSION_TEXT "0.1"
#define Console Serial

#define ACIA_BASE_ADDR 0xDF00

#if defined(ARDUINO_TEENSY41)
#define PORT_DBUS 6   /* GPIO6 */
#define DBUS_gp 16    /* P6.16-P6.23 */
#define DBUS_gm 0xFF  /* P6.00-P6.07 */
#define DBUS_vp 0     /* B0-B7 */
#define PIN_DBUS0 19  /* P6.16 */
#define PIN_DBUS1 18  /* P6.17 */
#define PIN_DBUS2 14  /* P6.18 */
#define PIN_DBUS3 15  /* P6.19 */
#define PIN_DBUS4 40  /* P6.20 */
#define PIN_DBUS5 41  /* P6.21 */
#define PIN_DBUS6 17  /* P6.22 */
#define PIN_DBUS7 16  /* P6.23 */
#define PORT_MA 6     /* GPIO6 */
#define MA_gp 24      /* P6.24-P6.31 */
#define MA_gm 0xFF    /* P6.24-P6.31 */
#define MA_vp 0       /* MA0-MA7 */
#define PIN_MA0 22    /* P6.24 */
#define PIN_MA1 23    /* P6.25 */
#define PIN_MA2 20    /* P6.26 */
#define PIN_MA3 21    /* P6.27 */
#define PIN_MA4 38    /* P6.28 */
#define PIN_MA5 39    /* P6.29 */
#define PIN_MA6 26    /* P6.30 */
#define PIN_MA7 27    /* P6.31 */
#define PORT_N 7      /* GPIO7 */
#define N_gp 0        /* P7.00-P7.02 */
#define N_gm 0x07     /* P7.00-P7.02 */
#define N_vp 0        /* N0-N2 */
#define PIN_N0 10     /* P7.00 */
#define PIN_N1 12     /* P7.01 */
#define PIN_N2 11     /* P7.02 */
#define PIN_Q 13      /* P7.03 */
#define PORT_EF 7     /* GPIO7 */
#define EF_gp 16      /* P7.16-P7.19 */
#define EF_gm 0xF     /* P7.16-P7.19 */
#define EF_vp 0       /* #EF1-#EF4 */
#define PIN_EF1 8     /* P7.16 */
#define PIN_EF2 7     /* P7.17 */
#define PIN_EF3 36    /* P7.18 */
#define PIN_EF4 37    /* P7.19 */
#define PIN_TPA 2     /* P9.04 */
#define PIN_TPB 3     /* P9.05 */
#define PIN_INTR 4    /* P9.06 */
#define PIN_SC0 0     /* P6.03 */
#define PIN_SC1 1     /* P6.02 */
#define PIN_CLOCK 5   /* P9.08 */
#define PIN_MRD 6     /* P7.10 */
#define PIN_MWR 9     /* P7.11 */
#define PIN_WAIT 32   /* P7.12 */
#define PIN_CLEAR 28  /* P8.18 */
#define PIN_DMAIN 31  /* P8.22 */
#define PIN_DMAOUT 30 /* P8.23 */
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
