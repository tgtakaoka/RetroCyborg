        cpu     6801
        include "mc6801.inc"

        org     $2000

rx_queue_size:  equ     128
rx_queue:       rmb     rx_queue_size

;;; SCI: Enable Rx interrupt
RX_INT_TX_NO:   equ     TRCSR_TE_bm|TRCSR_RE_bm|TRCSR_RIE_bm

        org     $1000
stack:  equ     *-1             ; MC6801's SP is post-decrement/pre-increment

        org     $1000
initialize:
        lds     #stack
        ldx     #rx_queue
        ldab    #rx_queue_size
        jsr     queue_init
        ;; Initialize SCI
        ldaa    #CCFS_NRZ_gc|SS_DIV16_gc
        staa    RMCR            ; set NRZ and E/16
        ldaa    #RX_INT_TX_NO
        staa    TRCSR           ; Enable Tx and Rx/Interrupt
        cli                     ; Enable IRQ

        ldx     #rx_queue
receive_loop:
        sei                     ; Disable IRQ
        jsr     queue_remove
        cli                     ; Enable IRQ
        bcc     receive_loop
transmit_loop:
        ldab    TRCSR
        bitb    #TRCSR_TDRE_bm
        beq     transmit_loop
transmit_data:
        staa    SCTDR
        cmpa    #$0d
        bne     receive_loop
        ldaa    #$0a
        bra     transmit_loop

        include "queue.inc"

isr_sci:
        ldab    TRCSR
	bitb    #TRCSR_ORFE_bm
        bne     isr_sci_error
        bitb    #TRCSR_RDRF_bm
        beq     isr_sci_return
isr_sci_receive:
        ldaa    SCRDR
        ldx     #rx_queue
        jsr     queue_add
isr_sci_return:
        rti
isr_sci_error:
        ldab    SCRDR          ; clear ORFE
        bra     isr_sci_return

        org     VEC_SCI
        fdb     isr_sci

        org     VEC_RESET
        fdb     initialize
