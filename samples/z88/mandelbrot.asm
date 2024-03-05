 ;;; -*- mode: asm; mode: flyspell-prog; -*-
        cpu     z88
        option  "reg-alias", "disable"
        option  "optimize-index", "enable"

        include "z88.inc"

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:          equ     %FF00
USARTD:         equ     0       ; Data register
USARTS:         equ     1       ; Status register
USARTC:         equ     1       ; Control register
USARTRI:        equ     2       ; Receive interrupt name (IRQ0~2)
USARTTI:        equ     3       ; Transmit interrupt name (IRQ0~2)
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

        org     %1000
stack:  equ     $

        org     VEC_IRQ0_P23
        dw      isr_intr_rx

        org     VEC_IRQ1_P21
        dw      isr_intr_tx

        org     ORG_RESET
        setrp   %C0
init_config:
        ld      EMT, #EMT_STACK_DM ; stack is on external data memory
        ld      P0M, #P0M_ADRH     ; Port 0 is address bus high
        ld      PM, #PM_P1BUS | PM_DM ; Port 1 is data bus and address bus low
        ldw     SPH, #stack
        ldw     rr2, #rx_queue
        ld      r1, #rx_queue_size
        call    queue_init
        ldw     rr2, #tx_queue
        ld      r1, #tx_queue_size
        call    queue_init

init_usart:
        ldw     rr2, #USART
        clr     r0
        lde     USARTC(rr2), r0
        lde     USARTC(rr2), r0
        lde     USARTC(rr2), r0 ; safest way to sync mode
        ld      r0, #CMD_IR_bm
        lde     USARTC(rr2), r0 ; reset
        nop
        nop
        ld      r0, #ASYNC_MODE
        lde     USARTC(rr2), r0 ; async 1stop 8data x16
        nop
        nop
        ld      r0, #RX_EN_TX_DIS
        lde     USARTC(rr2), r0 ; RTS/DTR, error reset, Rx enable, Tx disable
        ld      r0, #INTR_IRQ0
        lde     USARTRI(rr2), r0 ; enable RxRDY interrupt using IRQ0
        ld      r0, #INTR_IRQ1
        lde     USARTTI(rr2), r0 ; enable TxRDY interrupt using IRQ1

        ld      P2BM, #P2M_INTR_gm SHL 2 ; P23=input, interrupt enabled
        ld      P2AM, #P2M_INTR_gm SHL 2 ; P21=input, interrupt enabled
        ld      IPR, #IPR_ABC ; (IRQ0 > IRQ1) > (IRQ2,3,4) > (IRQ5,6,7)
        ld      IMR, #IMR_IRQ0 LOR IMR_IRQ1 ; enable IRQ0, IRQ1
        ei

        call    mandelbrot
        db      %7F

;;; Get character
;;; @return r0
;;; @return FLAGS.C 0 if no character
getchar:
        push    r3
        push    r2
        ldw     rr2, #rx_queue
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
        push    r3
        push    r2
        ldw     rr2, #tx_queue
putchar_retry:
        di
        call    queue_add
        ei
        jr      nc, putchar_retry ; branch if queue is full
        ldw     rr2, #USART
        ld      r0, #RX_EN_TX_EN
        lde     USARTC(rr2), r0 ; enable TX
putchar_exit:
        pop     r2
        pop     r3
        pop     r0
        ret

;;; Put newline
;;; @clobber r0
newline:
        ld      r0, #%0D
        call    putchar
        ld      r0, #%0A
        jr      putchar

;;; Put space
;;; @clobber r0
putspace:
        ld      r0, #' '
        jr      putchar

        include "mandelbrot.inc"
        include "arith.inc"
        include "queue.inc"

        setrp   -1
isr_intr_rx:
        ld      P2AIP, #1 SHL 5 ; clear P23 IRQ0
        push    r0
        push    r2
        push    r3
        ldw     rr2, #USART
        lde     r0, USARTS(rr2)
        and     r0, #ST_RxRDY_bm
        jr      z, isr_intr_rx_exit
        lde     r0, USARTD(rr2)
        ldw     rr2, #rx_queue
        call    queue_add
isr_intr_rx_exit:
        pop     r3
        pop     r2
        pop     r0
        iret

isr_intr_tx:
        ld      P2AIP, #1 SHL 1 ; clear P21 IRQ1
        push    r0
        push    r2
        push    r3
        ldw     rr2, #USART
        lde     r0, USARTS(rr2)
        and     r0, #ST_TxRDY_bm
        jr      z, isr_intr_tx_exit
        ldw     rr2, #tx_queue
        call    queue_remove
        ldw     rr2, #USART
        jr      nc, isr_intr_send_empty
        lde     USARTD(rr2), r0
isr_intr_tx_exit:
        pop     r3
        pop     r2
        pop     r0
        iret
isr_intr_send_empty:
        ld      r0, #RX_EN_TX_DIS
        lde     USARTC(rr2), r0 ; disable Tx
        pop     r3
        pop     r2
        pop     r0
        iret

        end
