#pragma once

//Defines are as follows:

//Rd/Rs/i/Op = Type;
//s = source, d = destination, n = intermediate
//i = intermediate, Op = opcode, ni = intermediate or register (depends on opcode)

//size_offset in bits
//_lshift is for when a left shift is applied to the data after it is obtained

//Rd3_0 = Low destination 3-bit register at bit 0 offset with no lshift
//i5_6_2 = 5-bit intermediate at bit 6 with 2 lshift
//s12 = 12-bit two's complement (located in r.ir)
//s23 = 23-bit two's complement offset (located in r.ir and r.nir)

#define Rd3_0 (r.ir & 7)
#define Rs3_3 ((r.ir >> 3) & 7)
#define Rni3_6 ((r.ir >> 6) & 7)
#define i5_6 ((r.ir >> 6) & 0x1F)
#define i5_6_1 ((r.ir >> 5) & 0x3E)
#define i5_6_2 ((r.ir >> 4) & 0x7C)
#define Op5_11 (r.ir >> 11)
#define Op7_9 (r.ir >> 9)
#define Op9_7 (r.ir >> 7)
#define Op10_6 (r.ir >> 6)
#define i7_0 (r.ir & 0x7F)
#define i8_0 (r.ir & 0xFF)
#define i8_0_1 ((r.ir & 0xFF) << 1)
#define i8_0_2 ((r.ir & 0xFF) << 2)
#define i7_0_2 ((r.ir & 0x7F) << 2)
#define s12 (((r.ir << 1) & 0xFFE) | (r.ir & 0x400 * (0xFFFFF000 / 0x400)))
#define s23 ((((r.ir << 1) & 0xFFE) | ((r.nir << 12) & 0x7FF000)) | (r.nir & 0x400 * (0xFF800000 / 0x400)))
#define Rd3_8 ((r.ir >> 8) & 7)
#define Op8_8 (r.ir >> 8)