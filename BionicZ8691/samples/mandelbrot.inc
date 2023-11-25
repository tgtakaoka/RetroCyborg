        org     %40
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

        org     %1000
mandelbrot:
        ld      vF, #HIGH 50    ; F=50
        ld      vF+1, #LOW 50
        ld      vY, #HIGH -12   ; Y=-12
        ld      vY+1, #LOW -12
loop_y:
        ld      vX, #HIGH -49   ; X=-49
        ld      vX+1, #LOW -49
loop_x:
        ld      vC, vX          ; C=X
        ld      vC+1, vX+1      ; C=X
        ld      tmp, #HIGH 229
        ld      tmp+1, #LOW 229
        ld      r4, #vC
        ld      r5, #tmp
        call    mulsi2          ; C=X*229
        ld      tmp, #HIGH 100
        ld      tmp+1, #LOW 100
        call    divsi2          ; C=X*229/100
        ld      vD, vY          ; D=Y
        ld      vD+1, vY+1
        ld      tmp, #HIGH 416
        ld      tmp+1, #LOW 416
        ld      r4, #vD
        call    mulsi2          ; D=Y*416
        ld      tmp, #HIGH 100
        ld      tmp+1, #LOW 100
        call    divsi2          ; D=Y*416/100
        ld      vA, vC          ; A=C
        ld      vA+1, vC+1
        ld      vB, vD          ; B=D
        ld      vB+1, vD+1
        ld      vI, #HIGH 0     ; I=0
        ld      vI+1, #LOW 0
loop_i:
        ld      vQ, vB          ; Q=B
        ld      vQ+1, vB+1
        ld      r4, #vQ
        ld      r5, #vF
        call    divsi2          ; Q=B/F
        ld      r4, #vS
        ld      r5, #vQ
        call    negsi2          ; S=-Q
        ld      r5, #vF
        call    mulsi2          ; S=-Q*F
        ld      r5, #vB
        call    addsi2          ; S=B-Q*F
        ld      tmp, vB
        ld      tmp+1, vB+1
        ld      r4, #tmp
        call    mulsi2          ; tmp=B*B
        ld      vT, vA
        ld      vT+1, vA+1
        ld      r4, #vT
        ld      r5, #vA
        call    mulsi2          ; T=A*A
        ld      r5, #tmp
        call    subsi2          ; T=A*A-B*B
        ld      r5, #vF
        call    divsi2          ; T=(A*A-B*B)/F
        ld      r5, #vC
        call    addsi2          ; T=(A*A-B*B)/F+C
        ld      tmp, vA
        ld      tmp+1, vA+1
        ld      r4, #tmp
        ld      r5, #vS
        call    mulsi2          ; tmp=A*S
        ld      r5, #vF
        call    divsi2          ; tmp=A*S/F
        ld      vB, vA          ; B=A
        ld      vB+1, vA+1
        ld      r4, #vB
        ld      r5, #vQ
        call    mulsi2          ; B=A*Q
        ld      r5, #tmp
        call    addsi2          ; B=A*Q+A*S/F
        ld      r5, #vB
        call    addsi2          ; B=2*(A*Q+A*S/F)
        ld      r5, #vD
        call    addsi2          ; B=2*(A*Q+A*S/F)+D
        ld      vA, vT          ; A=T
        ld      vA+1, vT+1
        ld      vP, vA
        ld      vP+1, vA+1
        ld      r4, #vP
        ld      r5, #vF
        call    divsi2          ; P=A/F
        ld      vQ, vB
        ld      vQ+1, vB+1
        ld      r4, #vQ
        call    divsi2          ; Q=B/F
        ld      tmp, vQ
        ld      tmp+1, vQ+1
        ld      r4, #tmp
        ld      r5, #vQ
        call    mulsi2          ; tmp=Q*Q
        ld      vT, vP
        ld      vT+1, vP+1
        ld      r4, #vT
        ld      r5, #vP
        call    mulsi2          ; T=P*P
        ld      r5, #tmp
        call    addsi2          ; T=P*P+Q*Q
        ld      tmp, #HIGH 4
        ld      tmp+1, #LOW 4
        ld      r4, #tmp
        ld      r5, #vT
        call    cmpsi2          ; 4-T
        jr      c, print_i      ; if 4<T
        incw    vI
        ld      tmp, #HIGH 16
        ld      tmp+1, #LOW 16
        ld      r4, #vI
        ld      r5, #tmp
        call    cmpsi2
        jp      lt, loop_i      ; if I<16
        ld      r0, #' '
        call    putchar
        jr      next_x
print_i:
        ld      r0, vI+1
        add     r0, #'0'
        cp      r0, #'9'+1
        jr      c, print_i2     ; if I<'9'+1
        add     r0, #'A'-'0'-10
print_i2:
        call    putchar
next_x:
        incw    vX
        ld      tmp, #HIGH 30
        ld      tmp+1, #LOW 30
        ld      r4, #vX
        ld      r5, #tmp
        call    cmpsi2
        jp      lt, loop_x      ; if X<30
        ld      r0, #%0D
        call    putchar
        ld      r0, #%0A
        call    putchar
        incw    vY
        ld      tmp, #HIGH 13
        ld      tmp+1, #LOW 13
        ld      r4, #vY
        ld      r5, #tmp
        call    cmpsi2
        jp      lt, loop_y      ; if X<13
        ret

;;; Local Variables:
;;; mode: asm
;;; End:
;;; vim: set ft=asm et ts=4 sw=4:
