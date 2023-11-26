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
        jmp     init_usart

        org     0100H
init_usart:
        lxi     sp, stack
        xra     A               ; clear A
        out     USARTC
        out     USARTC
        out     USARTC          ; safest way to sync mode
        mvi     A, CMD_IR_bm
        out     USARTC          ; reset
        nop
        nop
        mvi     A, MODE_STOP1_gc|MODE_LEN8_gc|MODE_BAUD_X16
        out     USARTC          ; async 1stop 8data x16
        nop
        nop
        mvi     A, CMD_RTS_bm|CMD_DTR_bm|CMD_ER_bm|CMD_RxEN_bm|CMD_TxEN_bm
        out     USARTC          ; RTS/DTR, error reset, Rx enable, Tx enable

        call    mandelbrot
        hlt

putchar:
        push    PSW
putchar_loop:
        in      USARTS
        ani     ST_TxRDY_bm
        jz      putchar_loop
        pop     PSW
        out     USARTD
        ret

newline:
        push    PSW
        mvi     A, 0DH
        call    putchar
        mvi     A, 0AH
        call    putchar
        pop     PSW
        ret

putspace:
        push    PSW
        mvi     A, ' '
        call    putchar
        pop     PSW
        ret

        include "mandelbrot.inc"
        include "arith.inc"

        end
