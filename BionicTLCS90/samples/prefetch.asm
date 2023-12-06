        cpu     tlcs90
        org     0
        ld      sp, 1000H
        ld      hl, 2000H
        ld      de, 3000H
        jp      pointer

dir:    equ     0FFF8H

        org     1000H
direct:
        ld      ix, 0FFFFH
        inc     ix
        incx    (dir)
        inc     ix
        decx    (dir)
        ldw     (dir), 1234H
        inc     (dir)
        dec     (dir)
        incw    (dir)
        decw    (dir)
        res     0,(dir)
        set     0,(dir)
        ld      (dir),hl
        ld      a,0
        swi

pointer:
        inc     (de)
        dec     (de)
        incw    (de)
        decw    (de)
        res     0,(de)
        set     0,(de)
        tset    0,(de)
        rld     (de)
        rrd     (de)
        rlc     (de)
        ld      (de),h
        ldw     (de),5678H
        ld      (de),hl
        add     (de),89H
        ld      a,0
        swi

block:
        ld      bc, 4
        ldir
        ld      bc, 4
        lddr
        ld      bc, 4
        cpir
        ld      bc, 4
        cpdr
        ld      a,0
        swi
