#include <stdio.h>
#include <stdlib.h>
#include "mmu.h"
#include "bios.h"
#include "debug.h"

int init_mmu(MMU *mmu, const char *filename) {
    if(load_rom(mmu, filename) != 0) {
        return 1;
    }
    if(reset_mmu(mmu) != 0) {
        return 1;
    }
    return 0;
}

int load_rom(MMU *mmu, const char *filename) {
    FILE *fp;
    unsigned int filesize;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("Cannot read ROM file...\n");
        return 1;
    }

    fseek(fp, 0L, SEEK_END);
    filesize = ftell(fp);  //ignore EOF
    fseek(fp, 0L, SEEK_SET);

    mmu->rom = (uint8_t*) calloc(filesize, sizeof(uint8_t));
    fread(mmu->rom, 1, filesize, fp);
    print_debug("Loaded %d bytes ROM into MMU\n", filesize);
    fclose(fp);
    return 0;
}

int reset_mmu(MMU *mmu) {
    mmu->inbios = 0; // Jumping over BIOS for now
    mmu->bios = _bios;
    mmu->io[0x05] = 0x00;   // TIMA
    mmu->io[0x06] = 0x00;   // TMA
    mmu->io[0x07] = 0x00;   // TAC
    //mmu->io[0x0F] = 0x01; // IF
    mmu->io[0x10] = 0x80;   // NR10
    mmu->io[0x11] = 0xBF;   // NR11
    mmu->io[0x12] = 0xF3;   // NR12
    mmu->io[0x14] = 0xBF;   // NR14
    mmu->io[0x16] = 0x3F;   // NR21
    mmu->io[0x17] = 0x00;   // NR22
    mmu->io[0x19] = 0xBF;   // NR24
    mmu->io[0x1A] = 0x7F;   // NR30
    mmu->io[0x1B] = 0xFF;   // NR31
    mmu->io[0x1C] = 0x9F;   // NR32
    mmu->io[0x1E] = 0xBF;   // NR33
    mmu->io[0x20] = 0xFF;   // NR41
    mmu->io[0x21] = 0x00;   // NR42
    mmu->io[0x22] = 0x00;   // NR43
    mmu->io[0x23] = 0xBF;   // NR30
    mmu->io[0x24] = 0x77;   // NR50
    mmu->io[0x25] = 0xF3;   // NR51
    mmu->io[0x26] = 0xF1;   // -GB, $F0-SGB ; NR52
    mmu->io[0x40] = 0x91;   // LCDC
    mmu->io[0x41] = 0x84;   // STAT
    mmu->io[0x42] = 0x00;   // SCY
    mmu->io[0x43] = 0x00;   // SCX
    mmu->io[0x44] = 0x00;   // LY
    mmu->io[0x45] = 0x00;   // LYC
    mmu->io[0x47] = 0xFC;   // BGP
    mmu->io[0x48] = 0xFF;   // OBP0
    mmu->io[0x49] = 0xFF;   // OBP1
    mmu->io[0x4A] = 0x00;   // WY
    mmu->io[0x4B]= 0;       // WX
    mmu->ie=0;              // IE
    return 0;
}

int unload_rom(MMU *mmu) {
    print_debug("Unloading rom");

    if (mmu->rom)
        free(mmu->rom);
    mmu->rom = NULL;
    return 0;
}

uint16_t read_word(MMU *mmu, uint16_t address) {
    return (read_byte(mmu, address +1) << 8 | read_byte(mmu, address));
}

uint8_t read_byte(MMU *mmu, uint16_t address) {

    switch(address&0xF000) {
    case 0x0000:
        if (mmu->inbios) {
            if (address<0x0100) {
                return mmu->bios[address];
            }
            else if(address == 0x0100) {
              mmu->inbios = 0;
              print_debug("Leaving BIOS.\n");
            }
        } else {
            return mmu->rom[address];
        }
        break;
    case 0x1000:
    case 0x2000:
    case 0x3000:
        return mmu->rom[address];
        break;
    default:
        print_debug("Out of bounds...\n");
        return 0;
        break;
    }

    return 0;
}

uint8_t write_byte(MMU *mmu, uint16_t address, uint8_t byte) {
    switch(address&0xF000) {

    // Cartridge ROM banks
    case 0x0000:
    case 0x1000:
    case 0x2000:
    case 0x3000:
    case 0x4000:
    case 0x5000:
    case 0x6000:
    case 0x7000:
        print_debug("Cartridge ROM bank not implemented");
        break;

    // VRAM
    case 0x8000:
    case 0x9000:
        mmu->vram[address&0x1FFF] = byte;
        // TODO: Update screen
        break;

    // ERAM
    case 0xA000:
    case 0xB000:
        print_debug("ERAM not implemented");
        break;

    // WRAM
    case 0xC000:
    case 0xD000:
    case 0xE000: // ECHO
        print_debug("ERAM not implemented");
        break;

    // ECHO and REST
    case 0xF000:

        switch(address&0x0F00) {
        // Remaining ECHO
        case 0x000: case 0x100: case 0x200: case 0x300:
        case 0x400: case 0x500: case 0x600: case 0x700:
        case 0x800: case 0x900: case 0xA00: case 0xB00:
        case 0xC00: case 0xD00:
            print_debug("ERAM not implemented");
            break;
        }

        // OAM
        case 0xE00:
            break;

    default:
        print_debug("Unknown memory?");
        break;

    //uint8_t io[128];            // FF00 - FF7F I/O ports
    //uint8_t hram[128];          // FF80 - FFFE High RAM
    }

    return 0;
}

uint16_t write_word(MMU *mmu, uint16_t address, uint16_t word) {
    write_byte(mmu, address + 1, word >> 8);
    write_byte(mmu, address, word & 0xFF);
    return 0;
}
