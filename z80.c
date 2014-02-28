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
