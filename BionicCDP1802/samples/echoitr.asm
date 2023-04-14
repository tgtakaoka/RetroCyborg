        cpu     1802
        include "cdp1802.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     0DF00H
        include "mc6850.inc"

        org     2000H

rx_queue_size:  equ     16
tx_queue_size:  equ     48
RX_INT_TX_NO:   equ     WSB_8N1_gc|RIEB_bm
RX_INT_TX_INT:  equ     WSB_8N1_gc|RIEB_bm|TCB_EI_gc

rx_queue:
        ds      rx_queue_size
tx_queue:
        ds      tx_queue_size
tx_int_control:
        ds      1

stack:  equ     1000H-1

        org     0100H
main:
        sep     R4
        dw      queue_init      ; call queue_init
        dw      rx_queue
        db      rx_queue_size
        sep     R4
        dw      queue_init      ; call queue_init
        dw      tx_queue
        db      tx_queue_size
        ;; initialize ACIA
        ldi     hi(ACIA)
        phi     R8
        ldi     lo(ACIA)
        plo     R8
        ldi     CDS_RESET_gc    ; Master reset
        str     R8              ; ACIA_control
        ldi     RX_INT_TX_NO
        str     R8              ; ACIA_control
        ;; initialize tx_int_control
        ldi     hi(tx_int_control)
        phi     R8
        ldi     lo(tx_int_control)
        plo     R8
        ldi     0
        str     R8              ; disable Tx interrupt
        sex     R3
        ret
        db      33h             ; enable interrupt

loop:
        sep     R4              ; call getchar
        dw      getchar
        bz      loop
        glo     R7
        sep     R4              ; call putchar
        dw      putchar
        ldi     ' '
        sep     R4              ; call putchar
        dw      putchar
        glo     R7
        sep     R4              ; call put_hex8
        dw      put_hex8
        ldi     ' '
        sep     R4              ; call putchar
        dw      putchar
        glo     R7
        sep     R4              ; call put_bin8
        dw      put_bin8
        ldi     x'0d'
        sep     R4              ; call putchar
        dw      putchar
        ldi     x'0a'
        sep     R4              ; call putchar
        dw      putchar
        lbr     loop

;;; Print uint8_t in hex
;;; @param D uint8_t value to be printed in binary.
put_hex8:
        plo     R15             ; save D to scratch pad
        sex     R2
        glo     R7              ; save R7.0
        stxd
        glo     R15             ; restore D
        ;;
        plo     R7              ; R7.0=data
        ldi     '0'
        sep     R4              ; call putchar
        dw      putchar
        ldi     'x'
        sep     R4              ; call putchar
        dw      putchar
        glo     R7
        shr
        shr
        shr
        shr
        sep     R4              ; call put_hex4
        dw      put_hex4
        glo     R7
        sep     R4              ; call put_hex4
        dw      put_hex4
        ;;
        sex     R2
        irx
        ldx                     ; restore R7.0
        plo     R7
        sep     R5              ; return

;;; Print hexadecimal digit
;;; @param D nibble
;;; @clobber D
put_hex4:
        ani     0Fh
        smi     10
        bnf     put_hex4_dec    ; branch if D < 10
        adi     'A'
        lskp
put_hex4_dec:
        adi     '0'+10
        sep     R4              ; call putchar
        dw      putchar
        sep     R5              ; return

;;; Print uint8_t in binary
;;; @param D uint8_t value to be printed in binary.
;;; @clobber D
put_bin8:
        plo     R15             ; save D to scratch pad
        sex     R2
        glo     R7              ; save R7
        stxd
        ghi     R7
        stxd
        glo     R15             ; restore D
        ;;
        phi     R7              ; R7.1=data
        ldi     8
        plo     R7              ; R7.0=bit count
put_bin8_loop:
        ghi     R7
        shl
        phi     R7              ; R7.1<<=1
        bnf     put_bin8_zero
        ldi     '1'
        lskp
put_bin8_zero:
        ldi     '0'
        sep     R4              ; call putchar
        dw      putchar
        dec     R7              ; R7--
        glo     R7
        bnz     put_bin8_loop
        ;;
        sex     R2
        irx
        ldxa                    ; restore R7
        phi     R7
        ldx
        plo     R7
        sep     R5              ; return

;;; Get character
;;; @return R7.0 char
;;; @return A 0 if no char received
getchar:
        sex     R3
        dis                     ; disable interrupt
        db      33h
        sep     R4              ; call queue_remove
        dw      queue_remove
        dw      rx_queue
        sex     R3
        ret                     ; enable interrupt
        db      33h
        sep     R5              ; return

;;; Put character
;;; @param D char
;;; @clobber D
putchar:
        plo     R15             ; save D to scratch pad
        sex     R2
        glo     R7              ; save R7.0
        stxd
        glo     R15             ; restore D
        ;;
        plo     R7              ; R7.0=char
putchar_loop:
        sex     R3
        dis                     ; disable interrupt
        db      33h
        sep     R4              ; call queue_add
        dw      queue_add
        dw      tx_queue
        sex     R3
        ret                     ; enable interrupt
        db      33h
        bz      putchar_loop    ; retry if queue is full
        sex     R3
        dis                     ; disable interrupt
        db      33h
        ldi     hi(tx_int_control)
        phi     R15
        ldi     lo(tx_int_control)
        plo     R15
        ldn     R15
        bnz     putchar_exit    ; branch if Tx interrupt enabled
        ldi     1
        str     R15             ; mark enable Tx interrupt
        ldi     hi(ACIA)
        phi     R15
        ldi     lo(ACIA)
        plo     R15
        ldi     RX_INT_TX_INT   ; enable Tx interrupt
        str     R15             ; ACIA_C
        sex     R3
        ret                     ; enable interrupt
        db      33h
putchar_exit:
        sex     R2
        irx
        ldx                     ; restore R7.0
        plo     R7
        sep     R5              ; return

        include "queue.inc"

;;; From scrt_isr, X=2, P=3
isr:
        glo     R8              ; save R8
        stxd
        ghi     R8
        stxd
        glo     R7              ; save R7
        stxd
        ghi     R7
        stxd
        ;;
        ldi     hi(ACIA)
        phi     R8
        ldi     lo(ACIA)
        plo     R8              ; R8=ACIA
        ldn     R8              ; ACIA_status
        phi     R7              ; R7.1=status
        ani     FERR_bm|OVRN_bm|PERR_bm
        bz      isr_receive
        inc     R8
        ldn     R8              ; clear error
        dec     R8
isr_receive:
        ghi     R7
        ani     RDRF_bm
        bz      isr_send        ; no data is received
        inc     R8
        ldn     R8              ; ACIA_data
        dec     R8
        plo     R7
        sep     R4              ; call queue_add
        dw      queue_add
        dw      rx_queue
isr_send:
        ghi     R7
        ani     TDRE_bm
        bz      isr_exit
        sep     R4              ; call queue_remove
        dw      queue_remove
        dw      tx_queue
        bz      isr_send_empty
        glo     R7
        inc     R8
        str     R8              ; ACIA_D
        dec     R8
        br      isr_exit
isr_send_empty:
        ldi     RX_INT_TX_NO    ; disable Tx interrupt
        str     R8              ; ACIA_C
        ldi     hi(tx_int_control)
        phi     R8
        ldi     lo(tx_int_control)
        plo     R8
        ldi     0
        str     R8              ; mark Tx interrupt disabled
isr_exit:
        sex     R2
        irx
        ldxa                    ; restore R7
        phi     R7
        ldxa
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
