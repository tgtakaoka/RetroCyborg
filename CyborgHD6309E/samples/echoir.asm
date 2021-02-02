        cpu     6809
        include "mc6809.inc"
        
;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     $DF00
        include "mc6850.inc"

        org     $2000

rx_queue_size:  equ     128
rx_queue:       rmb     rx_queue_size
RX_INT_TX_NO:   equ     WSB_8N1_gc|RIEB_bm

        org     $1000
stack:  equ     *

        org     $1000
initialize:
        lds     #stack
        ldx     #rx_queue
        ldb     #rx_queue_size
        lbsr    queue_init
        ;; initialize ACIA
        lda     #CDS_RESET_gc   ; Master reset
        sta     ACIA_control
        lda     #RX_INT_TX_NO
        sta     ACIA_control
        andcc   #~CC_IRQ        ; Clear IRQ mask

        ldx     #rx_queue
receive_loop:
        orcc    #CC_IRQ         ; Set IRQ mask
        lbsr    queue_remove
        andcc   #~CC_IRQ        ; Clear IRQ mask
        bcc     receive_loop
transmit_loop:
        ldb     ACIA_status
        bitb    #TDRE_bm
        beq     transmit_loop
transmit_data:
        sta     ACIA_data
        cmpa    #$0d
        bne     receive_loop
        lda     #$0a
        bra     transmit_loop

        include "queue.inc"

isr_irq:
        ldb     ACIA_status
        bitb    #IRQF_bm
        beq     isr_irq_return
isr_irq_receive:
        bitb    #RDRF_bm
        beq     isr_irq_recv_end
        lda     ACIA_data
        ldx     #rx_queue
        lbsr    queue_add
isr_irq_recv_end:
isr_irq_return:
        rti

        org     $FFF8
        fdb     isr_irq

        org     $FFFE
        fdb     initialize
