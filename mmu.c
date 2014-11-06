#include <stdio.h>
#include <stdlib.h>
#include "mmu.h"
#include "bios.h"
#include "debug.h"

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

int unload_rom(MMU *mmu) {
    print_debug("Unloading rom\n");

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
    // ROM bank 0 BIOS
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
    // Cartridge ROM bank 0
    case 0x1000:
    case 0x2000:
    case 0x3000:
    case 0x4000:
    case 0x5000:
    case 0x6000:
    case 0x7000:
        return mmu->rom[address];

    // VRAM
    case 0x8000:
    case 0x9000:
        return mmu->vram[address&0x1FFF];

    // ERAM
    case 0xA000:
    case 0xB000:
        return mmu->eram[address&0x1FFF];

    // WRAM
    case 0xC000:
    case 0xD000:
    case 0xE000: // ECHO
        return mmu->wram[address&0x1FFF];

    // ECHO and REST
    case 0xF000:
        switch(address&0x0F00) {
        // Remaining ECHO
        case 0x000: case 0x100: case 0x200: case 0x300:
        case 0x400: case 0x500: case 0x600: case 0x700:
        case 0x800: case 0x900: case 0xA00: case 0xB00:
        case 0xC00: case 0xD00:
            return mmu->echo[address&0x1FFF];

        // OAM
        case 0xE00:
            return mmu->OAM[address&0x00FF];

        // IO and High RAM
        case 0xF00:
            if ((address & 0x00FF) <= 0x7F) {
                // FF00 - FF7F I/O ports
                return mmu->io[address&0x00FF];
            } else {
                // FF80 - FFFE High RAM
                return mmu->hram[address&0x00FF - 0x80];
            }
            break;
        }

    default:
        print_debug("Memory: $%X. Out of bounds...\n", address);
        return 0;
    }

    return 0;
}

uint8_t write_byte(MMU *mmu, uint16_t address, uint8_t byte) {
    switch(address&0xF000) {

    // Cartridge ROM banks
    case 0x0000: // ROM bank 0
    case 0x1000: // ROM bank 0
    case 0x2000: // ROM bank 0
    case 0x3000: // ROM bank 0
    case 0x4000: // Switchable ROM bank
    case 0x5000: // Switchable ROM bank
    case 0x6000: // Switchable ROM bank
    case 0x7000: // Switchable ROM bank
        mmu->rom[address] = byte;
        return 0;

    // VRAM
    case 0x8000:
    case 0x9000:
        mmu->vram[address&0x1FFF] = byte;
        return 0;

    // ERAM
    case 0xA000:
    case 0xB000:
        mmu->eram[address&0x1FFF] = byte;
        return 0;

    // WRAM
    case 0xC000:
    case 0xD000:
    case 0xE000: // ECHO
        mmu->wram[address&0x1FFF] = byte;
        return 0;

    // ECHO and REST
    case 0xF000:

        switch(address&0x0F00) {
        // Remaining ECHO
        case 0x000: case 0x100: case 0x200: case 0x300:
        case 0x400: case 0x500: case 0x600: case 0x700:
        case 0x800: case 0x900: case 0xA00: case 0xB00:
        case 0xC00: case 0xD00:
            mmu->echo[address&0x1FFF] = byte;
            return 0;

        // OAM
        case 0xE00:
            mmu->OAM[address&0x00FF] = byte;
            return 0;

        case 0xF00:
            if ((address & 0x00FF) <= 0x7F) {
                // FF00 - FF7F I/O ports
                mmu->io[address&0x00FF] = byte;
            } else {
                // FF80 - FFFE High RAM
                mmu->hram[address&0x00FF - 0x80] = byte;
            }
            return 0;
        }

    default:
        print_debug("Unknown memory, $%X\n", address);
        //print_debug("Unknown memory, $%X\n", address & 0xF000);
        exit(1);
    }

    return 0;
}

uint16_t write_word(MMU *mmu, uint16_t address, uint16_t word) {
    write_byte(mmu, address + 1, word >> 8);
    write_byte(mmu, address, word & 0xFF);
    return 0;
}
