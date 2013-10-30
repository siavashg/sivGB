#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include "z80.h"
#include "mmu.h"

int main(int argc, char **argv) {
    MMU mmu;
    Z80 z80;
    const char *rom_file;

    if (argc < 2) {
        printf("Usage: %s <romfile>\n", argv[0]);
        exit(1);
    }

    rom_file = argv[1];
    reset_mmu(&mmu);
    reset_z80(&z80, &mmu);
    load_rom(&mmu, rom_file);

    execute(&z80);

    print_debug("MMU: %x\n", *(mmu).rom);

    unload_rom(&mmu);
    return 0;
}
