;;; -*- mode: asm; mode: flyspell-prog; -*-

;;; INS8060
;;; Status Register
S_CY:   equ     0x80            ; Carry bit
S_L:    equ     0x80            ; Link bit
S_OV:   equ     0x40            ; Overflow bit
S_SB:   equ     0x20            ; Sense B bit
S_SA:   equ     0x10            ; Sense A bit
S_IE:   equ     0x08            ; Interrupt Enable bit
S_F2:   equ     0x04            ; Flag 2 bit
S_F1:   equ     0x02            ; Flag 1 bit
S_F0:   equ     0x01            ; Flag 0 bit

;;; Dynamic Offset by E register
OFFSET_E:       equ     0x80    ; offset by E register

;;; Transfer locations
ORG_RESTART:    equ     0x0001  ; Restart transfer location
