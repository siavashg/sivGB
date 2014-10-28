#include <stdio.h>
#include "lcd.h"
#include <math.h>

int reset_lcd(LCD *lcd) {
    lcd->clock = 0;
    lcd->mode = 0;
    lcd->line = 0;
    // TODO: Possibly reset coloring
    return 0;
}

int update_lcd_status(Z80 *z80, MMU *mmu, LCD *lcd) {
    // Update LCD STAT
    uint8_t lcd_stat = read_byte(mmu, STAT);
    lcd_stat = (lcd_stat & 0xFC) | lcd->mode;
    write_byte(mmu, STAT, lcd_stat);
    // Update LCDC Y-coordinate
    write_byte(mmu, LY, lcd->line);

    return 0;
}

int scanline(Z80 *z80, MMU *mmu, LCD *lcd) {
    // TODO: Build rendering!
    uint8_t lcdc = read_byte(mmu, LCDC);

    if (!(lcdc & LCDC_LCD_ON)) {
        print_debug("LCD OFF\n");
        return 0;
    }

    if (lcdc & LCDC_BG_ON) {
        int i;
        uint16_t mem = 0x9000;
        uint8_t bg;
        for (i = 0; i < 1024; i++) {
            int j = 0;
            bg = read_byte(mmu, mem + i);
            for (j = 0; j < 8; j++) {
                if ((bg >> j) & 0x1) {
                    printf("#");
                }
            }
        }
    }

    if (lcdc & LCDC_OBJ_ON) {
        print_debug("DRAW OBJECTS\n");
    }
    return 0;
}
