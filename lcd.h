#ifndef LCD_H
#define LCD_H

#include <stdint.h>
#include "z80.h"
#include "mmu.h"
#include "debug.h"

// Memory mappings for common addresses
#define LCDC 0xFF40
#define STAT 0xFF41
#define LY 0xFF44

// LCDC bit map
#define LCDC_BG_ON 0x1
#define LCDC_OBJ_ON 0x2
#define LCDC_OBJ_SIZE 0x4
#define LCDC_BG_MAP 0x8
#define LCDC_BG_TILE 0x10
#define LCDC_WINDOW_DISPLAY 0x20
#define LCDC_WINDOW_MAP 0x40
#define LCDC_LCD_ON 0x80


typedef struct _RGB{
    int r;
    int g;
    int b;
} RGB;

typedef struct _LCD {
    RGB canvas[160][144];
    int clock;
    int mode;
    int line;
} LCD;

int reset_lcd(LCD *lcd);
int update_lcd_status(Z80 *z80, MMU *mmu, LCD *lcd);
int scanline(Z80 *z80, MMU *mmu, LCD *lcd);

#endif // LCD_H
