        cpu     8070

COLD4:          equ     0x0048
BAUDFLG:        equ     0xFD00

;;; Baudrate flag
        org     BAUDFLG
        ;; bit7: 0=external, 1=internal (#F1/SA)
        ;; bit2-1: 0=4800bps, 1=1200bps, 2=300bps, 3=110bps
        db      (1 << 7) | (0 << 1)

;;; Override RAM end check
        org     COLD4
        ld      p2, =0x7fff
        ld      a, @2, p2
