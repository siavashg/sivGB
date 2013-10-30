#include <stdio.h>
#include <stdlib.h>
#include "z80.h"
#include "mmu.h"
#include "debug.h"

int reset_z80(Z80 *z80, MMU *mmu) {
    z80->mmu = mmu;

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

int execute(Z80 *z80) {

    while(z80->pc < 0x1110) {
        uint16_t address = z80->pc++;
        uint8_t op_code = read_byte(z80->mmu, address);
        uint16_t op_aux;
        int8_t n;   // Signed temp byte

        print_debug("PC: %x\t", address);
        switch(op_code) {
            #include "opcodes.h"
            default:
                print_debug("Undefined OP_CODE: %x\n", op_code);
                return 1;
        }
    }
    return 0;
}
