        cpu     6805
        include "mc6805.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     $17F8
        include "mc6850.inc"

rx_queue_size:  equ     16
tx_queue_size:  equ     32
RX_INT_TX_NO:   equ     WSB_8N1_gc|RIEB_bm
RX_INT_TX_INT:  equ     WSB_8N1_gc|RIEB_bm|TCB_EI_gc

        org     $20
save_a:
        rmb     1
rx_queue:
        rmb     rx_queue_size
tx_queue:
        rmb     tx_queue_size
tx_int_control:
        rmb     1
isr_irq_work:
        rmb     1

        org     $0100
initialize:
        ldx     #rx_queue
        lda     #rx_queue_size
        jsr     queue_init
        ldx     #tx_queue
        lda     #tx_queue_size
        jsr     queue_init
        ;; initialize ACIA
        lda     #CDS_RESET_gc   ; master reset
        sta     ACIA_control
        lda     #RX_INT_TX_NO
        sta     ACIA_control
        clr     tx_int_control  ; disable Tx interrupt
        cli                     ; enable IRQ

loop:
        bsr     getchar
        bcc     loop
        sta     save_a
        bsr     putchar         ; echo
        lda     #' '            ; space
        bsr     putchar
        lda     save_a
        bsr     put_hex8        ; print in hex
        lda     #' '            ; space
        bsr     putchar
        lda     save_a
        bsr     put_bin8        ; print in binary
        bsr     newline
        bra     loop

;;; Put newline
;;; @clobber A
newline:
        lda     #$0D
        bsr     putchar
        lda     #$0A
        bra     putchar

;;; Print uint8_t in hex
;;; @param A(save_a) uint8_t value to be printed in hex.
put_hex8:
        sta     save_a
        lda     #'0'
        bsr     putchar
        lda     #'x'
        bsr     putchar
        lda     save_a
        lsra
        lsra
        lsra
        lsra
        bsr     put_hex4
        lda     save_a
put_hex4:
        and     #$0f
        cmp     #10
        blo     put_hex8_dec
        add     #'A'-10
        bra     putchar
put_hex8_dec:
        add     #'0'
        bra     putchar

;;; Print uint8_t in binary
;;; @param A(save_a) uint8_t value to be printed in binary.
put_bin8:
        sta     save_a
        lda     #'0'
        bsr     putchar
        lda     #'b'
        bsr     putchar
        bsr     put_bin4
        bsr     put_bin4
        lda     save_a
        rts
put_bin4:
        bsr     put_bin2
put_bin2:
        bsr     put_bin1
put_bin1:
        clc
        lda     #'0'
        lsl     save_a
        bcc     put_bin0
        inc     save_a          ; rotate save_a
        inca
put_bin0:
        bra     putchar

;;; Get character
;;; @clobber X
;;; @return A
;;; @return CC.C 0 if no char received
getchar:
        ldx     #rx_queue
        sei                     ; disable IRQ
        jsr     queue_remove
        cli                     ; enable IRQ
        rts

;;; Put character
;;; @param A
;;; @clobber A,X
putchar:
        ldx     #tx_queue
putchar_retry:
        sei                     ; disable IRQ
        jsr     queue_add
        cli                     ; enable IRQ
        bcc     putchar_retry   ; branch if queue is full
        sei                     ; disable IRQ
        tst     tx_int_control
        bne     putchar_exit
        lda     #RX_INT_TX_INT  ; enable Tx interrupt
        sta     ACIA_control
        com     tx_int_control
putchar_exit:
        cli                     ; enable IRQ
        rts

        include "queue.inc"

isr_irq:
        lda     ACIA_status
        sta     isr_irq_work
        bit     #IRQF_bm
        beq     isr_irq_exit
        bit     #FERR_bm|OVRN_bm|PERR_bm
        beq     isr_irq_receive
        lda     ACIA_data       ; reset error flags
isr_irq_receive:
        lda     isr_irq_work
        bit     #RDRF_bm
        beq     isr_irq_send
        lda     ACIA_data       ; receive character
        ldx     #rx_queue
        jsr     queue_add
isr_irq_send:
        lda     isr_irq_work
        bit     #TDRE_bm
        beq     isr_irq_exit
        ldx     #tx_queue
        jsr     queue_remove
        bcc     isr_irq_send_empty
        sta     ACIA_data       ; send character
isr_irq_exit:
        rti
isr_irq_send_empty:
        lda     #RX_INT_TX_NO
        sta     ACIA_control    ; disable Tx interrupt
        clr     tx_int_control
        rti

        org     VEC_IRQ
        fdb     isr_irq

        org     VEC_RESET
        fdb     initialize
