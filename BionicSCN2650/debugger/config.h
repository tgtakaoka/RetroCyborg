#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <Arduino.h>
#include <stdint.h>

#define VERSION_TEXT "0.1"
#define Console Serial

#define USART_BASE_ADDR 0xFF00

/**
 * BANK0:R240:%F0 Write only; [P0M] Port 0 Mode
 * D7-D0: Address or I/O
 *   0001_1111 = P07-P05, A12-A8
 */

/**
 * BANK0:R241:%F1 Write only; [PM] Port Mode
 * D6-5: Port 1 mode
 *   1x = Address/Data
 *   01 = Input
 *   00 = Output
 * D3: DM enable
 *   1 = Enable DM P35
 *   0 = Disable DM P35
 */
#define Z88_DATA_MEMORY 0

/**
 * BANK0:R254:%F8 Write only; [EMT] External Memory Timing
 * D7: Wait input selection
 *   1 = External #WAIT input
 *   0 = I/O
 * D6: Slow memory timing
 *   1 = Enabled
 *   0 = Disabled
 * D5-4: Program memory automatic waits
 *   0~3 = waits
 * D3-2: Data memory automatic waits
 *   0~3 = waits
 * D1: Stack location selection
 *   1 = Data memory
 *   0 = Register file
 * D0: DMA select
 *   1 = Data memory
 *   0 = Register file
 */
#define Z88_EXTERNAL_STACK 1

#if defined(ARDUINO_TEENSY41)
#define PORT_DATA 6    /* GPIO6 */
#define DATA_gp 16     /* P6.16-P6.23 */
#define DATA_gm 0xFF   /* P6.16-P6.23 */
#define DATA_vp 0      /* D0-D7 */
#define PIN_DATA0 19   /* P6.16 */
#define PIN_DATA1 18   /* P6.17 */
#define PIN_DATA2 14   /* P6.18 */
#define PIN_DATA3 15   /* P6.19 */
#define PIN_DATA4 40   /* P6.20 */
#define PIN_DATA5 41   /* P6.21 */
#define PIN_DATA6 17   /* P6.22 */
#define PIN_DATA7 16   /* P6.23 */
#define PORT_ADDR 6    /* GPIO6 */
#define ADDR_gp 16     /* P6.24-P6.31 */
#define ADDR_gm 0xFFFF /* P6.24-P6.31 */
#define ADDR_vp 0      /* A8-A15 */
#define PIN_ADDR0 19   /* P6.16 */
#define PIN_ADDR1 18   /* P6.17 */
#define PIN_ADDR2 14   /* P6.18 */
#define PIN_ADDR3 15   /* P6.19 */
#define PIN_ADDR4 40   /* P6.20 */
#define PIN_ADDR5 41   /* P6.21 */
#define PIN_ADDR6 17   /* P6.22 */
#define PIN_ADDR7 16   /* P6.23 */
#define PIN_ADDR8 22   /* P6.24 */
#define PIN_ADDR9 23   /* P6.25 */
#define PIN_ADDR10 20  /* P6.26 */
#define PIN_ADDR11 21  /* P6.27 */
#define PIN_ADDR12 38  /* P6.28 */
#define PIN_ADDR13 39  /* P6.29 */
#define PIN_ADDR14 26  /* P6.30 */
#define PIN_ADDR15 27  /* P6.31 */
#define PIN_AS 2       /* P9.04 */
#define PIN_RW 3       /* P9.05 */
#define PIN_IRQ1 4     /* P9.06 */
#define PIN_IRQ0 33    /* P9.07 */
#define PIN_TXD 0      /* P6.03 */
#define PIN_RXD 1      /* P6.04 */
#define PIN_XTAL1 5    /* P9.08 */
#define PIN_DS 29      /* P9.31 */
#define PIN_IRQ2 6     /* P7.10 */
#define PIN_WAIT 32    /* P7.12 */
#define PIN_RESET 28   /* P8.18 */
#define PIN_DM 30      /* P8.23 */
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
