;;; -*- mode: asm; mode: flyspell-prog; -*-
        cpu     z86c
        option  "reg-alias", "disable"

        include "z8.inc"

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:  equ     %FF00
USARTD: equ     USART+0         ; Data register
USARTS: equ     USART+1         ; Status register
USARTC: equ     USART+1         ; Control register
        include "i8251.inc"

        org     %1000
stack:  equ     $

        org     ORG_RESET
        jp      init_config

        org     %0100
init_config:
        srp     #%F0
        setrp   %F0
        ;; Stack is on external memory
        ld      P01M, #P01M_P0ADDR LOR P01M_P1DATA
        ld      P2M, #%FF       ; Port 2 is input
        ld      SPH, #HIGH stack
        ld      SPL, #LOW stack

init_usart:
        srp     #%10
        setrp   %10
        ld      r12, #HIGH USARTC
        ld      r13, #LOW USARTC
        clr     r0
        lde     @rr12, r0
        lde     @rr12, r0
        lde     @rr12, r0       ; safest way to sync mode
        ld      r0, #CMD_IR_bm
        lde     @rr12, r0       ; reset
        nop
        nop
        ld      r0, #MODE_STOP1_gc LOR MODE_LEN8_gc LOR MODE_BAUD_X16
        lde     @rr12, r0       ; async 1stop 8data x16
        nop
        nop
        ld      r0, #CMD_RTS_bm LOR CMD_DTR_bm LOR CMD_ER_bm LOR CMD_RxEN_bm LOR CMD_TxEN_bm
        lde     @rr12, r0 ; RTS/DTR, error reset, Rx enable, Tx enable
        ld      r8, #HIGH USARTD
        ld      r9, #LOW USARTD

        call      mandelbrot
        halt

putchar:
        push    r0
        incw    rr8
putchar_loop:
        lde     r0, @rr8
        tm      r0, #ST_TxRDY_bm
        jr      z, putchar_loop
        pop     r0
        decw    rr8
        lde     @rr8, r0
        ret

        include "mandelbrot.inc"
        include "arith.inc"

        end
