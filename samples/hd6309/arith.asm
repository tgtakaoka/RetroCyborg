        cpu     6309
        include "mc6809.inc"

;;; MC6850 Asynchronous Communication Interface Adapter
ACIA:   equ     $DF00
        include "mc6850.inc"

        org     $1000
stack:  equ     *-1             ; MC6800's SP is post-decrement/pre-increment

        org     VEC_SWI
        fdb     VEC_SWI         ; for halt to system

        org     VEC_RESET
        fdb     initialize

        org     $0100
initialize:
        ldmd    #1
        lds     #stack
        lda     #CDS_RESET_gc   ; Master reset
        sta     ACIA_control
        lda     #WSB_8N1_gc     ; 8 bits + No Parity + 1 Stop Bits
                                ; Transmit, Receive interrupts disabled
        sta     ACIA_control

        jsr     arith
        swi

;;; Print out char
;;; @param A char
;;; @clobber A B
putspace:
        lda     #' '
        bra     putchar
newline:
        lda     #$0D
        bsr     putchar
        lda     #$0A
putchar:
        ldb     ACIA_status
        bitb    #TDRE_bm
        beq     putchar
        sta     ACIA_data
        rts

;;; Print "X op U"
;;; @params A op letter
;;; @return W=X
;;; @return D=U
expr:
        pshs    A
        tfr     X, D
        jsr     print_int16     ; print X
        bsr     putspace
        puls    A
        bsr     putchar         ; print op
        bsr     putspace
        tfr     U, D
        jsr     print_int16     ; print U
        tfr     X, W
        rts

;;; Print " = W\n"
answer:
        bsr     putspace
        lda     #'='
        bsr     putchar
        bsr     putspace
        tfr     W, D
        jsr     print_int16     ; print W
        bra     newline

;;; Print "X rel U"
;;; @clobber D
comp:
        cmpr    U, X            ; X-U
        beq     comp_eq
        blt     comp_lt
        bgt     comp_gt
        lda     #'?'
        bra     comp_out
comp_gt:
        lda     #'>'
        bra     comp_out
comp_eq:
        lda     #'='
        bra     comp_out
comp_lt:
        lda     #'<'
comp_out:
        bsr     expr
        bra     newline

arith:
        ldx     #18000
        ldu     #28000
        lda     #'+'
        jsr     expr
        addr    D, W            ; W+=D
        jsr     answer          ; -19536

        ldx     #18000
        ldu     #-18000
        lda     #'+'
        jsr     expr
        addr    D, W            ; W+=D
        jsr     answer          ; 0

        ldx     #-18000
        ldu     #-18000
        lda     #'+'
        jsr     expr
        addr    D, W            ; W+=D
        jsr     answer          ; 29536

        ldx     #-18000
        ldu     #-28000
        lda     #'-'
        jsr     expr
        subr    D, W            ; W-=D
        jsr     answer          ; -19536

        ldx     #18000
        ldu     #-18000
        lda     #'-'
        jsr     expr
        subr    D, W            ; W-=D
        jsr     answer          ; 29536

        ldx     #-28000
        ldu     #-18000
        lda     #'-'
        jsr     expr
        subr    D, W            ; W-=D
        jsr     answer          ; -10000

        ldx     #100
        ldu     #300
        lda     #'*'
        jsr     expr
        pshsw
        muld    ,S++            ; D:W=D*EA
        jsr     answer          ; 30000

        ldx     #200
        ldu     #100
        lda     #'*'
        jsr     expr
        pshsw
        muld    ,S++            ; D:W=D*EA
        jsr     answer          ; 20000

        ldx     #300
        ldu     #-200
        lda     #'*'
        jsr     expr
        pshsw
        muld    ,S++            ; D:W=D*EA
        jsr     answer          ; 5536

        ldx     #100
        ldu     #-300
        lda     #'*'
        jsr     expr
        pshsw
        muld    ,S++            ; D:W=D*EA
        jsr     answer          ; -30000

        ldx     #-200
        ldu     #-100
        lda     #'*'
        jsr     expr
        pshsw
        muld    ,S++            ; D:W=D*EA
        jsr     answer          ; 20000

        ldx     #30000
        ldu     #100
        lda     #'/'
        jsr     expr
        pshs    D
        sexw
        divq    ,S++            ; D:W/EA=W...D
        jsr     answer          ; 30

        ldx     #-200
        ldu     #100
        lda     #'/'
        jsr     expr
        pshs    D
        sexw
        divq    ,S++            ; D:W/EA=W...D
        jsr     answer          ; -2

        ldx     #-30000
        ldu     #-200
        lda     #'/'
        jsr     expr
        pshs    D
        sexw
        divq    ,S++            ; D:W/EA=W...D
        jsr     answer          ; 150

        ldx     #-30000
        ldu     #78
        lda     #'/'
        jsr     expr
        pshs    D
        sexw
        divq    ,S++            ; D:W/EA=W...D
        jsr     answer          ; -384

        ldx     #5000
        ldu     #4000
        jsr     comp

        ldx     #5000
        ldu     #5000
        jsr     comp

        ldx     #4000
        ldu     #5000
        jsr     comp

        ldx     #-5000
        ldu     #-4000
        jsr     comp

        ldx     #-5000
        ldu     #-5000
        jsr     comp

        ldx     #-4000
        ldu     #-5000
        jsr     comp

        ldx     #32700
        ldu     #32600
        jsr     comp

        ldx     #32700
        ldu     #32700
        jsr     comp

        ldx     #32600
        ldu     #32700
        jsr     comp

        ldx     #-32700
        ldu     #-32600
        jsr     comp

        ldx     #-32700
        ldu     #-32700
        jsr     comp

        ldx     #-32600
        ldu     #-32700
        jsr     comp

        ldx     #18000
        ldu     #-28000
        jsr     comp

        ldx     #-28000
        ldu     #-28000
        jsr     comp

        ldx     #-28000
        ldu     #18000
        jsr     comp
        rts

        include "arith.inc"
