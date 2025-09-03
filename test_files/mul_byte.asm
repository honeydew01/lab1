

.ORIG	x3000
    LEA	R0,	f1_adr      ; Get factor 1 Address
    LDB R1, R0, 0       ; Load factor 1 into R1
    LEA R0, f2_adr      ; Get factor 2 Address
    LDB R2, R0, 0       ; Load factor 2 into R2

    RSHFL R2, R2, 8     ; Right shift the second factor to the top byte of R2. 
    ; We will use the MSB of R2 to detect if a digit in factor 2 is 1 or 0. (using the negative flag)

loop: 




    


f1_adr: .FILL	x3100 ; Factor 1 Address
f2_adr: .FILL	x3101 ; Factor 2 Address
P_adr:  .FILL	x3102 ; Product Address
OF_adr: .FILL	x3103 ; Overflow Address

.END