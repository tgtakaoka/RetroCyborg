;;; -*- mode: asm; mode: flyspell-prog; -*-

;;; MOS6502
;;; Processor Status Register P
P_CARRY:        equ     %00000001   ; set to 1 if carry occurred
P_ZERO:         equ     %00000010   ; set to 1 if result is zero
P_IRQ:          equ     %00000100   ; if 1, IRQ is disabled
P_DECIMAL:      equ     %00001000   ; if 1, decimal mode
P_BRK:          equ     %00010000   ; if 1, BRK command
P_OVERFLOW:     equ     %00000010   ; set to 1 if overflow occurred
P_NEGATIVE:     equ     %00001000   ; set to 1 if result is negative

;;; Vector
VEC_NMI:        equ     $FFFA  ; $FFFA: Non Maskable Interrupt
VEC_RESET:      equ     $FFFC  ; $FFFE: Reset
VEC_IRQ:        equ     $FFFE  ; $FFFE: Interrupt Request and BRK command
