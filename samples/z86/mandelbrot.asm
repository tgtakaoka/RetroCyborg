;;; -*- mode: asm; mode: flyspell-prog; -*-
        cpu     z86c
        option  "reg-alias", "disable"

        include "z8.inc"

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:          equ     %FF00
USARTD:         equ     USART+0 ; Receive/Transmit data
USARTS:         equ     USART+1 ; Status register
USARTC:         equ     USART+1 ; Control register
USARTRI:        equ     USART+2 ; Receive interrupt name (IRQ0~2)
USARTTI:        equ     USART+3 ; Transmit interrupt name (IRQ0~2)
        include "i8251.inc"
;;; Async 1stop 8data x16
ASYNC_MODE:     equ     MODE_STOP1_gc LOR MODE_LEN8_gc LOR MODE_BAUD_X16
;;; RTS/DTR, error reset, Rx enable, Tx enable
RX_EN_TX_EN:    equ     CMD_RTS_bm LOR CMD_DTR_bm LOR CMD_ER_bm LOR CMD_RxEN_bm LOR CMD_TxEN_bm
RX_EN_TX_DIS:   equ     CMD_RTS_bm LOR CMD_DTR_bm LOR CMD_ER_bm LOR CMD_RxEN_bm

        org     %2000
rx_queue_size:  equ     128
rx_queue:       ds      rx_queue_size
tx_queue_size:  equ     128
tx_queue:       ds      tx_queue_size

        org     VEC_IRQ0
        dw      isr_intr_rx

        org     VEC_IRQ1
        dw      isr_intr_tx

        org     ORG_RESET
        jp      init_config

;;; work area for mandelbrot.inc
        org     %40
vF:     ds      2
vC:     ds      2
vD:     ds      2
vA:     ds      2
vB:     ds      2
vP:     ds      2
vQ:     ds      2
vS:     ds      2
vT:     ds      2
tmp:    ds      2
vY:     ds      2
vX:     ds      2
vI:     ds      2

        org     %1000
stack:  equ     $

init_config:
        srp     #%F0
        setrp   %F0
        ;; Stack is on external memory
        ld      P01M, #P01M_P0ADDR LOR P01M_P1DATA
        ld      P2M, #%FF       ; Port 2 is input
        ld      SPH, #HIGH stack
        ld      SPL, #LOW stack
        srp     #%10
        setrp   %10
        ld      r2, #HIGH rx_queue
        ld      r3, #LOW rx_queue
        ld      r1, #rx_queue_size
        call    queue_init
        ld      r2, #HIGH tx_queue
        ld      r3, #LOW tx_queue
        ld      r1, #tx_queue_size
        call    queue_init

init_usart:
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
        ld      r0, #ASYNC_MODE
        lde     @rr12, r0       ; async 1stop 8data x16
        nop
        nop
        ld      r0, #RX_EN_TX_DIS
        lde     @rr12, r0       ; RTS/DTR, error reset, Rx enable, Tx disable
        ld      r0, #INTR_IRQ0
        ld      r13, #LOW USARTRI
        lde     @rr12, r0       ; enable RxRDY interrupt using IRQ0
        ld      r0, #INTR_IRQ1
        ld      r13, #LOW USARTTI
        lde     @rr12, r0       ; enable TxRDY interrupt using IRQ1

        ld      IPR, #IPR_BCA LOR IPR_B02 LOR IPR_C14 LOR IPR_A35
        ;; enable IRQ0 and IRQ1
        ld      IMR, #IMR_IRQ0 LOR IMR_IRQ1
        ei

        call      mandelbrot
        halt

;;; Get character
;;; @return r0
;;; @return FLAGS.C 0 if no character
getchar:
        push    r3
        push    r2
        ld      r2, #HIGH rx_queue
        ld      r3, #LOW rx_queue
        di
        call    queue_remove
        ei
        pop     r2
        pop     r3
        ret

;;; Put character
;;; @param r0
putchar:
        push    r0
        push    r2
        push    r3
        ld      r2, #HIGH tx_queue
        ld      r3, #LOW tx_queue
putchar_retry:
        di
        call    queue_add
        ei
        jr      nc, putchar_retry ; branch if queue is full
        ld      r2, #HIGH USARTC
        ld      r3, #LOW USARTC
        ld      r0, #RX_EN_TX_EN
        lde     @rr2, r0        ; enable Tx
putchar_exit:
        pop     r3
        pop     r2
        pop     r0
        ret

newline:
        ld      r0, #%0D
        call    putchar
        ld      r0, #%0A
        jr      putchar

putspace:
        ld      r0, #' '
        jr      putchar

        include "mandelbrot.inc"
        include "arith.inc"
        include "queue.inc"

        setrp   -1
isr_intr_rx:
        push    r0
        push    r2
        push    r3
        ld      r2, #HIGH USARTS
        ld      r3, #LOW USARTS
        lde     r0, @rr2        ; USARTS
        and     r0, #ST_RxRDY_bm
        jr      z, isr_intr_rx_exit
        ld      r3, #LOW USARTD
        lde     r0, @rr2        ; USARTD
        ld      r2, #HIGH rx_queue
        ld      r3, #LOW rx_queue
        call    queue_add
isr_intr_rx_exit:
        pop     r3
        pop     r2
        pop     r0
        iret

isr_intr_tx:
        push    r0
        push    r2
        push    r3
        ld      r2, #HIGH USARTS
        ld      r3, #LOW USARTS
        lde     r0, @rr2        ; USARTS
        and     r0, #ST_TxRDY_bm
        jr      z, isr_intr_tx_exit
        ld      r2, #HIGH tx_queue
        ld      r3, #LOW tx_queue
        call    queue_remove
        ld      r2, #HIGH USARTD
        ld      r3, #LOW USARTD
        jr      nc, isr_intr_send_empty
        lde     @rr2, r0        ; USARTD
isr_intr_tx_exit:
        pop     r3
        pop     r2
        pop     r0
        iret
isr_intr_send_empty:
        ld      r3, #LOW USARTC
        ld      r0, #RX_EN_TX_DIS
        lde     @rr2, r0        ; disable Tx
        pop     r3
        pop     r2
        pop     r0
        iret

        end
