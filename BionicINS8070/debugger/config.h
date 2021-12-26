#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define MPU_NAME "INS8070"

#define VERSION_TEXT F("* Bionic " MPU_NAME " 0.1")
#define Console Serial

#define IO_BASE_ADDR 0xDF00

#if defined(ARDUINO_TEENSY35)
#define PORT_D D      /* GPIOD */
#define D_gp 0        /* PTD00-PTD07 */
#define D_gm 0xFF     /* PTD00-PTD07 */
#define D_vp 0        /* D0-D7 */
#define PIN_D0 2      /* PTD00 */
#define PIN_D1 14     /* PTD01 */
#define PIN_D2 7      /* PTD02 */
#define PIN_D3 8      /* PTD03 */
#define PIN_D4 6      /* PTD04 */
#define PIN_D5 20     /* PTD05 */
#define PIN_D6 21     /* PTD06 */
#define PIN_D7 5      /* PTD07 */
#define PORT_AL C     /* GPIOC */
#define AL_gp 0       /* PTC00-PTC11 */
#define AL_gm 0xFFF   /* PTC00-PTC11 */
#define AL_vp 0       /* A0-A11 */
#define PIN_AL0 15    /* PTC00 */
#define PIN_AL1 22    /* PTC01 */
#define PIN_AL2 23    /* PTC02 */
#define PIN_AL3 9     /* PTC03 */
#define PIN_AL4 10    /* PTC04 */
#define PIN_AL5 13    /* PTC05 */
#define PIN_AL6 11    /* PTC06 */
#define PIN_AL7 12    /* PTC07 */
#define PIN_AL8 35    /* PTC08 */
#define PIN_AL9 36    /* PTC09 */
#define PIN_AL10 37   /* PTC10 */
#define PIN_AL11 38   /* PTC11 */
#define PORT_AH A     /* GPIOA */
#define AH_gp 12      /* PTA12 */
#define AH_gm 0xF     /* PTA12-PTA15 */
#define AH_vp 12      /* A12-A15 */
#define PIN_AH12 3    /* PTA12 */
#define PIN_AH13 4    /* PTA13 */
#define PIN_AH14 26   /* PTA14 */
#define PIN_AH15 27   /* PTA15 */
#define PIN_WDS 17    /* PTB01 */
#define PIN_SA 19     /* PTB02 */
#define PIN_SB 18     /* PTB03 */
#define PIN_F1 0      /* PTB16 */
#define PIN_F2 1      /* PTB17 */
#define PIN_XIN 29    /* PTB18 */
#define PIN_BREQ 30   /* PTB19 */
#define PIN_RDS 33    /* PTE24 */
#define PIN_F3 34     /* PTE25 */
#define PIN_HOLD 24   /* PTE26 */
#define PIN_RST 25    /* PTA05 */
#define PIN_ENIN 28   /* PTA16 */
#define PIN_ENOUT 39  /* PTA17 */
#define PIN_USRSW 31  /* PTB10 */
#define PIN_USRLED 32 /* PTB11 */
#endif

#if defined(ARDUINO_TEENSY41)
#define PORT_D 6      /* GPIO6 */
#define D_gp 16       /* P6.16-P6.23 */
#define D_gm 0xFF     /* P6.16-P6.23 */
#define D_vp 0        /* D0-D7 */
#define PIN_D0 19     /* P6.16 */
#define PIN_D1 18     /* P6.17 */
#define PIN_D2 14     /* P6.18 */
#define PIN_D3 15     /* P6.19 */
#define PIN_D4 40     /* P6.20 */
#define PIN_D5 41     /* P6.21 */
#define PIN_D6 17     /* P6.22 */
#define PIN_D7 16     /* P6.23 */
#define PORT_AL 6     /* GPIO6 */
#define AL_gp 24      /* P6.24-P6.31 */
#define AL_gm 0xFF    /* P6.24-P6.31 */
#define AL_vp 0       /* A0-A7 */
#define PIN_AL0 22    /* P6.24 */
#define PIN_AL1 23    /* P6.25 */
#define PIN_AL2 20    /* P6.26 */
#define PIN_AL3 21    /* P6.27 */
#define PIN_AL4 38    /* P6.28 */
#define PIN_AL5 39    /* P6.29 */
#define PIN_AL6 26    /* P6.30 */
#define PIN_AL7 27    /* P6.31 */
#define PORT_AM 7     /* GPIO7 */
#define AM_gp 0       /* P7.00-P7.03 */
#define AM_gm 0xF     /* P7.12-P7.15 */
#define AM_vp 8       /* A8-A11 */
#define PIN_AM8 10    /* P7.00 */
#define PIN_AM9 12    /* P7.01 */
#define PIN_AM10 11   /* P7.02 */
#define PIN_AM11 13   /* P7.03 */
#define PORT_AH 7     /* P7.16-P7.19 */
#define AH_gp 16      /* P7.16-P7.19 */
#define AH_gm 0xF     /* P7.16-P7.19 */
#define AH_vp 12      /* A12-A15 */
#define PIN_AH12 8    /* P7.16 */
#define PIN_AH13 7    /* P7.17 */
#define PIN_AH14 36   /* P7.18 */
#define PIN_AH15 37   /* P7.19 */
#define PIN_WDS 3     /* P9.05 */
#define PIN_SA 4      /* P9.06 */
#define PIN_SB 33     /* P9.07 */
#define PIN_F1 0      /* P6.03 */
#define PIN_F2 1      /* P6.04 */
#define PIN_XIN 5     /* P9.08 */
#define PIN_BREQ 29   /* P9.31 */
#define PIN_RDS 6     /* P7.10 */
#define PIN_F3 9      /* P7.11 */
#define PIN_HOLD 32   /* P7.12 */
#define PIN_RST 28    /* P8.18 */
#define PIN_ENIN 31   /* P8.22 */
#define PIN_ENOUT 30  /* P8.23 */
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
