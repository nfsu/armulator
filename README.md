# Armchitecture (ARM7/9)

ARM7/ARM9 are 16/32-bit RISC architectures where every opcode is 1 unit. In ARM-mode this would be 4 bytes, while in Thumb-mode this would be 2 bytes.

ARM7TDMI & ARM9TDMI contain 38 32-bit **registers** which can be accessed through virtual registers, which change physical register depending on the current active mode. This also contains the PSRs (Program Status Registers); the **CPSR** (Current PSR) and the **SPSR** (Saved PSR). The SPSR is used to save the CPSR when a software interrupt is triggered. This allows you to go back to function the interrupt was called from.

The accessible registers are the r0-r15 registers. r15 is the **PC** (program counter) which points to the current instruction. r14 is the **LR** (link register) which is used to hold a backup to the previous PC when a jump with link is performed. r13 is the **SP** (stack pointer) which points to the top (or bottom) of the stack.

The stack is Full Descending and virtual memory access depends on the current system.

## Thumb mode

The T in ARM7/9TDMI stands for Thumb; a 16-bit mode. This is a simplified instruction set though it still allows for doing almost the same things, albeit in more instructions. Regular instructions only allows access to r0-r7, though other registers can be manipulated by using the r0-r7 registers as an intermediate and by specialized instructions.

The entire documentation on the instruction set can be found [here](docs/thumb.md).

## Arm mode

If T is disabled, the program is running in Arm mode; 32-bit mode.

The entire documentation on the instruction set can be found [here](docs/arm.md).

## PSR

The PSR is like a regular 32-bit register, except it has bitflags that express how the next instructions will be executed. It contains the condition flags (what instructions will be executed and what branches will be), the thumb flag (if it is using thumb mode or arm mode) and other important flags such at the current mode (the banks it is using). The PSRs are not accessible through a mapped register, though some instructions can save and reset the PSR.

```cpp
struct PSR {
  
    //Control bits
    
    Mode mode : 5;			//The execution mode
    bool thumb : 1;			//T; if thumb mode is used
    bool disableFiq : 1;		//F: if FIQ is disabled
    bool disableIrq : 1;		//I: if IRQ is disabled
    
	//Unused for ARM7/9TDMI (20 bits)
    //Padding 2 bytes + 4 bits

	u8 p0[2];
	u8 p1 : 4;
    
    //Condition flags
    
    bool overflow : 1;			//V: the last operation caused a signed overflow
    bool carry : 1;			//C: the last operation caused an unsigned overflow
    bool zero : 1;			//Z: the last operation was zero
    bool negative : 1;			//N: the last operation was negative
    
};
```

## Condition flags

```cpp
enum Condition : u8 /* 4-bit */ {
  
    EQ,		//Equals					(zero)
    NE,		//Not Equals 					(!zero)
    
    CS,		//Carry Set; 		unsigned >= 		(carry)
    CC,		//Carry Clear;		unsigned <		(!carry)
    
    MI,		//MInus;			< 0		(negative)
    PL,		//PLus;				>= 0		(!negative)
    
    VS,		//oVerflow Set					(overflow)
    VC,		//oVerflow Clear				(!overflow)
    
    HI,		//Higher than		unsigned >		(carry && !zero)
    LS,		//Less or equal		unsigned <=		(!carry || zero)
    
    GE,		//Greater or equal	>=			(negative == overflow)
    LT,		//Less				<		(negative != overflow)
    
    GT,		//Greater			>		(!zero && negative == overflow)
    LE,		//Less or equal		<=			(zero || negative != overflow)
    
    AL,		//Always
    NV		//Never (software interrupt)
    
};
```

The conditions are checked from the CPSR to determine if it should execute an operation.

A compare and other operations can set the condition flags, changing the control flow of your program. A compare is the same as subtracting the right variable from the left variable. This will set the respective bits.

```assembly
# r0 = 0, r1 = 1
MOV r0, 0
MOV r1, 1

# Checks positive for: EQ ==, CC, PL >=, VC, LS <=, GE >=, LE <=, AL
CMP r0, r0 		# = 0 - 0 = 0

# Checks positive for: NE !=, CC, MI <, VC, LS <=, LT <, LE <=, AL
CMP r0, r1 		# = 0 - 1 = u32_MAX (-1)

# Checks positive for: NE !=, CC, PL >=, VC, HI >, GE >=, GT >, AL
CMP r1, r0 		# = 1 - 0 = 1
```

These conditions can then be used as a suffix for every operation:

```assembly
# Always branch to myLabel
BAL myLabel

# Goto myLabel if the last operation was zero (x == y)
BEQ myLabel
```

The flags aren't set by all operations; some operations only set negative & zero flags, while alu operations (regularly) set the negative, zero, carry & overflow flags. Other operations such as barrel shifts generally don't set the overflow flag.

These flags are calculated the following way; (a = left, b = right, c = result)

```cpp
//Zero
c == 0;

//Negative
c & i32_MIN;

//TODO: Carry

//Overflow
(a & i32_MIN) == (b & i32_MIN) && (c & i32_MIN) != (a & i32_MIN)
```



## Modes

Some accessible registers map to different registers depending on the mode set in the PSR. These modes are as follows:

```cpp
enum Mode : u8 /* 5-bit */ {
    USR = 0x10,
    FIQ = 0x11,
    IRQ = 0x12,
    SVC = 0x13,
    ABT = 0x17,
    UND = 0x1B,
    SYS = 0x1F
};
```

There are 7 modes available; USR (16), FIQ (17), IRQ (18), SVC (19), ABT (23), UND (27), SYS (31). Other modes are illegal and will cause the processor to enter an unrecoverable state.

The modes before **1 0000** are legacy modes and aren't used in ARM7/9 processors. ARM6 supports these 26-bit modes to allow for ARM2/ARM3 compatibility. 

All non-user modes are *privileged modes* allowing them full access to system resources and allowing them to change mode freely. 

To check for validity in a PSR mode, the following function can be used

```cpp
bool isValid(Mode m){
    return (m & USR) && ((m & 3) == 3 || m < SVC);
}
```

## Banked registers

r0-r15 and the SPSR are mapped to physical registers. These registers depend on the current mode. The following illustrates how registers are shared with modes:



![3](D:/programming/repos/nfse/docs/res/register_organization_in_ARM_state.svg)

**Figure 1: Register organization in ARM state** [[3](http://infocenter.arm.com/help/topic/com.arm.doc.ddi0229c/graphics/register_organization_in_ARM_state.svg)]



The SPSR, SP and LR are always affected and for FIQ the r8-r12 registers are as well. The r8_fiq is a different physical register than the regular r8. The mapping is as follows:

```cpp
struct RegisterData {
    
	u32 sysUsr[15];
	u32 pc;
	u32 fiq[7];
	u32 irq[2];
	u32 svc[2];
	u32 abt[2];
	u32 und[2];
	PSR cpsr;
	PSR spsr[6];
};

//Lookup table for mode id from mode enum
static constexpr u8 modeToId[] = {
    6,6,6,6, 6,6,6,6, 6,6,6,6, 6,6,6,6,			//Legacy ARM6 modes
    0,1,2,3, 6,6,6,4, 6,6,6,5, 6,6,6,0			//USR, FIQ, IRQ, ABT, UND, SYS
};

//Lookup table for the registers using their mode id
static constexpr u8 registerMapping[][16] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 },			//SYS and USR
	{ 0, 1, 2, 3, 4, 5, 6, 7, 16, 17, 18, 19, 20, 21, 22, 15 },			//FIQ
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 23, 24, 15 },			//IRQ
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 25, 26, 15 },			//SVC
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 27, 28, 15 },			//ABT
	{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 29, 30, 15 }			//UND
};

//registerMapping[modeToId[FIQ]][9] = r9_fiq
```

## Sources

[Arm infocenter](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0210c/CIHIAAGE.html) ARM7TDMI Technical Reference Manual control bits [0]

[Keil](https://www.keil.com/pack/doc/cmsis/Core_A/html/group__CMSIS__CPSR.html) Current Program Status Register (CPSR) [1]

[Arm infocenter](http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0229c/BGBJCJAE.html) The ARM state register set [2]

[Arm infocenter](http://infocenter.arm.com/help/topic/com.arm.doc.ddi0229c/graphics/register_organization_in_ARM_state.svg) Register organization in ARM state [3]
