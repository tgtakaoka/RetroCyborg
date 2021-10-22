;;; -*- mode: asm; mode: flyspell-prog; -*-

;;; MC6801/MC6803
;;; Condition Code Register (CC)
CC_CARRY      equ  %00000001   ; set to 1 if carry occurred
CC_OVERFLOW   equ  %00000010   ; set to 1 if overflow occurred
CC_ZERO       equ  %00000100   ; set to 1 if result is zero
CC_NEGATIVE   equ  %00001000   ; set to 1 if result is negative
CC_IRQ        equ  %00010000   ; if 1, IRQ is masked
CC_HALF_CARRY equ  %00100000   ; if 1, decimal carry from least digit occurred

;;; Internal Register Area
        org     $00
DDR1:   rmb     1               ; $00: Port 1 Data Direction Register
DDR2:   rmb     1               ; $01: Port 2 Data Direction Register
PORT1:  rmb     1               ; $02: Port 1 Data Register
PORT2:  rmb     1               ; $03: Port 2 Data Register
DDR3:   rmb     1               ; $04: Port 3 Data Direction Register
DDR4:   rmb     1               ; $05: Port 4 Data Direction Register
PORT3:  rmb     1               ; $06: Port 3 Data Register
PORT4:  rmb     1               ; $07: Port 4 Data Register
TCSR:   rmb     1               ; $08: Timer Control and Status Register
TCNT:   rmb     2               ; $09: Timer Counter
TOCR:   rmb     2               ; $0B: Output Compare Register
TCAP:   rmb     2               ; $0D: Input Capture Register
P3CSR:  rmb     1               ; $0F: Port 3 Control and Status Register
RMCR:   rmb     1               ; $10: Rate and Mode Control Registers
TRCSR:  rmb     1               ; $11: Transmit Receive Control and Status Register
SCRDR:  rmb     1               ; $12: Receiver Data Register
SCTDR:  rmb     1               ; $13: Transmit Data Register
RAMCR:  rmb     1               ; $14: RAM Control Register
        rmb     11              ; $15-$1F: Reserved
end_IRA:        equ     *       ; end of internal register area

;;; Port 2 Data Register
PC_gm:  equ     %11100000       ; PC group mask
PC_gp:  equ     5               ; PC group position
PC0_bm: equ     %00100000       ; PC0
PC1_bm: equ     %01000000       ; PC1
PC2_bm: equ     %10000000       ; PC2

;;; Port 3 Control and Status Register
P3_LATCH_bm:    equ     %00001000 ; Latch Enable
P3_OSS_bm:      equ     %00010000 ; Output Strobe Select
P3_IS3IE_bm:    equ     %01000000 ; IS3 IRQ1 Enable
P3_IS3F_bm:     equ     %10000000 ; IS3 Flag

;;; RAM Control Register
RAME_bm:        equ     %01000000 ; RAM Enable
STBY_PWR_bm:    equ     %10000000 ; Standby Power status

;;; Timer Control and Status Register
TCSR_OLVL_bm:   equ     %00000001 ; Output Level
TCSR_IEDG_bm:   equ     %00000010 ; Input Edge. 0=negative/1=positive
TCSR_ETOI_bm:   equ     %00000100 ; Enable Timer Overflow Interrupt for IRQ2
TCSR_EOCI_bm:   equ     %00001000 ; Enable Output Compare Interrupt for IRQ2
TCSR_EICI_bm:   equ     %00010000 ; Enable Input Capture Interrupt for IRQ2
TCSR_TOF_bm:    equ     %00100000 ; Timer Overflow Flag
TCSR_OCF_bm:    equ     %01000000 ; Output Compare Flag
TCSR_ICF_bm:    equ     %10000000 ; Input Capture Flag

;;; Serial Communication Interface
;;; Rate and Mode Control Registers
        ;; Speed Select
SS_gm:          equ     %00000011 ; Group mask
SS_DIV16_gc:    equ     %00000000 ; E/16
SS_DIV128_gc:   equ     %00000001 ; E/128
SS_DIV1024_gc:  equ     %00000010 ; E/1024
SS_DIV4096_gc:  equ     %00000011 ; E/4096
        ;; Clock Control and Format Select
CCFS_gm:        equ     %00001100 ; Group mask
CCFS_BIPH_gc:   equ     %00000000 ; Bi-Phase
CCFS_NRZ_gc:    equ     %00000100 ; NRZ
CCFS_NRZO_gc:   equ     %00001000 ; NRZ P22=Clock Output
CCFS_NRZI_gc:   equ     %00001100 ; NRZ P22=Clock Input
;;; Transmit/Receive Control and Status Register
TRCSR_WU_bm:    equ     %00000001 ; Wake-up on idle line
TRCSR_TE_bm:    equ     %00000010 ; Transmit Enable; if set, P24 is output
TRCSR_TIE_bm:   equ     %00000100 ; Transmit Interrupt Enable for IRQ2
TRCSR_RE_bm:    equ     %00001000 ; Receiver Enable; if set, P23 is input
TRCSR_RIE_bm:   equ     %00010000 ; Receiver Interrupt Enable for IRQ2
TRCSR_TDRE_bm:  equ     %00100000 ; Transmit Data Register Empty
TRCSR_ORFE_bm:  equ     %01000000 ; Overrun Framing Error
TRCSR_RDRF_bm:  equ     %10000000 ; Receive Data Register Full

;;; Vector
        org     $FFF0
VEC_SCI:        rmb     2       ; $FFF0: SCI Interrupt (TDRE+RDRF+ORFE)
VEC_TOF:        rmb     2       ; $FFF2: Timer Overflow Interrupt
VEC_OCF:        rmb     2       ; $FFF4: Output Compare Interrupt
VEC_ICF:        rmb     2       ; $FFF6: Input Capture Interrupt
VEC_IRQ1:       rmb     2       ; $FFF8: Maskable Interrupt Request 1
VEC_SWI:        rmb     2       ; $FFFA: Software Interrupt
VEC_NMI:        rmb     2       ; $FFFC: Non Maskable Interrupt
VEC_RESET:      rmb     2       ; $FFFE: Reset
