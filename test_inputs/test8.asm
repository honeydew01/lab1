; multiple segments
.ORIG x3000
    LEA R0,NUM
    LDB R1,R0,#0
    TRAP x25
.ORIG x3100
NUM .FILL x0042
.END

