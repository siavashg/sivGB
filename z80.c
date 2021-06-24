#include <stdio.h>
#include <stdlib.h>
#include "z80.h"
#include "mmu.h"
#include "lcd.h"
#include "debug.h"

int reset_z80(Z80 *z80) {
    // Reset cycle counters
    z80->m = 0;
    z80->t = 0;

    z80->ime = 0;

    z80->pc = 0x0100; // Jump over BIOS check
    z80->sp = 0xFFFE;

    // Reset registers
    z80->a = 0x01;
    z80->f = 0xB0;
    z80->b = 0x00;
    z80->c = 0x13;
    z80->d = 0x00;
    z80->e = 0xD8;
    z80->h = 0x01;
    z80->l = 0x4D;

    z80->halt = 0;
    z80->stop = 0;

    print_debug("Z80 reset");
    return 0;
}

void debug_dump(Z80 *z80) {
#ifdef DEBUG_VERBOSE
    if (DEBUG_VERBOSE) {
        print_debug(
            "\nPC: 0x%.4X\t"
            "SP: 0x%.4X\t\n"

            "A: 0x%.2X\t"
            "B: 0x%.2X\t"
            "C: 0x%.2X\t"
            "D: 0x%.2X\t\n"

            "E: 0x%.2X\t"
            "F: 0x%.2X\t"
            "H: 0x%.2X\t"
            "L: 0x%.2X\t\n"

            "AF: %.2X%.2X\n"
            "BC: %.2X%.2X\n"
            "DE: %.2X%.2X\n"
            "HL: %.2X%.2X\n"

            "IME: 0x%.2X\t"
            "M: 0x%.2X\t"
            "T: 0x%.2X\t"

            "HALT: 0x%.2X\t"
            "STOP: 0x%.2X\n",
            z80->pc, z80->sp,
            z80->a, z80->b, z80->c, z80->d,
            z80->e, z80->f, z80->h, z80->l,
            z80->a, z80->f,
            z80->b, z80->c,
            z80->d, z80->e,
            z80->h, z80->l,
            z80->ime, z80->m, z80->t,
            z80->halt, z80->stop);
    }
#endif
}
