#pragma once

#define Cond4_28 (r.ir >> 28)
#define Op4_24 ((r.ir >> 24) & 0xF)
#define Op7_21 ((r.ir >> 21) & 0x7F)
#define S1_20 (r.ir & 0x100000)
#define Rn4_16 ((r.ir >> 16) & 0xF)
#define _Rn4_16 r.registers[m[Rn4_16]]
#define Rd4_12 ((r.ir >> 12) & 0xF)
#define _Rd4_12 r.registers[m[Rd4_12]]
#define Rs4_8 ((r.ir >> 8) & 0xF)
#define _Rs4_8 r.registers[m[Rs4_8]]
#define Rm4_0 (r.ir & 0xF)
#define _Rm4_0 r.registers[m[Rm4_0]]
#define i8_0 (r.ir & 0xFF)

//#define Op2 immediate ? arm::ror<false, false>(r.cpsr, i8_0, Is) : 