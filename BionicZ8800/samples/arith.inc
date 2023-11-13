        cpu     z88
        option  optimize-index, on

;;; Negation; result = -value
;;; @param @r4: result
;;; @param @r5: value
;;; @clobber r14, r15
negsi2:
        ldw     rr14, @r5
        com     r14
        com     r15
        incw    rr14
        ld      0(r4), r14
        ld      1(r4), r15
        ret

;;; Signed addition: summand += addend
;;; @param @r4: summand
;;; @param @r5: addend
;;; @clobber r14, r15
addsi2:
        ldw     rr14, @r4
        inc     r5
        add     r15, @r5
        dec     r5
        adc     r14, @r5
        ld      0(r4), r14
        ld      1(r4), r15
        ret

;;; Singed subtraction: minuend -= subtrahend
;;; @param @r4: minuend
;;; @param @r5: subtrahend
;;; @clobber r14, r15
subsi2:
        ldw     rr14, @r4
        inc     r5
        sub     r15, @r5
        dec     r5
        sbc     r14, @r5
        ld      0(r4), r14
        ld      1(r4), r15
        ret

;;; Signed comparison: minuend - subtrahend
;;; @param @r4: minuend
;;; @param @r5: subtrahend
;;; @clobber r14, r15
cmpsi2:
        ldw     rr14, @r4
        inc     r5
        sub     r15, @r5
        dec     r5
        sbc     r14, @r5        ; rr14=@r4-@r5
        call    addsub_flags    ; set C,Z,S,V
        ret

;;; Set add/sub flags
;;; @param @r4: operand 1
;;; @param @r5: operand 2
;;; @param rr14: result
;;; @param FLAGS
;;; @return FLAGS: C, Z, S, V
addsub_flags:
        push    r15
        push    r14
        push    r13
        ld      r13, FLAGS
        or      r13, #F_ZERO LOR F_OVERFLOW ; set Z, V
        or      r15, r14
        jr      z, addsub_flags_z
        xor     r13, #F_ZERO    ; clear Z
addsub_flags_z:
        xor     r14, @r4
        xor     r14, @r5        ; r14:7 carry into S
        jr      nc, addsub_flags_nc
        xor     r14, #%80       ; r14:7=C^(carry into S)
addsub_flags_nc:
        jr      mi, addsub_flags_v
        xor     r13, #F_OVERFLOW ; clear V
addsub_flags_v:
        ld      FLAGS, r13
        pop     r13
        pop     r14
        pop     r15
        ret

;;; Unsigned multiplication: result = multiplicand * multiplier
;;; @param rr14: multiplicand
;;; @param rr12: multiplier
;;; @return rr14: result
;;; @clobber r10-r15
umul16:
        ldw     rr10, rr14      ; rr10=multiplicand
        ldw     rr14, #0        ; result=0
        jr      umul16_check
umul16_loop:
        rcf                     ; multiplier >>= 1
        rrc     r12
        rrc     r13
        jr      nc, umul16_next ; if lsb(multiplier) == 0
        add     r15, r11
        adc     r14, r10        ; result += multiplicand
umul16_next:
        rcf                     ; multiplicand <<= 1
        rlc     r11
        rlc     r10
umul16_check:
        incw    rr12
        decw    rr12
        jr      nz, umul16_loop ; while multiplier != 0
umul16_end:
        ret

;;; Unsigned multiplication: multiplicand *= multiplier
;;; @param @r4: multiplicand
;;; @param @r5: multiplier
;;; @clobber r10-r15
umulsi2:
        ldw     rr14, @r4
        ldw     rr12, @r5
        call    umul16          ; rr10=multiplicand * multiplier
        ld      0(r4), r14
        ld      1(r4), r15
        ret

;;; Signed multiplication: multiplicand *= multiplier
;;; @param @r4: multiplicand
;;; @param @r5: multiplier
;;; @clobber r10-r15
mulsi2:
        ldw     rr14, @r4
        ldw     rr12, @r5
        btjrf   mulsi2_abs_muliplicand, r12, #7
        com     r12
        com     r13
        incw    rr12            ; multiplicand = -multiplicand
mulsi2_abs_muliplicand:
        btjrf   mulsi2_multiply, r14, #7
        com     r14
        com     r15
        incw    rr14            ; multiplier = -multiplier
mulsi2_multiply:
        call    umul16          ; result = multiplicand * multiplier
        ld      r12, @r4
        xor     r12, @r5        ; r12=sign(@r4)^sign(@r5)
        btjrf   mulsi2_end, r12, #7
        com     r14
        com     r15
        incw    rr14            ; result = -result
mulsi2_end:
        ld      0(r4), r14
        ld      1(r4), r15
        ret

;;; Unsigned division: divident / divisor = quotient ... reminder
;;; @praram rr14: divident
;;; @praram rr12: divisor
;;; @return rr14: reminder
;;; @return rr12: quotient
;;; @clobber r10-r15
udiv16:
        push    r9
        incw    rr12
        decw    rr12
        jp      z, udiv16_end
        ld      r9, #1          ; r9=bits
        jp      udiv16_prep
udiv16_prep_loop:
        rcf                     ; divisor <<= 1
        rlc     r13
        rlc     r12
        inc     r9
udiv16_prep:                    ; while msb(divisor) == 0
        btjrf   udiv16_prep_loop, r12, #7
        ldw     rr10, #0        ; rr10=quotient
        jp      udiv16_enter_loop
udiv16_loop:
        rcf                     ; divisor >>= 1
        rrc     r12
        rrc     r13
        rcf                     ; quotient <<= 1
        rlc     r11
        rlc     r10
udiv16_enter_loop:
        sub     r15, r13        ; divident -= divisor
        sbc     r14, r12
        jp      c, udiv16_readd ; if divident < 0
        inc     r11             ; quotient |= 1
        jp      udiv16_next
udiv16_readd:
        add     r15, r13        ; divident += divisor
        adc     r14, r12        ; FLAG.D is always cleared
udiv16_next:
        djnz    r9, udiv16_loop ; rr14=reminder
        ldw     rr12, rr10      ; rr12=quotient
udiv16_end:
        pop     r9
        ret

;;; Unsigned division: dividend /= divisor
;;; @praram @r4: dividend
;;; @praram @r5: divisor
;;; @clobber r10-r15
udivsi2:
        ldw     rr14, @r4
        ldw     rr12, @r5
        call    udiv16
        ld      0(r4), r12
        ld      1(r4), r13
        ret

;;; Signed division: divident *= divisor
;;; @param @r4: divident
;;; @param @r5: divisor
;;; @clobber r10-r15
divsi2:
        ldw     rr14, @r4
        ldw     rr12, @r5
        btjrf   divsi2_abs_divident, r14, #7
        com     r14
        com     r15
        incw    rr14            ; divident = -divident
divsi2_abs_divident:
        btjrf   divsi2_divide, r12, #7
        com     r12
        com     r13
        incw    rr12            ; divisor = -divisor
divsi2_divide:
        call    udiv16
        ld      r14, @r4
        xor     r14, @r5        ; r14=sign(@r4)^sign(@r5)
        btjrf   divsi2_end, r14, #7
        com     r12
        com     r13
        incw    rr12            ; divident = = -divident
divsi2_end:
        ld      0(r4), r12
        ld      1(r4), r13
        ret

;;; Local Variables:
;;; mode: asm
;;; End:
;;; vim: set ft=asm et ts=4 sw=4:
