        cpu     6811
        include "mc68hc11d.inc"

        org     $2000

rx_queue_size:  equ     128
rx_queue:       rmb     rx_queue_size

;;; SCI: Enable Rx interrupt
RX_INT_TX_NO:   equ     SCCR2_TE_bm|SCCR2_RE_bm|SCCR2_RIE_bm

        org     $1000
stack:  equ     *-1             ; MC6801's SP is post-decrement/pre-increment

        org     $1000
initialize:
        lds     #stack
        ldx     #rx_queue
        ldab    #rx_queue_size
        jsr     queue_init
        ;; Initialize SCI
        clr     SCCR1           ; 8bit 1stop
        ldaa    #BAUD_SCP1_gc|BAUD_SCR1_gc ; E/16
        staa    BAUD
        ldaa    #RX_INT_TX_NO
        staa    SCCR2           ; Enable Tx and Rx/Interrupt
        cli                     ; Enable IRQ

        ldx     #rx_queue
receive_loop:
        sei                     ; Disable IRQ
        jsr     queue_remove
        cli                     ; Enable IRQ
        bcc     receive_loop
transmit_loop:
        ldab    SCSR
        bitb    #SCSR_TDRE_bm
        beq     transmit_loop
transmit_data:
        staa    SCDR
        cmpa    #$0d
        bne     receive_loop
        ldaa    #$0a
        bra     transmit_loop

        include "queue.inc"

isr_sci:
        ldab    SCSR
	bitb    #SCSR_OR_bm|SCSR_NF_bm|SCSR_FE_bm
        bne     isr_sci_error
        bitb    #SCSR_RDRF_bm
        beq     isr_sci_return
isr_sci_receive:
        ldaa    SCDR
        ldx     #rx_queue
        jsr     queue_add
isr_sci_return:
        rti
isr_sci_error:
        ldab    SCDR            ; clear OR/NF/FE
        bra     isr_sci_return

        org     VEC_SCI
        fdb     isr_sci

        org     VEC_RESET
        fdb     initialize
