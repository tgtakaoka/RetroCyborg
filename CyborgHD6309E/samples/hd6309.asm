        cpu     6309
        include "hd6309.inc"

        org     $F000
stack:  equ     *

        org     $1000

mc6809: lda     $0E
        ldmd    #0
        nop
        bra     mc6809

hd6309: lda     $06
        ldmd    #MD_EMULATION
        nop
        bra     hd6309

        org     $FFFC
        fdb     mc6809
        fdb     hd6309
