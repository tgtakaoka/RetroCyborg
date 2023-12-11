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
tx_queue_size:  equ     128
tx_queue:       rs      tx_queue_size

        org     0
        jmp     init

rx_vec:
        jmp     isr_intr_rx

tx_vec:
        jmp     isr_intr_tx

        org     H'0200'
init:
        pi      init_stack
        dci     rx_queue
        lr      Q, DC
        li      rx_queue_size
        lr      0, A
        pi      call
        da      queue_init
        dci     tx_queue
        lr      Q, DC
        li      tx_queue_size
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
;;; enable TxRDY interrupt
        li      tx_vec
        out     USARTTV

        pi      call
        da      mandelbrot
        dc      H'2F'

;;; Get character
;;; @return 0
;;; @return CC.C 0 if no character
getchar:
        ei
        nop
        di
        dci     rx_queue
        lr      Q, DC
        jmp     queue_remove

;;; Put character
;;; @param 0
;;; @clobber 0
putchar:
        ei
        nop
        di
        dci     tx_queue
        lr      Q, DC
        pi      call
        da      queue_add
        bnc     putchar         ; branch if queue is full
        in      USARTTV
        oi      0
        bnz     putchar_exit    ; already enabled
        li      tx_vec
        out     USARTTV
putchar_exit:
        jmp     return

isr_intr_rx:
        lr      K, P
        pi      pushK
        pi      push0
        in      USARTS
        ni      ST_RxRDY_bm
        bz      isr_intr_end
        in      USARTD
        lr      0, A
        dci     rx_queue
        lr      Q, DC
        pi      call
        da      queue_add
isr_intr_end:
        pi      pop0
        jmp     return

isr_intr_tx:
        lr      K, P
        pi      pushK
        pi      push0
        dci     tx_queue
        lr      Q, DC
        pi      call
        da      queue_remove
        bnc     isr_intr_send_empty
        lr      A, 0
        out     USARTD          ; send character
        pi      pop0
        jmp     return
isr_intr_send_empty:
        clr
        out     USARTTV         ; disable Tx interrupt
        pi      pop0
        jmp     return

;;; Print unsigned 16-bit integer as decimal
;;; @param 0:1 value
;;; @clobber 0 1 4 5 6 7 A
print_uint16:
        lr      A, 0
        oi      0
        bnz     print_uint16_inner
        lr      A, 1
        oi      0
        bz      print_uint16_zero
print_uint16_inner:
        lr      A, 0
        lr      4, A
        lr      A, 1
        lr      5, A            ; 4:5=value
print_uint16_loop:
        lr      A, 4
        oi      0
        bnz     print_uint16_digit
        lr      A, 5
        oi      0
        bnz     print_uint16_digit
        jmp     return
print_uint16_digit:
        clr
        lr      6, A
        li      10
        lr      7, A            ; 6:7=10
        pi      call
        da      udiv16          ; 4:5/6:7=4:5...6:7
        lr      A, 7
        lr      0, A
        pi      push0           ; push reminder
        pi      call
        da      print_uint16_loop
        pi      pop0
print_uint16_zero:
        li      C'0'
        as      0
        lr      0, A
        jmp     putchar

;;; Print signed 16-bit integer as decimal
;;; @param 0:1 value
;;; @clobber 0 1 4 5 6 7 A
print_int16:
        lr      A, 0
        oi      0
        bp      print_uint16
        pi      push0
        li      C'-'
        lr      0, A
        pi      call
        da      putchar         ; print '-'
        pi      pop0
        lr      A, 0
        com
        lr      0, A
        lr      A, 1
        com
        inc
        lr      1, A
        lr      A, 0
        lnk
        lr      0, A            ; 0:1=-value
        br      print_uint16

print_2:
        pi      call
        da      putchar
        li      C'='
        lr      0, A
        pi      call
        da      putchar
        lr      A, 2
        lr      IS, A
        lr      A, I
        lr      0, A
        lr      A, S
        lr      1, A
        pi      call
        da      print_int16
        li      C' '
        lr      0, A
        jmp     putchar

newline:        
        li      H'0D'
        lr      0, A
        pi      call
        da      putchar
        li      H'0A'
        lr      0,A
        jmp     putchar

        include "stack.inc"
        include "queue.inc"
        include "arith.inc"
        include "mandelbrot.inc"
