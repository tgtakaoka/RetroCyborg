;;; -*- mode: asm; mode: flyspell-prog; -*-
        cpu     z88
        option  "reg-alias", "disable"
        option  "optimize-index", "enable"

        include "z88.inc"

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:  equ     %FF00
USARTD: equ     0               ; Data register
USARTS: equ     1               ; Status register
USARTC: equ     1               ; Control register
        include "i8251.inc"

        org     %1000
stack:  equ     $

        org     ORG_RESET
        jp      init_config

        org     %100
        setrp   %C0
init_config:
        ld      EMT, #EMT_STACK_DM ; stack is on external data memory
        ld      P0M, #P0M_ADRH     ; Port 0 is address bus high
        ld      PM, #PM_P1BUS | PM_DM ; Port 1 is data bus and address bus low
        ldw     SPH, #stack

init_usart:
        ldw     rr12, #USART
        clr     r0
        lde     USARTC(rr12), r0
        lde     USARTC(rr12), r0
        lde     USARTC(rr12), r0 ; safest way to sync mode
        ld      r0, #CMD_IR_bm
        lde     USARTC(rr12), r0 ; reset
        nop
        nop
        ld      r0, #MODE_STOP1_gc LOR MODE_LEN8_gc LOR MODE_BAUD_X16
        lde     USARTC(rr12), r0 ; async 1stop 8data x16
        nop
        nop
        ld      r0, #CMD_RTS_bm LOR CMD_DTR_bm LOR CMD_ER_bm LOR CMD_RxEN_bm LOR CMD_TxEN_bm
        lde     USARTC(rr12), r0 ; RTS/DTR, error reset, Rx enable, Tx enable

        call    mandelbrot
        db      %7F

putchar:
        push    r13
        push    r12
        ldw     rr12, #USART
putchar_loop:
        lde     r1, USARTS(rr12)
        tm      r1, #ST_TxRDY_bm
        jr      z, putchar_loop
        lde     USARTD(rr12), r0
        cp      r0, #%0D
        jr      nz, putchar_end
        ld      r0, #%0A
        jr      putchar_loop
putchar_end:
        pop     r12
        pop     r13
        ret

        include "mandelbrot.inc"
        include "arith.inc"

        end
