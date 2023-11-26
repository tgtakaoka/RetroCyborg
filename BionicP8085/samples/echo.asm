        cpu     8085
        include "i8085.inc"

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:  equ     00H
USARTD: equ     USART+0         ; Data register
USARTS: equ     USART+1         ; Status register
USARTC: equ     USART+1         ; Control register
        include "i8251.inc"

        org     1000H
stack:  equ     $

        org     ORG_RESET
        lxi     sp, stack

init_usart:
        xra     a               ; clear A
        out     USARTC
        out     USARTC
        out     USARTC          ; safest way to sync mode
        mvi     a, CMD_IR_bm
        out     USARTC          ; reset
        nop
        nop
        mvi     a, MODE_STOP1_gc|MODE_LEN8_gc|MODE_BAUD_X16
        out     USARTC          ; async 1stop 8data x16
        nop
        nop
        mvi     a, CMD_RTS_bm|CMD_DTR_bm|CMD_ER_bm|CMD_RxEN_bm|CMD_TxEN_bm
        out     USARTC          ; RTS/DTR, error reset, Rx enable, Tx enable

receive_loop:
        in      USARTS
        ani     ST_RxRDY_bm
        jz      receive_loop
receive_data:
        in      USARTD
        mov     b, a
        ora     a
        jnz     transmit_loop
        hlt
transmit_loop:
        in      USARTS
        ani     ST_TxRDY_bm
        jz      transmit_loop
transmit_data:
        mov     a, b
        out     USARTD
        cpi     0DH
        jnz     receive_loop
        mvi     b, 0AH
        jmp     transmit_loop
