;;; -*- mode: asm; mode: flyspell-prog; -*-

;;; INS8070
;;; Status Register
S_CY:   equ     0x80            ; Carry bit
S_L:    equ     0x80            ; Link bit
S_OV:   equ     0x40            ; Overflow bit
S_SB:   equ     0x20            ; Sense B bit
S_SA:   equ     0x10            ; Sense A bit
S_F3:   equ     0x08            ; Flag 3 bit
S_F2:   equ     0x04            ; Flag 2 bit
S_F1:   equ     0x02            ; Flag 1 bit
S_IE:   equ     0x01            ; Interrupt Enable bit

;;; Transfer locations
ORG_RESTART:    equ     0x0001  ; Restart transfer location
ORG_INTA:       equ     0x0004  ; Interrupt A transfer location
ORG_INTB:       equ     0x0007  ; Interrupt B transfer location
ORG_END:        equ     0x000A

;;; Call vectors
VEC_CALL0:      equ     0x0020  ; Call 0 vector
VEC_CALL1:      equ     0x0022  ; Call 0 vector
VEC_CALL2:      equ     0x0024  ; Call 0 vector
VEC_CALL3:      equ     0x0026  ; Call 0 vector
VEC_CALL4:      equ     0x0028  ; Call 0 vector
VEC_CALL5:      equ     0x002A  ; Call 0 vector
VEC_CALL6:      equ     0x002C  ; Call 0 vector
VEC_CALL7:      equ     0x002E  ; Call 0 vector
VEC_CALL8:      equ     0x0030  ; Call 0 vector
VEC_CALL9:      equ     0x0032  ; Call 0 vector
VEC_CALL10:     equ     0x0034  ; Call 0 vector
VEC_CALL11:     equ     0x0036  ; Call 0 vector
VEC_CALL12:     equ     0x0038  ; Call 0 vector
VEC_CALL13:     equ     0x003A  ; Call 0 vector
VEC_CALL14:     equ     0x003C  ; Call 0 vector
VEC_CALL15:     equ     0x003E  ; Call 0 vector
VEC_END:        equ     0x0040
