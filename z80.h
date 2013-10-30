#ifndef Z80_H
#define Z80_H

#include <stdint.h>
#include "mmu.h"

/* Flag values
 *
 * 0x80: Zero, set if last operation procued a result 0
 * 0x40, Operation, set if last operation was a substraction
 * 0x20, Half-carry, Set if, in the result of the last operation, the lower
 *       half of the byte overflowed past 15
 * 0x10, Carry, Set if the last operation produced a result over 255 (for
 *       additions) or under 0 (for subtractions).
 */
#define Z_FLAG 0x80
#define N_FLAG 0x40
#define H_FLAG 0x20
#define C_FLAG 0x10

typedef struct _Z80 {
    MMU *mmu;
    uint16_t pc;    // Program counter
    uint16_t sp;    // Stack pointer

    uint8_t a;      // Register A
    uint8_t b;      // Register B
    uint8_t c;      // Register C
    uint8_t d;      // Register D
    uint8_t e;      // Register E
    uint8_t h;      // Register H
    uint8_t l;      // Register L

    uint8_t f;      // Flags register, see Flag values

    uint8_t ime;    // Interupt master enabled [set via set_ime]
    uint8_t m;      // Machine cycle counter
    uint8_t t;      // Clock cycle counter

    uint8_t halt;   // HALT switch
    uint8_t stop;   // STOP switch
} Z80;

int reset_z80(Z80 *z80, MMU *mmu);
int execute(Z80 *z80);

#endif // Z80_H
