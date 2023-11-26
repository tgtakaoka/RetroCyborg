#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define VERSION_TEXT "0.1"
#define Console Serial

#define USART_BASE_ADDR 0x00

#if defined(ARDUINO_TEENSY41)
#define PORT_DBUS 6    /* GPIO6 */
#define DBUS_gp 16     /* P6.16-P6.23 */
#define DBUS_gm 0xFF   /* P6.16-P6.23 */
#define DBUS_vp 0      /* DBUS0-DBUS7 */
#define PIN_DBUS0 19   /* P6.16 */
#define PIN_DBUS1 18   /* P6.17 */
#define PIN_DBUS2 14   /* P6.18 */
#define PIN_DBUS3 15   /* P6.19 */
#define PIN_DBUS4 40   /* P6.20 */
#define PIN_DBUS5 41   /* P6.21 */
#define PIN_DBUS6 17   /* P6.22 */
#define PIN_DBUS7 16   /* P6.23 */
#define PORT_ADRL 6    /* GPIO6 */
#define ADRL_gp 24     /* P6.24-P6.31 */
#define ADRL_gm 0xFF   /* P6.24-P6.31 */
#define ADRL_vp 0      /* ADR0-ADR7 */
#define PIN_ADR0 22    /* P6.24 */
#define PIN_ADR1 23    /* P6.25 */
#define PIN_ADR2 20    /* P6.26 */
#define PIN_ADR3 21    /* P6.27 */
#define PIN_ADR4 38    /* P6.28 */
#define PIN_ADR5 39    /* P6.29 */
#define PIN_ADR6 26    /* P6.30 */
#define PIN_ADR7 27    /* P6.31 */
#define PORT_ADRM 7    /* GPIO7 */
#define ADRM_gp 0      /* P7.00-P7.03 */
#define ADRM_gm 0xF    /* P7.00-P7.03 */
#define ADRM_vp 8      /* ADR8-ADR11 */
#define PIN_ADR8 10    /* P7.00 */
#define PIN_ADR9 12    /* P7.01 */
#define PIN_ADR10 11   /* P7.02 */
#define PIN_ADR11 13   /* P7.03 */
#define PORT_ADRH 7    /* GPIO7 */
#define ADRH_gp 16     /* P7.16-P7.18 */
#define ADRH_gm 0x7    /* P7.16-P7.18 */
#define ADRH_vp 12     /* ADR12-ADR14 */
#define PIN_ADR12 8    /* P7.16 */
#define PIN_ADR13 7    /* P7.17 */
#define PIN_ADR14 36   /* P7.18 */
#define PIN_MIO 37     /* P6.19 */
#define PIN_PAUSE 2    /* P9.04 */
#define PIN_RW 3       /* P9.05 */
#define PIN_INTREQ 4   /* P9.06 */
#define PIN_INTACK 33  /* P9.07 */
#define PIN_FLAG 0     /* P6.03 */
#define PIN_SENSE 1    /* P6.04 */
#define PIN_CLOCK 5    /* P9.08 */
#define PIN_RUNWAIT 29 /* P9.31 */
#define PIN_OPREQ 6    /* P7.10 */
#define PIN_WRP 9      /* P7.11 */
#define PIN_DBUSEN 32  /* P7.12 */
#define PIN_RESET 28   /* P8.18 */
#define PIN_ADREN 31   /* P8.22 */
#define PIN_OPACK 30   /* P8.23 */
#define PIN_USRSW 35   /* P7.28 */
#define PIN_USRLED 34  /* P7.29 */
#endif

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// tab-width: 4
// End:
// vim: set ft=cpp et ts=4 sw=4:
