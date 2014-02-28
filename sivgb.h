#ifndef SIVGB_H
#define SIVGB_H

#include "z80.h"
#include "mmu.h"
#include "lcd.h"

int main(int argc, char **argv);
int start(const char *filename);
int run(Z80 *z80, MMU *mmu, LCD *lcd);

#endif // SIVGB_H
