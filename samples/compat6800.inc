;;; -*- mode: asm; mode: flying-spell; -*-
;;; MC6800 compatible mcaros
	listing off

;;; Clear Carry
clc:    macro
        andcc   #~$01
        endm

;;; Clear IRQ flag
cli:    macro
        andcc  	#~$10
        endm

;;; Clear Overflow flag
clv:    macro
        andcc	#~$02
        endm

;;; Set Carry flag
sec:    macro
        orcc	#$01
        endm

;;; Set IRQ flag
sei:    macro
        orcc	#$10
        endm

;;; Set Overflow flag
sev:    macro
        orcc	#$02
        endm

;;; Decrement S
des:    macro
        leas	-1,s
        endm

;;; Decrement X
dex:    macro
        leax	-1,x
        endm

;;; Increment S
ins:    macro
        leas	1,s
        endm

;;; Increment X
inx:    macro
        leax	1,x
        endm

;;; Push A
psha:   macro
        pshs    a
        endm

;;; Push B
pshb:   macro
        pshs    b
        endm

;;; Push X
pshx:   macro
        pshs    x
        endm

;;; Pull A
pula:   macro
        puls    a
        endm

;;; Pull B
pulb:   macro
        puls    b
        endm

;;; Pull X
pulx:   macro
        puls    x
        endm

;;; Add B to A
aba:    macro
        pshs    b
        adda    ,s+
        endm

;;; Compare B with A
cba:    macro
        pshs    b
        cmpa    ,s+
        endm

;;; Subtract B from A
sba:    macro
        pshs    b
        suba    ,s+
        endm

;;; Transfer A to B and set flags
tab:    macro
        tfr     a,b
        tsta
        endm

;;; Transfer A to CC
tap:    macro
        tfr     a,cc
        endm

;;; Transfer B to A and set flags
tba:    macro
        tfr     b,a
        tsta
        endm

;;; Transfer CC to A
tpa:    macro
        tfr     cc,a
        endm

;;; Transfer S to X
tsx:    macro
        tfr     s,x
        endm

;;; Transfer X to S
txs:    macro
        tfr     x,s
        endm

;;; Wait for interrupt
wai:    macro
        cwai	#$ff
        endm

;;; Arithmetic shift left D
asld:   macro
        aslb
        rola
        endm

;;; Arithmetic shift right D
asrd:   macro
        asra
        rorb
        endm

;;; Clear D
clrd:   macro
        clra
        clrb
        endm

;;; Complement D
comd:   macro
        coma
        comb
        endm

;;; Logical shift left D
lsld:   macro
        lslb
        rola
        endm

;;; Logical shift right D
lsrd:   macro
        lsra
        rorb
        endm

;;; Roll left D
rold:   macro
        rolb
        rola
        endm

;;; Roll right D
rord:   macro
        rora
        rorb
        endm

        listing on
