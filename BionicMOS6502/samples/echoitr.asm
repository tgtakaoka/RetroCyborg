        cpu     6502
        include "w65c816s.inc"

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
tx_queue:
        rmb     queue_buf
        fdb     $2000 + rx_queue_size
tx_queue_size:  equ     128
tx_int_control:
        rmb     1
isr_irq_work:
        rmb     1

RX_INT_TX_NO:   equ     WSB_8N1_gc|RIEB_bm
RX_INT_TX_INT:  equ     WSB_8N1_gc|RIEB_bm|TCB_EI_gc

stack:  equ     $01ff

        org     $1000
initialize:
        ldx     #stack
        txs
        cld                     ; clear decimal flag
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
        lda     #0
        sta     tx_int_control  ; disable Tx interrupt
        cli                     ; enable IRQ

receive_loop:
        jsr     getchar
        bcs     receive_loop
echo_back:
        pha
        jsr     putchar         ; echo
        lda     #' '            ; space
        jsr     putchar
        pla
        pha
        jsr     put_hex8        ; print in hex
        lda     #' '            ; space
        jsr     putchar
        pla
        jsr     put_bin8        ; print in binary
        jsr     newline
        jmp     receive_loop

;;; Put newline
;;; @clobber A
newline:
        lda     #$0d
        jsr     putchar
        lda     #$0a
        jmp     putchar

;;; Print uint8_t in hex
;;; @param A uint8_t value to be printed in hex.
;;; @clobber A
put_hex8:
        pha
        lda     #'0'
        jsr     putchar
        lda     #'x'
        jsr    putchar
        pla
        pha
        lsr     a
        lsr     a
        lsr     a
        lsr     a
        jsr     put_hex4
        pla
put_hex4:
        and     #$0f
        cmp     #10
        bcc     put_hex8_dec    ; <10
        adc     #'A'-10-1       ; C=1
        jmp     putchar
put_hex8_dec:
        adc     #'0'
        jmp     putchar

;;; Print uint8_t in binary
;;; @param A uint8_t value to be printed in binary.
;;; @clobber A
put_bin8:
        pha
        lda     #'0'
        jsr     putchar
        lda     #'b'
        jsr     putchar
        pla
        jsr     put_bin4
        asl     a
        jsr     put_bin4
        rts
put_bin4:
        jsr     put_bin2
        asl     a
put_bin2:
        jsr     put_bin1
        asl     a
put_bin1:
        pha
        ora     #0
        bpl     put_bin0
        lda     #'1'
        bne     put_binchar
put_bin0:
        lda     #'0'
put_binchar:
        jsr     putchar
        pla
        rts

;;; Get character
;;; @return A
;;; @return P.C 1 if no character
;;; @clobber X
getchar:
        php                     ; save P
        ldx     #rx_queue
        sei                     ; disable IRQ
        jsr     queue_remove
        bcc     getchar_exit
        plp                     ; restore P
        sec
        rts
getchar_exit:
        plp
        clc
        rts

;;; Put character
;;; @param A
;;; @clobber X
putchar:
        php                     ; save P
        pha
putchar_retry:
        ldx     #tx_queue
        sei                     ; disable IRQ
        jsr     queue_add
        cli                     ; enable IRQ
        bcs     putchar_retry   ; queue is full
        lda     tx_int_control
        bne     putchar_exit
        lda     #RX_INT_TX_INT  ; enable Tx interrupt
        sta     ACIA_control
        inc     tx_int_control
putchar_exit:
        pla
        plp                     ; restore P
        rts

        include "queue.inc"

isr_irq:
        cld                     ; clear decimal flag
        pha                     ; save A
        txa
        pha                     ; save X
        tya
        pha                     ; save Y
        lda     ACIA_status
        sta     isr_irq_work    ; save ACIA status
        and     #IRQF_bm
        beq     isr_irq_exit
        lda     isr_irq_work
        and     #FERR_bm|OVRN_bm|PERR_bm
        beq     isr_irq_receive
        lda     ACIA_data       ; reset error flags
isr_irq_receive:
        lda     isr_irq_work
        and     #RDRF_bm
        beq     isr_irq_send
        lda     ACIA_data       ; receive character
        ldx     #rx_queue
        jsr     queue_add
isr_irq_send:
        lda     isr_irq_work
        and     #TDRE_bm
        beq     isr_irq_exit
        ldx     #tx_queue
        jsr     queue_remove
        bcs     isr_irq_send_empty
        sta     ACIA_data       ; send character
isr_irq_exit:
        pla                     ; restore Y
        tay
        pla                     ; restore X
        tax
        pla                     ; restore Y
        rti                     ; restore P and PC
isr_irq_send_empty:
        lda     #RX_INT_TX_NO
        sta     ACIA_control    ; disable Tx interrupt
        lda     #0
        sta     tx_int_control
        beq     isr_irq_exit    ; always branch

        org     NVEC_IRQ
        fdb     isr_irq

        org     VEC_IRQ
        fdb     isr_irq

        org     VEC_RESET
        fdb     initialize
