;;; -*- mode: asm; mode: flyspell-prog; -*-
        cpu     z88
        option  "reg-alias", "disable"
        option  "optimize-index", "enable"

        include "z88.inc"

;;; i8251 Universal Synchronous/Asynchronous Receiver/Transmitter
USART:          equ     %FF00
USARTD:         equ     0       ; Receive/Transmit data
USARTS:         equ     1       ; Status register
USARTC:         equ     1       ; Control register
USARTRI:        equ     2       ; Receive interrupt name (IRQ0~2)
USARTTI:        equ     3       ; Transmit interrupt name (IRQ0~2)
        include "i8251.inc"

        org     %2000
rx_queue_size:  equ     128
rx_queue:       ds      rx_queue_size

        org     %1000
stack:  equ     $

        org     VEC_IRQ0_P23
        dw      isr_intr

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
        ld      r0, #INTR_IRQ0
        lde     USARTRI(rr12), r0 ; enable RxRDY interrupt using IRQ0
        ld      r0, #INTR_NONE
        lde     USARTTI(rr12), r0 ; disable TxRDY interrupt

        ld      P2BM, #P2M_INTR_gm SHL 2 ; P23=input, interrupt enabled
        ld      IPR, #IPR_ABC ; (IRQ0 > IRQ1) > (IRQ2,3,4) > (IRQ5,6,7)
        ld      IMR, #IMR_IRQ0  ; enable IRQ0
        ei

receive_loop:
        di                      ; Disable INTR
        call    queue_remove
        ei                      ; Enable INTR
        jr      nc, receive_loop
        or      r0,r0
        jr      nz,transmit_loop
        db      %7F             ; unknown instruction
transmit_loop:
        lde     r1, USARTS(rr12)
        tm      r1, #ST_TxRDY_bm
        jr      z, transmit_loop
transmit_data:
        lde     USARTD(rr12), r0
        cp      r0, #%0D
        jr      nz, receive_loop
        ld      r0, #%0A
        jr      transmit_loop

        include "queue.inc"

        setrp   -1
isr_intr:
        ld      P2AIP, #1 SHL 5 ; clear P23 IRQ0
        push    r0
        lde     r0, USARTS(rr12)
isr_intr_receive:
        tm      r0, #ST_RxRDY_bm
        jr      z, isr_intr_recv_end
        lde     r0, USARTD(rr12)
        call    queue_add
isr_intr_recv_end:
        pop     r0
        iret
