        cpu     1802
        include "cdp1802.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     0DF00H
        include "mc6850.inc"

        org     2000H

rx_queue_size:  equ     16
rx_queue:
        ds      rx_queue_size

RX_INT_TX_NO:   equ     WSB_8N1_gc|RIEB_bm


stack:  equ     1000H-1

        org     0100H
main:
        sep     R4
        dw      queue_init      ; call queue_init
        dw      rx_queue
        db      rx_queue_size
        ;; initialize ACIA
        ldi     ACIA >> 8
        phi     R8
        ldi     ACIA & 0FFH
        plo     R8
        ldi     CDS_RESET_gc    ; Master reset
        str     R8              ; ACIA_control
        ldi     RX_INT_TX_NO
        str     R8              ; ACIA_control
        sex     R3
        ret
        db      33h             ; enable interrupt

loop:
        sex     R3
        dis                     ; disable interrupt
        db      33h             ; X=3, P=3
        sep     R4              ; call queue_remove
        dw      queue_remove
        dw      rx_queue
        sex     R3
        ret                     ; enable interrupt
        db      33h
        bz      loop            ; branch if queue is empty
echo:
        glo     R7
        sep     R4              ; call putchar
        dw      putchar
        glo     R7
        xri     0DH             ; carriage return
        bnz     loop
        ldi     0AH             ; newline
        plo     R7
        br      echo

;;; @param D char
putchar:
        plo     R15             ; save D to R15.0
        sex     R2
        glo     R8              ; save R8
        stxd
        ghi     R8
        stxd
        ;;
        ldi     ACIA >> 8
        phi     R8
        ldi     ACIA & 0FFh
        plo     R8
putchar_loop:
        ldn     R8              ; ACIA_status
        ani     TDRE_bm
        bz      putchar_loop
        inc     R8
        glo     R15             ; restore D
        str     R8              ; ACIA_data
        ;;
        sex     R2
        irx
        ldxa                    ; restore R8
        phi     R8
        ldx
        plo     R8
        sep     R5              ; return

        include "queue.inc"

;;; From scrt_isr, P=3
isr:
        glo     R8              ; save R8
        stxd
        ghi     R8
        stxd
        glo     R7              ; save R7.0
        stxd
        ;;
        ldi     ACIA >> 8
        phi     R8
        ldi     ACIA & 0FFh
        plo     R8              ; R8=ACIA
        ldn     R8              ; ACIA_status
        plo     R7              ; R7.0=status
        ani     IRQF_bm
        lbz     isr_exit        ; no interrupt
isr_receive:
        glo     R7
        ani     RDRF_bm
        lbz     isr_exit        ; no data is received
        inc     R8
        ldn     R8              ; ACIA_data
        dec     R8
        plo     R7
        sep     R4              ; call queue_add
        dw      queue_add
        dw      rx_queue
isr_exit:
        sex     R2
        irx
        ldxa                    ; restore R7.0
        plo     R7
        ldxa                    ; restore R8
        phi     R8
        ldx
        plo     R8
        sep     R1              ; return to scrt_isr

        org     ORG_RESET
        dis                     ; disable interrupt
        db      00h             ; X:P=0:0
        lbr     scrt_init

        include "scrt.inc"
