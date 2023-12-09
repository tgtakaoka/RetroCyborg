;;; -*- mode: asm; mode: flyspell-prog; -*-
        cpu     f3850

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:          equ     0FFF0H
USARTD:         equ     USART+0 ; Receive/Transmit data
USARTS:         equ     USART+1 ; Srtatus register
USARTC:         equ     USART+1 ; Control register
USARTRV:        equ     USART+2 ; Receive interrupt vector (ORG_*)
USARTTV:        equ     USART+3 ; Transmit interrupt vector (ORG_*)
        include "i8251.inc"

        org     1000H
stack:  

        org     2000H
rx_queue_size:  equ     128
rx_queue:       rs      rx_queue_size

        org     0
        jmp     init

rx_vec:
        jmp     isr_intr
        
init:
        pi      init_stack
        dci     rx_queue
        lr      Q, DC
        li      rx_queue_size
        lr      0, A
        pi      call
        da      queue_init
init_usart:
        lis     0
        out     USARTC
        out     USARTC
        out     USARTC          ; safest way to sync mode
;;; reset
        li      CMD_IR_bm
        out     USARTC
        nop
        nop
;;;  async 1stop 8data x16
        li      MODE_STOP1_gc|MODE_LEN8_gc|MODE_BAUD_X16
        out     USARTC
        nop
        nop
;;;  RTS/DTR, error reset, Rx enable, Tx enable
        li      CMD_RTS_bm|CMD_DTR_bm|CMD_ER_bm|CMD_RxEN_bm|CMD_TxEN_bm
        out     USARTC
;;; enable RxRDY interrupt using INT0
        li      rx_vec
        out     USARTRV
;;; disable TxRDY interrupt
        lis     0
        out     USARTTV

receive_loop:
        ei                      ; Enable INTR
        nop
        di                      ; Disable INTR
        dci     rx_queue
        lr      Q, DC
        pi      call
        da      queue_remove
        bnc     receive_loop
        lr      A, 0
        oi      0
        bnz     transmit_loop
        dc      H'2F'           ; unknown instruction
transmit_loop:
        ei
        nop
        di
        in      USARTS
        ni      ST_TxRDY_bm
        bz      transmit_loop
transmit_data:
        lr      A, 0
        out     USARTD
        ci      H'0D'
        bnz     receive_loop
        lis     H'0A'
        lr      0, A
        br      transmit_loop

isr_intr:
        lr      K, P
        pi      pushK
        pi      push0
        in      USARTS
        ni      ST_RxRDY_bm
        bz      isr_intr_recv_end
        in      USARTD
        lr      0, A
        dci     rx_queue
        lr      Q, DC
        pi      call
        da      queue_add
isr_intr_recv_end:
        pi      pop0
        jmp     return

        include "stack.inc"
        include "queue.inc"
