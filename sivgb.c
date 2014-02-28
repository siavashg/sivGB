#include <stdio.h>
#include <stdlib.h>
#include "sivgb.h"
#include "z80.h"
#include "mmu.h"
#include "lcd.h"
#include "debug.h"


int main(int argc, char **argv) {
    const char *rom_filename;

    if (argc < 2) {
        printf("Usage: %s <romfile>\n", argv[0]);
        exit(1);
    }
    rom_filename = argv[1];
    start(rom_filename);
    return 0;
}

int start(const char *filename) {
    int ret;
    Z80 z80;
    MMU mmu;
    LCD lcd;

    reset_z80(&z80);
    reset_mmu(&mmu);
    reset_lcd(&lcd);

    print_debug("Loading rom: %s\n", filename);
    load_rom(&mmu, filename);
    print_debug("Starting emulation of: %s\n", filename);

    ret = run(&z80, &mmu, &lcd);

    unload_rom(&mmu);
    print_debug("Emulation ended\n");
    return ret;
}

int run(Z80 *z80, MMU *mmu, LCD *lcd) {

    while(z80->pc < 0x1110) {
        int lcd_mode;
        uint16_t address = z80->pc++;
        uint8_t op_code = read_byte(mmu, address);
        uint16_t op_aux;
        int8_t n;   // Signed temp byte

        print_debug("PC: %x,\t"
                    "OP: %x\t"
                    "LCD: %x\t",
                    address, op_code, lcd->clock);

        // Z80
        switch(op_code) {
            #include "opcodes.h"
            default:
                print_debug("Undefined OP_CODE: %x\n", op_code);
                return 1;
        }
        lcd->clock += z80->t;

        // LCD
        switch (lcd->mode) {
            // H-Blank
            case 0:
                if (lcd->clock >= 204) {
                    lcd->clock = 0;
                    lcd->line++;

                    if (lcd->line == 143) {
                        lcd->mode = 1;
                        // TODO: Render image
                    } else {
                        lcd->mode = 2;
                    }
                }
                break;

            // V-Blank
            case 1:
                if (lcd->clock >= 456) {
                    lcd->clock = 0;
                    lcd->line++;

                    if (lcd->line > 153) {
                        lcd->mode = 2;
                        lcd->line = 0;
                    }

                }
                break;

            // OAM read
            case 2:
                if (lcd->clock >= 80) {
                    lcd->clock = 0;
                    lcd->mode = 3;
                }
                break;

            // VRAM read, scanline
            case 3:
                if (lcd->clock >= 172) {
                    lcd->clock = 0;
                    lcd->mode = 0;
                }
                break;
        }

        #define STAT 0xFF41
        write_byte(mmu, STAT, (read_byte(mmu, STAT) & 0xFF) | lcd->mode);

        #define LY 0xFF44
        write_byte(mmu, LY, lcd->line);
    }

    return 0;
}
