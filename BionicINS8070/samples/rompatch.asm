        cpu     8070

COLD4:          equ     0x0048
CHKBRK:         equ     0x091E
INITBD:         equ     0x09E5
ENDNIBL2:       equ     0x0A00
BAUDFLG:        equ     0xFD00

;;; MC6850 ACIA
ACIA:   equ     0xDF00
ACIA_C: equ     0               ; ACIA control offset
ACIA_S: equ     0               ; ACIA status offset
ACIA_D: equ     1               ; ACIA data register offset

;;; Baudrate flag
        org     BAUDFLG
        db      0               ; bit7:0 call extenal routine
        dw      getchar
        dw      putchar

;;; Override RAM end check
        org     COLD4
        ld      p2, =0x7fff
        ld      a, @2, p2

;;; Break check
        org     CHKBRK + 4
        jsr     check_break

;;; Override initialize console
        org     INITBD          ; override INITBD
init_console:
        pli     p2, =ACIA
        ld      a, =3           ; master reset
        st      a, ACIA_C, p2
        ld      a, =0x14        ; 8bits + no parity + 1stop
        st      a, ACIA_C, p2
        pop     p2
        ret

;;; New code

        org     ENDNIBL2

check_break:
        pli     p2, =ACIA
        ld      a, ACIA_S, p2
        and     a, =1           ; bit0: receive data ready flag
        bz      no_break
        ld      a, ACIA_D, p2   ; receive char
        xor     a, =0x03        ; Control-C
        bz      got_break
no_break:
        ld      a, =1
got_break:
        pop     p2
        ret

getchar:
        pli     p2, =ACIA
        push    a
getchar_loop:
        ld      a, ACIA_S, p2
        and     a, =1           ; bit0:1 receive data ready flag
        bz      getchar_loop
        ld      a, ACIA_D, p2   ; receive char
        st      a, 0, sp
        xor     a, =0x0a        ; newline
        bz      getchar_loop
        pop     a
        pop     p2
        ;; fall-through to putchar

putchar:
        pli     p2, =ACIA
        push    a               ; save char
putchar_loop:
        ld      a, ACIA_S, p2
        and     a, =2           ; bit1:1 transmit data register empty
        bz      putchar_loop
        pop     a
        push    a
        and     a, =0x7f        ; clear MSB
        st      a, ACIA_D, p2
        pop     a
        pop     p2
        ret
