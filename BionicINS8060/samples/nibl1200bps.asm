;;; NIBL Tiny Basic Console I/O

NUM:    equ     -21
TEMP:   equ     -22
TEMP2:  equ     -23
TEMP3:  equ     -24

;*************************************
;*     GET CHARACTER AND ECHO IT     *
;*************************************
        ORG     0x0F77
GECO:   LDI     8               ;SET COUNT = 8
        ST      NUM(P2)
        CSA                     ;SET READER RELAY
        ORI     2
        CAS
GETCO1: CSA                     ;WAIT FOR START BIT
        ANI     0x20
        JNZ     GETCO1          ;NOT FOUND
        LDI     61              ; DELAY 1/2 BIT TIME
        DLY     0
        CSA                     ; IS START BIT STILL THERE?
        ANI     0x20
        JNZ     GETCO1          ; NO
        CSA                     ;SEND START BIT
        ANI     0xFD            ; RESET READER RELAY
        ORI     1
        CAS
GETCO2: LDI     118             ; DELAY 1 BIT TIME
        DLY     0
        CSA                     ;GET BIT (SENSED)
        ANI     0x20
        JZ      GETCO3
        LDI     1
        JMP     GETCO4
GETCO3: LDI     0
        JNZ     GETCO4
GETCO4: ST      TEMP(P2)        ;SAVE BIT VALUE (0 OR 1)
        RRL                     ;ROTATE INTO LINK
        XAE
        SRL                     ; SHIFT INTO CHARACTER
        XAE                     ; RETURN CHAR TO E
        CSA                     ;ECHO BIT TO OUTPUT
        ORI     1
        XOR     TEMP(P2)
        CAS
        DLD     NUM(P2)         ;DECREMENT BIT COUNT
        JNZ     GETCO2          ;LOOP UNTIL 0
        LDI     118
        DLY     0               ; DELAY APPROX. 1 BIT TIME
        CSA                     ;SET STOP BIT
        ANI     0xFE
        CAS
        LDE                     ; AC HAS INPUT CHARCTER
        XAE
        LDE
        XPPC    P3              ;RETURN
        JMP     GECO

;*************************************
;*     PRINT CHARACTER  AT  TTY      *
;*************************************
        ORG     0x0FC2
PUTC:   XAE
        LDI     134              ; DELAY ALMOST
        DLY     1                ; 3  BIT  TIMES
        CSA                      ;SET OUTPUT  BIT  TO  LOGIC  0
        ORI     1                ; FOR START  BIT
        CAS
        LDI     9                ; INITIALIZE BIT COUNT
        ST      TEMP3(P2)
PUTC1:  LDI     129              ; DELAY 1 BIT TIME
        DLY     0
        DLD     TEMP3(P2)        ; DECREMENT BIT COUNT
        JZ      PUTC2
        LDE                      ; PREPARE NEXT BIT
        ANI     1
        ST      TEMP2(P2)
        XAE
        SR
        XAE
        CSA                      ; SET UP OUTPUT BIT
        ORI    1
        XOR    TEMP2(P2)
        CAS                      ; PUT BIT INTO TTY
        JMP    PUTC1
PUTC2:  CSA                      ; SET STOP BIT
        ANI    0xFE
        CAS
        XPPC   P3
        JMP    PUTC
