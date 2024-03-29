        cpu     8085

;;; Negation; result = -value
;;; @param @BC: result
;;; @param @DE: value
;;; @clobber A
negsi2:
        ldax    D
        cma
        adi     1
        stax    B
        inx     D
        ldax    D
        cma
        aci     0
        inx     B
        stax    B
        dcx     D
        dcx     B
        ret

;;; Signed addition: summand += addend
;;; @param @BC: summand
;;; @param @DE: addend
;;; @clobber A
addsi2:
        push    H
        ldax    D
        mov     L, A
        inx     D
        ldax    D
        mov     H, A
        dcx     D
        ldax    B
        add     L
        stax    B
        inx     B
        ldax    B
        adc     H
        stax    B
        dcx     B
        pop     H
        ret

;;; Singed subtraction: minuend -= subtrahend
;;; @param @BC: minuend
;;; @param @DE: subtrahend
;;; @clobber A
subsi2:
        push    H
        ldax    D
        mov     L, A
        inx     D
        ldax    D
        dcx     D
        mov     H, A
        ldax    B
        sub     L
        stax    B
        inx     B
        ldax    B
        sbb     H
        stax    B
        dcx     B
        pop     H
        ret

;;; Signed comparison: minuend - subtrahend
;;; @param @BC: minuend
;;; @param @DE: subtrahend
;;; @return PSW.Z, PSW.S
;;; @clobber A
cmpsi2:
        push    B
        push    D
        push    H
        ldax    D
        mov     L, A
        inx     D
        ldax    D
        mov     D, A
        mov     E, L            ; DE=@DE
        ldax    B
        mov     L, A
        inx     B
        ldax    B
        mov     B, A
        mov     C, L            ; BC=@BC
        mov     A, C
        sub     E
        mov     L, A
        mov     A, B
        sbb     D
        mov     H, A            ; HL=@BC-@DE
        ora     L
        jz      cmpsi2_return   ; PSW.Z=1
        mov     A, H            ; high(@BC-@DE)
        xra     B               ; high((@BC-@DE)^@BC)
        mov     L, A
        mov     A, B
        xra     D               ; high(@BC^@DE)
        ana     L               ; overflow
        xra     H               ; PSW.S=overflow^sign
        ori     1               ; clear PSW.Z
cmpsi2_return:
        pop     H
        pop     D
        pop     B
        ret

;;; Unsigned multiplication: result = multiplicand * multiplier
;;; @param BC: multiplicand
;;; @param DE: multiplier
;;; @return HL: result
;;; @clobber BC DE HL A
umul16:
        lxi     H, 0            ; result=0
        jmp     umul16_check
umul16_loop:
        ora     A               ; cleat PSW.C
        mov     A, D            ; multiplier >>= 1
        rar
        mov     D, A
        mov     A, E
        rar
        mov     E, A
        jnc     umul16_next     ; if lsb(multiplier) == 0
        dad     B               ; result += multiplicand
umul16_next:
        ora     A               ; clear PSW.C
        mov     A, C            ; multiplicand <<= 1
        ral
        mov     C, A
        mov     A, B
        ral
        mov     B, A
umul16_check:
        mov     A, D
        ora     E
        jnz     umul16_loop     ; while multiplier != 0
umul16_end:
        ret

;;; Unsigned multiplication: multiplicand *= multiplier
;;; @param @BC: multiplicand
;;; @param @DE: multiplier
;;; @clobber HL A
umulsi2:
        push    B
        push    D
        ldax    B
        mov     L, A
        inx     B
        ldax    B
        mov     B, A
        mov     C, L            ; BC=multiplicand
        ldax    D
        mov     L, A
        inx     D
        ldax    D
        mov     D, A
        mov     E, L            ; DE=multiplier
        call    umul16          ; HL=multiplicand * multiplier
        pop     D
        pop     B
        mov     A, L
        stax    B
        inx     B
        mov     A, H
        stax    B
        dcx     B               ; @BC=HL
        ret

;;; Signed multiplication: multiplicand *= multiplier
;;; @param @BC: multiplicand
;;; @param @DE: multiplier
;;; @clobber HL A
mulsi2:
        push    B
        push    D
        ldax    B
        mov     L, A
        inx     B
        ldax    B
        mov     B, A
        mov     C, L            ; BC=multiplicand
        mov     H, B            ; H=high(multiplicand)
        ora     A
        jp      mulsi2_abs_muliplicand
        cma
        mov     B, A
        mov     A, C
        cma
        mov     C, A
        inx     B               ; multiplicand = -multiplicand
mulsi2_abs_muliplicand:
        ldax    D
        mov     L, A
        inx     D
        ldax    D
        mov     D, A
        mov     E, L            ; DE=multiplier
        xra     H               ; H=high(multiplicand^multiplier)
        push    PSW             ; save sign
        mov     A, D
        ora     A
        jp      mulsi2_multiply
        cma
        mov     D, A
        mov     A, E
        cma
        mov     E, A
        inx     D               ; multiplier = -multiplier
