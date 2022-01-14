        cpu     6502
        include "mos6502.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     $DF00
        include "mc6850.inc"

        org     0
queue_tmp:
        rmb     2
rx_queue:
        rmb     queue_buf
        fdb     $2000
rx_queue_size:  equ     128
RX_INT_TX_NO:   equ     WSB_8N1_gc|RIEB_bm

stack:  equ     $01ff

        org     $1000
initialize:
        ldx     #stack
        txs
        cld                     ; clear decimal flag
        ldx     #rx_queue
        lda     #rx_queue_size
        jsr     queue_init
        ;; initialize ACIA
        lda     #CDS_RESET_gc   ; Master reset
        sta     ACIA_control
        lda     #RX_INT_TX_NO
        sta     ACIA_control
        cli                     ; Enable IRQ

        ldx     #rx_queue
receive_loop:
        sei                     ; Disable IRQ
        jsr     queue_remove
        cli                     ; Enable IRQ
        bcs     receive_loop
        pha
transmit_loop:
        lda     ACIA_status
        and     #TDRE_bm
        beq     transmit_loop
transmit_data:
        pla
        sta     ACIA_data
        cmp     #$0d
        bne     receive_loop
        lda     #$0a
        pha
        bne     transmit_loop

        include "queue.inc"

isr_irq:
        cld                     ; clear decimal flag
        pha                     ; save A
        txa
        pha                     ; save X
        tya
        pha                     ; save Y
        lda     ACIA_status
        tax
        and     #IRQF_bm
        beq     isr_irq_return
isr_irq_receive:
        txa
        and     #RDRF_bm
        beq     isr_irq_recv_end
        lda     ACIA_data
        ldx     #rx_queue
        jsr     queue_add
isr_irq_recv_end:
isr_irq_return:
        pla                     ; restore Y
        tay
        pla                     ; restore X
        tax
        pla                     ; restore Y
        rti

        org     VEC_IRQ
        fdb     isr_irq

        org     VEC_RESET
        fdb     initialize
