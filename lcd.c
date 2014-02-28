#include <stdio.h>
#include "lcd.h"

int reset_lcd(LCD *lcd) {
    lcd->clock = 0;
    lcd->mode = 0;
    lcd->line = 0;
    // TODO: Possibly reset coloring
    return 0;
}
