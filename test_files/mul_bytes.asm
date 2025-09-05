

.ORIG	x3000
    LEA	R0,	f1_adr      ; Get factor 1 Address
    LDB R1, R0, #0      ; Load factor 1 into R1
    LEA R0, f2_adr      ; Get factor 2 Address
    LDB R2, R0, #0      ; Load factor 2 into R2

    AND R3, R3, #0      ; Initialize product to 0
    AND R4, R4, #0      ; Initialize overflow to 0

LOOP ADD R2, R2, #0     ; For some reason, label cannot have its own line or reference assembler throws error 2
    BRz EXIT            ; Exit if factor 2 = 0
    AND R5, R2, #1
    BRz CONTINUE        ; Skip addition if factor 2 LSB = 0
    ADD R3, R3, R1      ; Add factor 1 to result

CONTINUE LSHF R1, R1, #1 ; Left shift factor 1
    RSHFL R2, R2, #1    ; Right shift factor 2
    BR LOOP
    
EXIT AND R5, R5, #0     ; Clear R5
    RSHFL R5, R3, #8     ; Check top byte for overflow
    BRz NO_OVERFLOW
    ADD R4, R4, #1      ; Indicate overflow
    
NO_OVERFLOW LEA R1, P_adr ; Get product address
    STB R3, R1, #0      ; Store product into memory
    LEA R1, OF_adr      ; Get overflow address
    STB R4, R1, #0      ; Store overflow into memory

f1_adr .FILL	x3100   ; Factor 1 Address
f2_adr .FILL	x3101   ; Factor 2 Address
P_adr  .FILL	x3102   ; Product Address
OF_adr .FILL	x3103   ; Overflow Address

.END