mulsi2_multiply:
        call    umul16          ; HL = multiplicand * multiplier
        pop     B               ; B=sign
        mov     A, B
        ora     A
        jp      mulsi2_return
        mov     A, H
        cma
        mov     H, A
        mov     A, L
        cma
        mov     L, A
        inx     H               ; result = -result
mulsi2_return:
        pop     D
        pop     B
        mov     A, L
        stax    B
        inx     B
        mov     A, H
        stax    B
        dcx     B               ; @BC=result
        ret

;;; Unsigned division: divident / divisor = quotient ... reminder
;;; @praram BC: divident
;;; @praram DE: divisor
;;; @return HL: reminder
;;; @return BC: quotient
;;; @clobber BC DE HL A
udiv16:
        mov     A, E
        ora     D
        rz                      ; divide by zero
        mvi     L, 1            ; L=bits
        jmp     udiv16_prep
udiv16_prep_loop:
        ora     A               ; clear PSW.C
        mov     A, E            ; divisor <<= 1
        ral
        mov     E, A
        mov     A, D
        ral
        mov     D, A
        inx     H               ; ++bits
udiv16_prep:                    ; while msb(divisor) == 0
        mov     A, D
        ora     A
        jp      udiv16_prep_loop
        mov     A, L
        push    PSW             ; push bits
        mov     H, B
        mov     L, C            ; HL=divident
        xra     A
        mov     B, A
        mov     C, A            ; BC=quotient
        jmp     udiv16_enter_loop
udiv16_loop:
        push    PSW             ; push bits
        ora     A               ; clear PSW.C
        mov     A, D            ; divisor >>= 1
        rar
        mov     D, A
        mov     A, E
        rar
        mov     E, A
        ora     A               ; clear PSW.C
        mov     A, C            ; quotient <<= 1
        ral
        mov     C, A
        mov     A, B
        ral
        mov     B, A
udiv16_enter_loop:
        mov     A, L
        sub     E
        mov     L, A
        mov     A, H
        sbb     D
        mov     H, A            ; divident -= divisor
        jc      udiv16_readd    ; if divident < 0
        inx     B               ; quotient |= 1
        jmp     udiv16_next    ;
udiv16_readd:
        dad     D               ; divident += divisor
udiv16_next:
        pop     PSW
        dcr     A               ; --bits
        jnz     udiv16_loop     ; while bits != 0
        ret

;;; Unsigned division: dividend /= divisor
;;; @praram @BC: dividend
;;; @praram @DE: divisor
;;; @clobber HL A
udivsi2:
        push    B
        push    D
        ldax    B
        mov     L, A
        inx     B
        ldax    B
        mov     B, A
        mov     C, L            ; BC=divident
        ldax    D
        mov     L, A
        inx     D
        ldax    D
        mov     D, A
        mov     E, L            ; DE=divisor
        call    umul16          ; HL=reminder BC=quotient
        mov     H, B
        mov     L, C            ; HL=quotient
        pop     D
        pop     B
        mov     A, L
        stax    B
        inx     B
        mov     A, H
        stax    B
        dcx     B               ; @BC=HL
        ret

;;; Signed division: divident *= divisor
;;; @param @BC: divident
;;; @param @DE: divisor
;;; @clobber HL A
divsi2:
        push    B
        push    D
        ldax    B
        mov     L, A
        inx     B
        ldax    B
        mov     B, A
        mov     C, L            ; BC=divident
        mov     H, B            ; H=high(divident)
        ora     A
        jp      divsi2_abs_divident
        cma
        mov     B, A
        mov     A, C
        cma
        mov     C, A
        inx     B               ; divident = -divident
divsi2_abs_divident:
        ldax    D
        mov     L, A
        inx     D
        ldax    D
        mov     D, A
        mov     E, L            ; DE=divisor
        xra     H               ; H=high(divident^divisor)
        push    PSW             ; save sign
        mov     A, D
        ora     A
        jp      divsi2_divide
        cma
        mov     D, A
        mov     A, E
        cma
        mov     E, A
        inx     D               ; divisor = -divisor
divsi2_divide:
        call    udiv16          ; BC = divident / divisor
        mov     H, B
        mov     L, C            ; HL=quotient
        pop     B               ; B=sign
        mov     A, B
        ora     A
        jp      divsi2_return
        mov     A, H
        cma
        mov     H, A
        mov     A, L
        cma
        mov     L, A
        inx     H               ; quotient=-quotient
divsi2_return:
        pop     D
        pop     B
        mov     A, L
        stax    B
        inx     B
        mov     A, H
        stax    B
        dcx     B               ; @BC=result
        ret

;;; Local Variables:
;;; mode: asm
;;; End:
;;; vim: set ft=asm et ts=4 sw=4:
