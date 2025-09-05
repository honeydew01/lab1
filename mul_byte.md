# Multiplication with ADD and Bit Shifting

We can multiply two numbers efficiently without a loop the size of one of the factors by decomposing one of the factors into powers of 2. Then we create bit shifted versions of the other factor in terms of the decomposed factor and then add the results together to get the answer. 

### Example:

Lets multiply `A = 101101` by `B = 10111`. 

We can decompose B into:

```
B = 1 * 2^4 + 0 * 2^3 + 1 * 2^2 + 1 * 2^1 + 1 * 2^0
```

We can then multiply A with B to get:

```
A * B = (101101) * (1 * 2^4 + 0 * 2^3 + 1 * 2^2 + 1 * 2^1 + 1 * 2^0)
= (101101 * (1 * 2^4)) + (101101 * (0 * 2^3)) + (101101 * (1 * 2^2)) + (101101 * (1 * 2^1)) + (101101 * (1 * 2^0))
```

And we see that every `x * 2^y` can just be represented as a bitshift. So in essence, we just need a shift and accumulate function to do the multiplication. 

```
A * B = 1*(101101 << 4) + 0*(101101 << 3) + 1*(101101 << 2) + 1*(101101 << 1) + 1*(101101 << 0)
```

## mul_bytes.asm Code Analysis

```asm
.ORIG	x3000
    LEA	R0,	f1_adr      ; Get factor 1 Address
    LDB R1, R0, #0      ; Load factor 1 into R1
    LEA R0, f2_adr      ; Get factor 2 Address
    LDB R2, R0, #0      ; Load factor 2 into R2

    AND R3, R3, #0      ; Initialize product to 0
    AND R4, R4, #0      ; Initialize overflow to 0
```

Let:
- R1: 8-bit Factor 1
- R2: 8-bit Factor 2
- R3: 16-bit intermediate product
- R4: Overflow flag register

```asm
LOOP ADD R2, R2, #0     ; For some reason, label cannot have its own line or reference assembler throws error 2
    BRz EXIT            ; Exit if factor 2 = 0
    AND R5, R2, #1
    BRz CONTINUE        ; Skip addition if factor 2 LSB = 0
    ADD R3, R3, R1      ; Add factor 1 to result
```

I believe the error is the correct. I think its a part of the wacky LC3-B asm they spec'd. 

The first `ADD` instruction checks if R2 is 0. If so, we are done. Otherwise, we continue the operation.

The `AND` instruction retrieves the LSB of the second factor and stores it in R5. We then check if the LSB was zero or not. If so, we add the first factor (which is left shifted to the correct power of 2) and add it to the accumulator register.

```asm
CONTINUE LSHF R1, R1, #1 ; Left shift factor 1
    RSHFL R2, R2, #1    ; Right shift factor 2
    BR LOOP
```

Regardless of if we added a shifted version of factor 1, we left shift factor 1 to prepare for the next accumulate. 

We also right shift factor 1 to move the next bianry digit to the LSB place in R2. 

```asm    
EXIT AND R5, R5, #0     ; Clear R5
    LSHF R5, R3, #8     ; Check top byte for overflow
    BRz NO_OVERFLOW
    ADD R4, R4, #1      ; Indicate overflow
```

When we get here, we R2 is zero and we are done with the multiplication. We then check for overflow by left shifting the top 8 bits in the product register into the bottom 8 bits.

```asm    
NO_OVERFLOW LEA R1, P_adr ; Get product address
    STB R3, R1, #0      ; Store product into memory
    LEA R1, OF_adr      ; Get overflow address
    STB R4, R1, #0      ; Store overflow into memory

f1_adr .FILL	x3100   ; Factor 1 Address
f2_adr .FILL	x3101   ; Factor 2 Address
P_adr  .FILL	x3102   ; Product Address
OF_adr .FILL	x3103   ; Overflow Address

.END
```

Then we do basic store operations and we are done. 

```c++
void mul_bytes(){
    uint16_t factor_1; /* Both Factor 1 and 2 are values from 0 to 0xFFF */
    uint16_t factor_2;

    uint16_t product = 0;
    bool overflow = false;

    /* load values from memory */

    while(factor_2 != 0){
        bool factor_2_lsb = factor_2 & 0x1;

        if(factor_2_lsb){
            product += factor_1;
        }

        factor_1 <<= 1;
        factor_2 >>= 1;
    }

    if(product >> 8){
        overflow = true;
    }

    /* Store operations for product and overflow registers */
}


```