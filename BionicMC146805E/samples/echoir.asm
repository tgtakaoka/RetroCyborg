        cpu     6805
        include "mc6805.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     $17F8
        include "mc6850.inc"

        org     $2000

rx_queue_size:  equ     16
RX_INT_TX_NO:   equ     WSB_8N1_gc|RIEB_bm

        org     $20
save_a:
        rmb     1
rx_queue:
        rmb     rx_queue_size

        org     $0100
initialize:
        ldx     #rx_queue
        lda     #rx_queue_size
        jsr     queue_init
        ;; initialize ACIA
        lda     #CDS_RESET_gc   ; Master reset
        sta     ACIA_control
        lda     #RX_INT_TX_NO
        sta     ACIA_control
        cli                     ; Enable IRQ

loop:
        ldx     #rx_queue
        sei                     ; Disable IRQ
        jsr     queue_remove
        cli                     ; Enable IRQ
        bcc     loop
echo:
        bsr     putchar
        cmp     #$0D            ; carriage return
        bne     loop
        lda     #$0A            ; newline
        bra     echo

putchar:
        sta     save_a
transmit_loop:
        lda     ACIA_status
        bit     #TDRE_bm
        beq     transmit_loop
transmit_data:
        lda     save_a
        sta     ACIA_data
        rts

        include "queue.inc"

isr_irq:
        lda     ACIA_status
        bit     #IRQF_bm
        beq     isr_irq_return
isr_irq_receive:
        bit     #RDRF_bm
        beq     isr_irq_recv_end
        lda     ACIA_data
        ldx     #rx_queue
        jsr     queue_add
isr_irq_recv_end:
isr_irq_return:
        rti

        org     VEC_IRQ
        fdb     isr_irq

        org     VEC_RESET
        fdb     initialize
