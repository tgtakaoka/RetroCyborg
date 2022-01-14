;;; -*- mode: asm; mode: flyspell-prog; -*-

        include "mos6502.inc"

;;; W65C816S
;;; Processor Status Register P
P_X:    equ     %00010000       ; if 0, X and Y are 16 bit.
P_M:    equ     %00100000       ; if 0, A is 16 bit.

;;; Emulation Mode Vector Locations
VEC_ABORT:      equ     $FFF8   ; $FFF8: Abort
VEC_COP:        equ     $FFF4   ; $FFF4: COP command

;;; Native Mode Vector Locations
NVEC_IRQ:       equ     $FFEE   ; $FFEE: Interrupt Request
NVEC_NMI:       equ     $FFEA   ; $FFEA: Non Maskable Interrupt
NVEC_ABORT:     equ     $FFE8   ; $FFE8: Abort
NVEC_BRK:       equ     $FFE6   ; $FFE6: BRK command
NVEC_COP:       equ     $FFE4   ; $FFE4: COP command
