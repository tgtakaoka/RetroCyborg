        cpu     6811
        include "mc68hc11d.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     $DF00
        include "mc6850.inc"

        org     $2000

rx_queue_size:  equ     128
rx_queue:       rmb     rx_queue_size
RX_INT_TX_NO:   equ     WSB_8N1_gc|RIEB_bm

        org     $1000
stack:  equ     *-1             ; MC6801's SP is post-decrement/pre-increment

        org     $1000
initialize:
        lds     #stack
        ldx     #rx_queue
        ldab    #rx_queue_size
        jsr     queue_init
        ;; initialize ACIA
        ldaa    #CDS_RESET_gc   ; Master reset
        staa    ACIA_control
        ldaa    #RX_INT_TX_NO
        staa    ACIA_control
        cli                     ; Enable IRQ

        ldy     #ACIA
        ldx     #rx_queue
receive_loop:
        sei                     ; Disable IRQ
        jsr     queue_remove
        cli                     ; Enable IRQ
        bcc     receive_loop
transmit_loop:
        brclr   0,y, #TDRE_bm, transmit_loop
transmit_data:
        staa    ACIA_data
        cmpa    #$0d
        bne     receive_loop
        ldaa    #$0a
        bra     transmit_loop

        include "queue.inc"

isr_irq:
        ldab    ACIA_status
        bitb    #IRQF_bm
        beq     isr_irq_return
isr_irq_receive:
        bitb    #RDRF_bm
        beq     isr_irq_recv_end
        ldaa    ACIA_data
        ldx     #rx_queue
        jsr     queue_add
isr_irq_recv_end:
isr_irq_return:
        rti

        org     VEC_IRQ
        fdb     isr_irq

        org     VEC_RESET
        fdb     initialize
