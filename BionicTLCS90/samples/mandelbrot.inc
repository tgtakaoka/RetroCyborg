        org     0200H
vF:     ds      2
vC:     ds      2
vD:     ds      2
vA:     ds      2
vB:     ds      2
vP:     ds      2
vQ:     ds      2
vS:     ds      2
vT:     ds      2
tmp:    ds      2
vY:     ds      2
vX:     ds      2
vI:     ds      2

        org     1000H
mandelbrot:
        ldw     (vF), 50        ; F=50
        ldw     (vY), -12       ; Y=-12
loop_y:
        ldw     (vX), -49       ; X=-49
loop_x:
        ld      HL, (vX)
        ld      (vC), HL        ; C=X
        ldw     (tmp), 229
        ld      IX, vC
        ld      IY, tmp
        call    mulsi2          ; C=X*229
        ldw     (tmp), 100
        call    divsi2          ; C=X*229/100
        ld      HL, (vY)
        ld      (vD), HL        ; D=Y
        ldw     (tmp), 416
        ld      IX, vD
        call    mulsi2          ; D=Y*416
        ldw     (tmp), 100
        call    divsi2          ; D=Y*416/100
        ld      HL, (vC)
        ld      (vA), HL        ; A=C
        ld      HL, (vD)
        ld      (vB), HL        ; B=D
        ldw     (vI), 0         ; I=0
loop_i:
        ld      HL, (vB)
        ld      (vQ), HL        ; Q=B
        ld      IX, vQ
        ld      IY, vF
        call    divsi2          ; Q=B/F
        ld      IX, vS
        ld      IY, vQ
        call    negsi2          ; S=-Q
        ld      IY, vF
        call    mulsi2          ; S=-Q*F
        ld      IY, vB
        call    addsi2          ; S=B-Q*F
        ld      HL, (vB)
        ld      (tmp), HL
        ld      IX, tmp
        call    mulsi2          ; tmp=B*B
        ld      HL, (vA)
        ld      (vT), HL
        ld      IX, vT
        ld      IY, vA
        call    mulsi2          ; T=A*A
        ld      IY, tmp
        call    subsi2          ; T=A*A-B*B
        ld      IY, vF
        call    divsi2          ; T=(A*A-B*B)/F
        ld      IY, vC
        call    addsi2          ; T=(A*A-B*B)/F+C
        ld      HL, (vA)
        ld      (tmp), HL
        ld      IX, tmp
        ld      IY, vS
        call    mulsi2          ; tmp=A*S
        ld      IY, vF
        call    divsi2          ; tmp=A*S/F
        ld      HL, (vA)
        ld      (vB), HL        ; B=A
        ld      IX, vB
        ld      IY, vQ
        call    mulsi2          ; B=A*Q
        ld      IY, tmp
        call    addsi2          ; B=A*Q+A*S/F
        ld      IY, vB
        call    addsi2          ; B=2*(A*Q+A*S/F)
        ld      IY, vD
        call    addsi2          ; B=2*(A*Q+A*S/F)+D
        ld      HL, (vT)
        ld      (vA), HL        ; A=T
        ld      (vP), HL        ; P=A
        ld      IX, vP
        ld      IY, vF
        call    divsi2          ; P=A/F
        ld      HL, (vB)
        ld      (vQ), HL
        ld      IX, vQ
        call    divsi2          ; Q=B/F
        ld      HL, (vQ)
        ld      (tmp), HL
        ld      IX, tmp
        ld      IY, vQ
        call    mulsi2          ; tmp=Q*Q
        ld      HL, (vP)
        ld      (vT), HL
        ld      IX, vT
        ld      IY, vP
        call    mulsi2          ; T=P*P
        ld      IY, tmp
        call    addsi2          ; T=P*P+Q*Q
        ldw     (tmp), 4
        ld      IX, tmp
        ld      IY, vT
        call    cmpsi2          ; 4-T
        jp      LT, print_i     ; if 4<T
        incw    (vI)            ; I+=1
        ldw     (tmp), 16
        ld      IX, vI
        ld      IY, tmp
        call    cmpsi2
        jp      LT, loop_i      ; if I<16
        ld      A, ' '
        jp      print_char
print_i:
        ld      A, (vI)
        cp      A, 10
        jr      ULT, print_i2   ; if I<10
        add     A, 'A'-'0'-10
print_i2:
        add     A, '0'
print_char:     
        call    putchar
        call    getchar
        jr      nc, next_x
        or      a, a
        jr      nz, next_x
        swi                     ; break
next_x:
        incw    (vX)            ; X+=1
        ldw     (tmp), 30
        ld      IX, vX
        ld      IY, tmp
        call    cmpsi2
        jp      LT, loop_x      ; if X<30
        ld      A, 0DH
        call    putchar
        ld      A, 0AH
        call    putchar
        incw    (vY)              ; Y+=1
        ldw     (tmp), 13
        ld      IX, vY
        ld      IY, tmp
        call    cmpsi2
        jp      LT, loop_y      ; if X<13
        ret

;;; Local Variables:
;;; mode: asm
;;; End:
;;; vim: set ft=asm et ts=4 sw=4:
