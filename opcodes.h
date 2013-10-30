#include "debug.h"

#define D16(h,l) ((h << 8) | l)

case 0x00: // NOP
case 0x40: // LD B,B
case 0x49: // LD C,C
case 0x52: // LD D,D
case 0x5B: // LD E,E
case 0x64: // LD H,H
case 0x6D: // LD L,L
case 0x7F: // LD A, A
    print_debug("NOP\n");
    break;

case 0x06: // LD B,n
    z80->b = read_byte(z80->mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD B, $%x\n", z80->b);
    break;

case 0x07: // RLCA
    z80->f = (z80->a & 0x80) >> 3;
    // Rotate a
    z80->a = (z80->a << 1) | (z80->a >> 7);
    z80->t = 4;
    print_debug("RLCA\n");
    break;

case 0x0E: // LD C,n
    z80->c = read_byte(z80->mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD C, $%x\n", z80->c);
    break;

case 0x01: // LD BC,nn
    z80->c = read_byte(z80->mmu, z80->pc++);
    z80->b = read_byte(z80->mmu, z80->pc++);
    z80->t = 12;
    print_debug("LD BC, $%x%x\n", z80->c, z80->b);
    break;

case 0x11: // LD DE,nn
    z80->e = read_byte(z80->mmu, z80->pc++);
    z80->d = read_byte(z80->mmu, z80->pc++);
    z80->t = 12;
    print_debug("LD DE, $%x%x\n", z80->e, z80->d);
    break;

case 0x18: // JR n
    n = read_byte(z80->mmu, z80->pc++);
    z80->pc += n;
    z80->t = 12;
    print_debug("JR n, $%x (%i)\n", n, n);
    break;

case 0x21: // LD HL,nn
    z80->l = read_byte(z80->mmu, z80->pc++);
    z80->h = read_byte(z80->mmu, z80->pc++);
    z80->t = 12;
    print_debug("LD HL, $%x%x\n", z80->h, z80->l);
    break;

case 0x31: // LD SP,nn
    z80->sp = read_word(z80->mmu, z80->pc);
    z80->pc += 2;
    z80->t = 12;
    print_debug("LD SP, $%hx\n", z80->sp);
    break;

case 0x3E: // LD A,n
    z80->a = read_byte(z80->mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD A, $%x\n", z80->a);
    break;


/*
case 0x32: // LD (HL-),A
    print_debug("H: %x\n", z80->h);
    print_debug("L: %x\n", z80->l);
    print_debug("HL: %x\n", D16(z80->h,z80->l));

    //writeMem(HL, A,gbcpu.mem);
    //HL--;
    break;
*/

case 0x47: // LD B,A
    z80->b = z80->a;
    z80->t = 4;
    print_debug("LD B,A\n");
    break;

case 0x57: // LD D,A
    z80->d = z80->a;
    z80->t = 4;
    print_debug("LD D,A\n");
    break;

case 0x76: // HALT
    z80->halt = 1;
    z80->t = 4;

    // The instruction immediately following the HALT instruction is "skipped"
    // when interrupts are disabeld (DI)
    // Details at page 20 in http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
    if (z80->ime) {
        z80->pc++;
    }
    print_debug("HALT\n");
    break;

case 0xAF: // XOR A
    z80->a ^= z80->a;
    z80->a &= 255;
    z80->f = z80->a ? 0 : 0x80;
    z80->t = 4;
    print_debug("XOR A\n");
    break;

case 0xC3: // JP nn
    op_aux = read_word(z80->mmu, z80->pc);
    z80->pc = op_aux;
    z80->t = 16;
    print_debug("JP %x\n", op_aux);
    break;

case 0xCD: // CALL nn
    op_aux = read_word(z80->mmu, z80->pc);
    z80->pc += 2;

    // Push address of next instruction onto stack
    z80->sp -= 2;
    write_word(z80->mmu, z80->sp, z80->pc);

    // Jump to nn (op_aux)
    z80->pc = op_aux;
    z80->t = 16;
    print_debug("CALL %x\n", op_aux);
    break;


case 0xD0: // RET NC
    // Check carry flag
    if (z80->f & C_FLAG) {
        z80->pc = read_word(z80->mmu, z80->sp);
        z80->sp += 2;
        z80->t = 8;
        print_debug("RET NC\n");
    }else {
        z80->pc += 2;
    }
    break;

// Put A into memory address $FF00+n.
case 0xE0: // LDH (a8), A
    write_byte(z80->mmu, 0xFF00+read_byte(z80->mmu, z80->pc++), z80->a);
    print_debug("LDH $%x,A\n", z80->a);
    z80->t = 12;
    break;

// Put memory address $FF00+n into A.
case 0xF0: // LDH (a8), A
    z80->a = 0xFF00+read_byte(z80->mmu, z80->pc++);
    print_debug("LDH A,$%x\n", z80->a);
    z80->t = 12;
    break;

case 0xF3: // DI
    print_debug("DI\n");
    z80->ime = 0;
    z80->t = 4;
    break;

case 0xFB: // EI
    print_debug("EI\n");
    z80->ime = 0;
    z80->t = 4;
    break;

case 0xFE: // CP, n
    n = read_byte(z80->mmu, z80->pc++);

    CPn: function() { 
        var i=Z80._r.a;
        var m=MMU.rb(Z80._r.pc);
        i-=m;
        Z80._r.pc++;
        Z80._r.f=(i<0)?0x50:0x40;
        i&=255;
        if(!i)
            Z80._r.f|=0x80;
        if((Z80._r.a^i^m)&0x10)
            Z80._r.f|=0x20; Z80._r.m=2;
        },

    print_debug("EI\n");
    z80->ime = 0;
    z80->t = 4;
    break;
