# Thumb instructions

Thumb instructions generally use the first 5 bits to decide which operation to use, though sometimes the next 2 or 5 bits are used to add more instructions that only use 3 or 2 registers (RegOp3b, RegOp0b).

The following notation is used for instructions;
**Rd** = Destination register (r0-r7)
**Rs** = Source register (r0-r7)
**Rn** = Intermediate register
**Hd** = Destination register (r8-r15)
**Hs** = Source register (r8-r15)
**#x** = a x-bit constant (i)
**#xs** = a x-bit signed constant (i)

| opCode         | Instruction                      | Name                                                         | Layout   | Condtion code |
| -------------- | -------------------------------- | ------------------------------------------------------------ | -------- | ------------- |
| 0 0000         | LSL Rd, Rs, #5                   | Logical Shift Left (<<)                                      | RegOp5b  | +             |
| 0 0001         | LSR Rd, Rs, #5                   | Logical Shift Right (>>)                                     |          | +             |
| 0 0010         | ASR Rd, Rs, #5                   | Arithmetic Shift Right                                       |          | +             |
| 0 0011 00      | ADD Rd, Rs, Rn                   | Addition (+)                                                 | RegOp3b  | +             |
| 0 0011 01      | SUB Rd, Rs, Rn                   | Subtraction (-)                                              |          | +             |
| 0 0011 10      | ADD Rd, Rs, #3                   | Addition (+)                                                 |          | +             |
| 0 0011 11      | SUB Rd, Rs, #3                   | Subtraction (-)                                              |          | +             |
| 0 0100         | MOV Rd, #8                       | Move (=)                                                     | RegOp8b  | +             |
| 0 0101         | CMP Rd, #8                       | Compare                                                      |          | +             |
| 0 0110         | ADD Rd, #8                       | Add assign (+=)                                              |          | +             |
| 0 0111         | SUB Rd, #8                       | Subtract assign (-=)                                         |          | +             |
| 0 1000 0 0000  | AND Rd, Rs                       | Bitwise and assign (&=)                                      | RegOp0b  | +             |
| 0 1000 0 0001  | EOR Rd, Rs                       | Bitwise xor assign (^=)                                      |          | +             |
| 0 1000 0 0010  | LSL Rd, Rs                       | Logical shift left assign (<<=)                              |          | +             |
| 0 1000 0 0011  | LSR Rd, Rs                       | Logical shift right assign (>>=)                             |          | +             |
| 0 1000 0 0100  | ASR Rd, Rs                       | Arithmetic shift right assign (maintains sign)               |          | +             |
| 0 1000 0 0101  | ADC Rd, Rs                       | Assign add with carry (+=)                                   |          | +             |
| 0 1000 0 0110  | SBC Rd, Rs                       | Subtract with carry (-=)                                     |          | +             |
| 0 1000 0 0111  | ROR Rd, Rs                       | ROtate Right (>>>= in Java)                                  |          | +             |
| 0 1000 0 1000  | TST Rd, Rs                       | Rd & Rs (set condition flags)                                |          | +             |
| 0 1000 0 1001  | NEG Rd, Rs                       | Negate (= -x)                                                |          | +             |
| 0 1000 0 1010  | CMP Rd, Rs                       | Rd - Rs (set condition flags)                                |          | +             |
| 0 1000 0 1011  | CMN Rd, Rs                       | Rd + Rs (set condition flags)                                |          | +             |
| 0 1000 0 1100  | ORR Rd, Rs                       | Bitwise or (\|=)                                             |          | +             |
| 0 1000 0 1101  | MUL Rd, Rs                       | Multiply (*=)                                                |          | +             |
| 0 1000 0 1110  | BIC Rd, Rs                       | Bitwise nand (&= ~Rs)                                        |          | +             |
| 0 1000 0 1111  | MVN Rd, Rs                       | Bitwise not (= ~Rs)                                          |          | +             |
| 0 1000 1 0000  | <u>undefined</u>                 |                                                              |          |               |
| 0 1000 1 0001  | ADD Rd, Hs                       | ADD r8-r15 to r0-r7 (+=)                                     |          | +             |
| 0 1000 1 0010  | ADD Hd, Rs                       | ADD r0-r7 to r8-r15 (+=)                                     |          | +             |
| 0 1000 1 0011  | ADD Hd, Hs                       | ADD r8-r15 to r8-15 (+=)                                     |          | +             |
| 0 1000 1 0100  | <u>undefined</u>                 |                                                              |          |               |
| 0 1000 1 0101  | CMP Rd, Hs                       | CMP r8-r15 to r0-r7                                          |          | +             |
| 0 1000 1 0110  | CMP Hd, Rs                       | CMP r0-r7 to r8-r15                                          |          | +             |
| 0 1000 1 0111  | CMP Hd, Hs                       | CMP r8-r15 to r8-15                                          |          | +             |
| 0 1000 1 1000  | <u>undefined</u>                 |                                                              |          |               |
| 0 1000 1 1001  | MOV Rd, Hs                       | MOV r8-r15 to r0-r7                                          |          | +             |
| 0 1000 1 1010  | MOV Hd, Rs                       | MOV r0-r7 to r8-r15                                          |          | +             |
| 0 1000 1 1011  | MOV Hd, Hs                       | MOV r8-r15 to r8-15                                          |          | +             |
| 0 1000 1 1100  | BX Rs                            | MOV pc, Rs                                                   |          | -             |
| 0 1000 1 1101  | BX Hs                            | MOV pc, Hs                                                   |          | -             |
| 0 1000 1 1110  | <u>undefined</u>                 |                                                              |          |               |
| 0 1000 1 1111  | <u>undefined</u>                 |                                                              |          |               |
| 0 1001         | LDR Rd, [PC, #8]                 | Rd =data[pc NAND 3 + 4 + i << 2]<br />Load u32 relative to the next instruction. Specify i in u32s. | RegOp8b  | -             |
| 0 1010 00      | STR Rd, [Rs, Rn]                 | u32[Rs + Rn] = Rd                                            | RegOp3b  | -             |
| 0 1010 01      | STRH Rd, [Rs, Rn]                | u16[Rs + Rn] = Rd                                            |          | -             |
| 0 1010 10      | STRB Rd, [Rs, Rn]                | u8[Rs + Rn] = Rd                                             |          | -             |
| 0 1010 11      | LDSB Rd, [Rs, Rn]                | Rd = i8[Rs + Rn]                                             |          | -             |
| 0 1011 00      | LDR Rd, [Rs, Rn]                 | Rd = u32[Rs + Rn]                                            |          | -             |
| 0 1011 01      | LDRH Rd, [Rs, Rn]                | Rd = u16[Rs + Rn]                                            |          | -             |
| 0 1011 10      | LDRB Rd, [Rs, Rn]                | Rd = u8[Rs + Rn]                                             |          | -             |
| 0 1011 11      | LDSH Rd, [Rs, Rn]                | Rd = i16[Rs + Rn]                                            |          | -             |
| 0 1100         | STR Rd, [Rs, #5]                 | u32[Rs + i << 2] = Rd                                        | RegOp5b  | -             |
| 0 1101         | LDR Rd, [Rs, #5]                 | Rd = u32[Rs + i << 2]                                        |          | -             |
| 0 1110         | STRB Rd, [Rs, #5]                | u8[Rs + i] = Rd                                              |          | -             |
| 0 1111         | LDRB Rd, [Rs, #5]                | Rd = u8[Rs + i]                                              |          | -             |
| 1 0000         | STRH Rd, [Rs, #5]                | u16[Rs + i << 1] = Rd                                        |          | -             |
| 1 0001         | LDRH Rd, [Rs, #5]                | Rd = u16[Rs + i << 1]                                        |          | -             |
| 1 0010         | STR Rd, [SP, #8]                 | u32[SP + i << 2] = Rd                                        | RegOp8b  | -             |
| 1 0011         | LDR Rd, [SP, #8]                 | Rd = u32[SP + i << 2]                                        |          | -             |
| 1 0100         | ADD Rd, PC, #8                   | Rd = PC + i << 2                                             |          | +             |
| 1 0101         | ADD Rd, SP, #8                   | Rd = SP + i << 2                                             |          | +             |
| 1 0110 000     | ADD SP, #8s                      | b ? SP -= i << 2 : SP += i << 2                              | RegOp7b  | +             |
| 1 0110 001     | <u>undefined</u>                 |                                                              |          |               |
| 1 0110 010     | <u>undefined</u>                 |                                                              |          |               |
| 1 0110 011     | <u>undefined</u>                 |                                                              |          |               |
| 1 0110 100     | PUSH { Rs..., Rs... }            | Push the specified registers onto the stack and update the SP. i is a bitflag for x << regId. |          | -             |
| 1 0110 101     | PUSH { Rs..., Rs..., LR }        | Push the registers and the LR onto the stack and update the SP. i is a bitflag for x << regId |          | -             |
| 1 0110 110     | <u>undefined</u>                 |                                                              |          |               |
| 1 0110 111     | <u>undefined</u>                 |                                                              |          |               |
| 1 0111 000     | <u>undefined</u>                 |                                                              |          |               |
| 1 0111 001     | <u>undefined</u>                 |                                                              |          |               |
| 1 0111 010     | <u>undefined</u>                 |                                                              |          |               |
| 1 0111 011     | <u>undefined</u>                 |                                                              |          |               |
| 1 0111 100     | POP { Rs..., Rs... }             | Pop the stack and put the values into the specified registers. i is a bitflag for x << regId |          | -             |
| 1 0111 101     | POP { Rs..., Rs..., PC }         | Pop the stack and put the values into the specified registers and PC. i is a bitflag for x << regId |          | -             |
| 1 0111 110     | <u>undefined</u>                 |                                                              |          |               |
| 1 0111 111     | <u>undefined</u>                 |                                                              |          |               |
| 1 1000         | STMIA Rb!, { Rs..., Rs... }      | **TODO:**                                                    | RegOp8b  | -             |
| 1 1001         | LDMIA Rb!,{ Rs..., Rs... }       | **TODO:**                                                    |          | -             |
| 1 1010, 1 1011 | B{cond} #8s                      | Increments PC by i << 1. cond = 1110 is undefined            | RegOp12b | -             |
| 1 1011 111     | SWI #8                           | Causes a software interrupt. LR = next pc. CPSR into SPSR_SVC. SWI vector address 0x8 into the PC. Switch to arm mode, mode to SVC. SWI handler uses i to determine what the interrupt should do (depending on the system) |          | -             |
| 1 1100         | B #11                            | Unconditional branch                                         | RegOp11b | -             |
| 1 1101         | <u>undefined</u>                 |                                                              |          |               |
| 1 1110, 1 1111 | BL #23<br />BLH #11<br />BLL #11 | Long branch; 23-bit jump.<br />Long branch high: LR = i << 12<br />Long branch low: LR \|= i <<  1<br />LR is relative to the next pc. PC is set to LR and the previous next pc \| 1 is placed into LR. |          | -             |

## r15 (pc register)

A mov to the pc (r15) via a branch will cause the thumb mode to switch if bit 0 is set (1). This means branches can jump to ARM code.

If the pc is touched through an instruction it is set to the address of the next ARM instruction. pc NAND 2 + 4 where pc is the pc before the instruction. If pc is non WORD aligned this will result in unpredictable execution (pc AND 2).

## RegOp

"RegOp" stands for regular operation, there are 3 variants; RegOp0b, RegOp3b, RegOp5b, RegOp8b. The xb stands for how many bits are used for data used besides the destination registers and the op code. The layout is as follows:

```cpp
struct RegOp0b {
  
    u16 Rd : 3;		//Destination register
    u16 Rs : 3;		//Source register
    u16 op : 10;	//Opcode
    
};

struct RegOp3b {
  
    u16 Rd : 3;		//Destination register
    u16 Rs : 3;		//Source register
    u16 Rni : 3;	//Rn or intermediate
    u16 op : 7;		//Opcode
    
};

struct RegOp5b {
  
    u16 Rd : 3;		//Destination register
    u16 Rs : 3;		//Source register
    u16 i : 5;		//Intermediate
    u16 op : 5;		//Opcode
    
};

struct RegOp7b {
  
    u16 i : 7;		//Intermediate
    u16 b : 1;		//Additional opcode bit
    u16 op : 8;		//Opcode
    
};

struct RegOp8b {
  
    u16 i : 8;		//Intermediate
    u16 Rd : 3;		//Destination register
    u16 op : 5;		//Opcode
    
};

struct RegOp11b {
    
    u16 i : 11;		//Intermediate
    u16 op : 5;		//Opcode
    
};

struct RegOp12b {
    
    u16 i : 8;		//Intermediate
    u16 cond : 4;	//Execute condition
    u16 op : 4;		//Opcode
    
};
```

