        cpu     tlcs90

;;; Negation; result = -value
;;; @param @IX: result
;;; @param @IY: value
;;; @clobber HL
negsi2:
        ld      HL, (IY)
        xor     HL, 0FFFFH
        inc     HL
        ld      (IX), HL
        ret

;;; Signed addition: summand += addend
;;; @param @IX: summand
;;; @param @IY: addend
;;; @clobber HL
addsi2:
        ld      HL, (IX)
        add     HL, (IY)
        ld      (IX), HL
        ret

;;; Singed subtraction: minuend -= subtrahend
;;; @param @IX: minuend
;;; @param @IY: subtrahend
;;; @clobber HL
subsi2:
        ld      HL, (IX)
        sub     HL, (IY)
        ld      (IX), HL
        ret

;;; Signed comparison: minuend - subtrahend
;;; @param @IX: minuend
;;; @param @IY: subtrahend
;;; @return F.S, F.V
;;; @clobber HL
cmpsi2:
        ld      HL, (IX)
        cp      HL, (IY)
        ret

;;; Unsigned multiplication: result = multiplicand * multiplier
;;; @param BC: multiplicand
;;; @param DE: multiplier
;;; @return HL: result
;;; @clobber BC DE HL A
umul16:
        ld      HL, 0           ; result=0
        jr      umul16_check
umul16_loop:
        srl     D
        rr      E               ; multiplier >>= 1
        jr      nc, umul16_next ; if lsb(multiplier) == 0
        add     HL, BC          ; result += multiplicand
umul16_next:
        sll     C
        rl      B               ; multiplicand <<= 1
umul16_check:
        ld      A, D
        or      A, E
        jr      nz, umul16_loop ; while multiplier != 0
umul16_end:
        ret

;;; Unsigned multiplication: multiplicand *= multiplier
;;; @param @IX: multiplicand
;;; @param @IY: multiplier
;;; @clobber HL A
umulsi2:
        push    BC
        push    DE
        ld      BC, (IX)        ; BC=multiplicand
        ld      DE, (IY)        ; DE=multiplier
        call    umul16          ; HL=multiplicand * multiplier
        ld      (IX), HL
        pop     DE
        pop     BC
        ret

;;; Signed multiplication: multiplicand *= multiplier
;;; @param @IX: multiplicand
;;; @param @IY: multiplier
;;; @clobber HL A
mulsi2:
        push    BC
        push    DE
        ld      HL, (IX)        ; HL=multiplicand
        ld      A, H            ; A=high(multiplicand)
        bit     7, H
        jr      Z, mulsi2_abs_muliplicand
        xor     HL, 0FFFFH
        inc     HL              ; multiplicand = -multiplicand
mulsi2_abs_muliplicand:
        ld      BC, HL          ; BC=abs(multiplicand)
        ld      HL, (IY)        ; HL=multiplier
        xor     A, H            ; A=high(multiplicand^multiplier)
        bit     7, H
        jr      Z, mulsi2_multiply
        xor     HL, 0FFFFH
        inc     HL              ; multiplier = -multiplier
mulsi2_multiply:
        ld      DE, HL          ; DE=abs(multiplyer)
        push    AF              ; save sign
        call    umul16          ; HL = multiplicand * multiplier
        pop     AF              ; A=sign
        bit     7, A
        jr      Z, mulsi2_return
        xor     HL, 0FFFFH
        inc     HL              ; result = -result
mulsi2_return:
        ld      (IX), HL
        pop     DE
        pop     BC
        ret

;;; Unsigned division: divident / divisor = quotient ... reminder
;;; @praram BC: divident
;;; @praram DE: divisor
;;; @return HL: reminder
;;; @return BC: quotient
;;; @clobber BC DE HL A
udiv16:
        ld      A, D
        or      A, E
        ret     Z               ; divide by zero
        ld      A, 1            ; A=bits
        jr      udiv16_prep
udiv16_prep_loop:
        sll     E
        rl      D               ; divisor <<= 1
        inc     A               ; ++bits
udiv16_prep:                    ; while msb(divisor) == 0
        bit     7, D
        jr      Z, udiv16_prep_loop
        ld      HL, BC          ; HL=divident
        ld      BC, 0           ; BC=quotient
        jr      udiv16_enter_loop
udiv16_loop:
        srl     D
        rr      E               ; divisor >>= 1
        sll     C
        rl      B               ; quotient <<= 1
udiv16_enter_loop:
        sub     HL, DE          ; divident -= divisor
        jr      ULT, udiv16_readd ; if divident < 0
        inc     C                 ; quotient |= 1
        dec     A
        jr      NZ, udiv16_loop ; while bits != 0
        ret
udiv16_readd:
        add     HL, DE          ; divident += divisor
        dec     A               ; --bits
        jr      NZ, udiv16_loop ; while bits != 0
        ret

;;; Unsigned division: dividend /= divisor
;;; @praram @IX: dividend
;;; @praram @IY: divisor
;;; @clobber HL A
udivsi2:
        push    BC
        push    DE
        ld      BC, (IX)
        ld      DE, (IY)
        call    udiv16
        ld      (IX), BC
        pop     DE
        pop     BC
        ret

;;; Signed division: divident *= divisor
;;; @param @IX: divident
;;; @param @IY: divisor
;;; @clobber HL A
divsi2:
        push    BC
        push    DE
        ld      HL, (IX)        ; HL=divident
        ld      A, H            ; A=high(divident)
        bit     7, H
        jr      Z, divsi2_abs_divident
        xor     HL, 0FFFFH
        inc     HL              ; divident = -divident
divsi2_abs_divident:
        ld      BC, HL          ; BC=abs(divident)
        ld      HL, (IY)        ; HL=divisor
        xor     A, H            ; A=high(divident^divisor)
        bit     7, H
        jr      Z, divsi2_divide
        xor     HL, 0FFFFH
        inc     HL              ; divisor = -divisor
divsi2_divide:
        ld      DE, HL          ; DE=abs(dvisor)
        push    AF              ; save sign
        call    udiv16          ; BC = divident / divisor
        pop     AF              ; A=sign
        ld      HL, BC          ; HL=quotient
        bit     7, A
        jr      Z, divsi2_return
        xor     HL, 0FFFFH
        inc     HL              ; quotient=-quotient
divsi2_return:
        ld      (IX), HL
        pop     DE
        pop     BC
        ret

;;; Local Variables:
;;; mode: asm
;;; End:
;;; vim: set ft=asm et ts=4 sw=4:
