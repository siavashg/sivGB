#ifndef LCD_H
#define LCD_H

#include <stdint.h>

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

#endif // LCD_H
