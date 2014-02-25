#ifndef MMU_H
#define MMU_H

#include <stdint.h>

typedef struct _MMU {
    char *bios;
    uint8_t *rom;
                                // 0000 - 3FFF 16KB ROM Bank 00
                                // 4000 - 7FFF 16KB ROM Bank 01..NN
    uint8_t vram[8192];         // 8000 - 9FFF 8KB Video RAM (VRAM)
    uint8_t eram[8192];         // A000 - BFFF 8KB External RAM
    uint8_t wram[8192];         // C000 - DFFF 8KB Work RAM (2 banks 4KB)
    uint8_t echo[7680];         // E000 - FDFF Same as C000 - DDFF
    uint8_t OAM[160];           // FE00 - FE9F Sprite Attribute Table (OAM)
    uint8_t NA[96];             // FEA0 - FEFF Not Usable
    uint8_t io[128];            // FF00 - FF7F I/O ports
    uint8_t hram[128];          // FF80 - FFFE High RAM
    uint8_t ie;                 // Interrupt Enable Register

    char name[17];
    int inbios;
} MMU;


int init_mmu(MMU *mmu, const char *filename);
int load_rom(MMU *mmu, const char *filename);
int reset_mmu(MMU *mmu);
int unload_rom(MMU *mmu);
uint8_t read_byte(MMU *mmu, uint16_t address);
uint16_t read_word(MMU *mmu, uint16_t address);
uint8_t write_byte(MMU *mmu, uint16_t address, uint8_t byte);
uint16_t write_word(MMU *mmu, uint16_t address, uint16_t word);

#define RES(bit, byte)\
        byte &= ~(0x1 << bit)

#endif // MMU_H
