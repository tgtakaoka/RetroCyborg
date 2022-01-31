;;; -*- mode: asm; mode: flyspell-prog; -*-

;;; Standard Call and Return Technique
;;; R0: DMA pointer
;;; R1: Program counter for Interrupt routine
;;; R2: Stack pointer
;;; R3: Program counter
;;; R4: Dedicated program counter for CALL routine
;;; R5: Dedicated program counter for RETURN routine
;;; R6: Link register, pointer to the return location and arguments
;;;     passed by the calling program

;;; Call subroutine
;;;   SEP R4
;;;   DW  subroutine
;;;   DB  arguments...
;;; Subroutine return
;;;   SEP R5
;;; Return from interrupt
;;;   SEP R1

;;; Initialize for SCRT, P=0
;;; @param P!=3
;;; @param stack top address of stack
;;; @param main start address of main routine
;;; @return P=3
;;; @return R1=scrt_isr
;;; @return R2=stack
;;; @return R3=main
;;; @return R4=scrt_call
;;; @return R5=scrt_return
;;; @clobber D, R15
scrt_init:
        ldi     scrt_start >> 8
        phi     R3
        ldi     scrt_start & 0ffh
        plo     R3
        sep     R3              ; P=3
scrt_start:     
        ldi     scrt_init_tab >> 8
        phi     R15
        ldi     scrt_init_tab & 0ffh
        plo     R15
        lda     R15             ; setup interrupt
        phi     R1
        lda     R15
        plo     R1
        lda     R15             ; setup stack
        phi     R2
        lda     R15
        plo     R2
        lda     R15             ; setup call
        phi     R4
        lda     R15
        plo     R4
        lda     R15             ; setup return
        phi     R5
        lda     R15
        plo     R5
        lbr     main            ; goto main with P=3

scrt_init_tab:
        dw      scrt_isr        ; R1
        dw      stack           ; R2
        dw      scrt_call       ; R4
        dw      scrt_return     ; R5
        
;;; Call subroutine, P=4
;;; @param M(R3):M(R3+1) subroutine address
;;; @param M(R3+2) optional arguments, depending on a subroutine called.
;;; @return R6 points optional arguments
;;; @return M(R2) R6.1, R6.0
;;; @clobber R15.1, X
;;; @unchanged D, IE, R0, R1, R7-R14, R15.0
scrt_call_exit: 
        sep     R3              ; go to subroutine
scrt_call:
        phi     R15             ; save D to R15.1
        sex     R2              ; select stack
        glo     R6              ; push old link register R6
        stxd
        ghi     R6
        stxd
        ghi     R3              ; load link register
        phi     R6
        glo     R3
        plo     R6
        lda     R6              ; load the address of subroutine
        phi     R3
        lda     R6
        plo     R3              ; R3=subroutine address
        ghi     R15             ; restore D
        br      scrt_call_exit

;;; Return subroutine, P=5
;;; @param R6 return address
;;; @oaram M(R2) R6.1, R6.0, X:P(3)
;;; @clobber R15.1, X
;;; @unchanged D, R0, R1, R7-R14, R15.0
scrt_return_exit:
        sep     R3              ; return to subroutine caller
scrt_return:
        phi     R15             ; save D to R15.1
        ghi     R6              ; load return address from link register
        phi     R3
        glo     R6
        plo     R3
        sex     R2              ; select stack
        irx
        ldxa                    ; pop link register R6
        phi     R6
        ldx
        plo     R6
        ghi     R15             ; restore D
        br      scrt_return_exit

;;; Interrupt entry P=1
;;; @unchanged D, DF, X, P, R3, R6, R15
scrt_isr_exit:
        sex     R2              ; select stack
        irx
        ldxa                    ; pop program counter R3
        phi     R3
        ldxa
        plo     R3
        ldxa                    ; pop call pointer R4
        phi     R4
        ldxa
        plo     R4
        ldxa                    ; pop return pointer R5
        phi     R5
        ldxa
        plo     R5
        ldxa                    ; pop scratch pad register R15
        phi     R15
        ldxa
        plo     R15
        ldxa                    ; pop DF into BSB
        shl                     ; restore DF
        ldxa                    ; pop D
        ret                     ; restore X,P IE=1
        ;; P1 points scrt_isr
;;; CDP1802 interrupt entry, X=2, P=1, IE=0
scrt_isr:
        ;; R2[0] must be preserved because it may be in the pop process
        dec     R2
        sav                     ; push X,P
        dec     R2
        stxd                    ; push D
        shrc                    ; MSB of D=DF
        stxd                    ; push DF
        glo     R15             ; push scratch pad register R15
        stxd
        ghi     R15
        stxd
        glo     R5              ; push return pointer R5
        stxd
        ghi     R5
        stxd
        glo     R4              ; push call pointer R4
        stxd
        ghi     R4
        stxd
        glo     R3              ; push program counter R3
        stxd
        ghi     R3
        stxd
        ;;
        ldi     scrt_call >> 8  ; initialize call pointer R4
        phi     R4
        ldi     scrt_call & 0FFh
        plo     R4
        ldi     scrt_return >> 8 ; initialize return pointer R5
        phi     R5
        ldi     scrt_return & 0FFh
        plo     R5
        ;;
        ldi     isr >> 8
        phi     R3
        ldi     isr & 0FFh
        plo     R3
        sep     R3              ; call interrupt service routine with P=3
        br      scrt_isr_exit   ; return from isr by SEP P1
