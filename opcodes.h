#include "debug.h"

#define D16(h,l) ((h << 8) | l)

#define INC(reg) \
        reg++; \
        reg &= 0xFF; \
        z80->f = (z80->f & C_FLAG) | reg ? 0 : Z_FLAG; \
        z80->t = 4;

#define DEC(reg) \
        reg--; \
        reg &= 0xFF; \
        z80->f = (z80->f & C_FLAG) | reg ? 0 : Z_FLAG; \
        z80->t = 4;

#define INC16(h, l) \
    l++; \
    l &= 0xFF; \
    if(!l) { \
        h++; \
        h &= 0xFF; \
    }

case 0x00: // NOP
case 0x40: // LD B,B
case 0x49: // LD C,C
case 0x52: // LD D,D
case 0x5B: // LD E,E
case 0x64: // LD H,H
case 0x6D: // LD L,L
case 0x7F: // LD A, A
    z80->t = 4;
    print_debug("NOP\n");
    break;

case 0x04: // INC B
    INC(z80->b);
    print_debug("INC B (%x)\n", z80->b);
    break;

case 0x05: // DEC B
    DEC(z80->b);
    print_debug("DEC B (%x)\n", z80->b);
    break;

case 0x06: // LD B,n
    z80->b = read_byte(mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD B, $%x\n", z80->b);
    break;

case 0x07: // RLCA
    z80->f = (z80->a & Z_FLAG) >> 3;
    // Rotate a
    n = z80->a;
    z80->a = (z80->a << 1) | (z80->a >> 7);
    z80->t = 4;
    print_debug("RLCA ($%x -> $%x)\n", n, z80->a);
    break;

case 0x0E: // LD C,n
    z80->c = read_byte(mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD C, $%x\n", z80->c);
    break;

case 0x01: // LD BC,nn
    z80->c = read_byte(mmu, z80->pc++);
    z80->b = read_byte(mmu, z80->pc++);
    z80->t = 12;
    print_debug("LD BC, $%x%x\n", z80->c, z80->b);
    break;

case 0x11: // LD DE,nn
    z80->e = read_byte(mmu, z80->pc++);
    z80->d = read_byte(mmu, z80->pc++);
    z80->t = 12;
    print_debug("LD DE, $%x%x\n", z80->e, z80->d);
    break;

case 0x18: // JR n
    n = read_byte(mmu, z80->pc++);
    z80->pc += n;
    z80->t = 12;
    print_debug("JR $%x\n", n);
    break;

case 0x20: // JR NZ, n
    // Jump to n if Z flag is reset
    n = read_byte(mmu, z80->pc++);
    if (!(z80->f & Z_FLAG)) {
        z80->pc += n;
        z80->t = 12;
    } else {
        z80->t = 8;
    }
    print_debug("JR NZ(%x), $%x\n", (z80->f & Z_FLAG), n);
    break;

case 0x21: // LD HL,nn
    z80->l = read_byte(mmu, z80->pc++);
    z80->h = read_byte(mmu, z80->pc++);
    z80->t = 12;
    print_debug("LD HL, $%x%x\n", z80->h, z80->l);
    break;

case 0x31: // LD SP,nn
    z80->sp = read_word(mmu, z80->pc);
    z80->pc += 2;
    z80->t = 12;
    print_debug("LD SP, $%hx\n", z80->sp);
    break;

case 0x3C: // INC A
    INC(z80->a);
    print_debug("INC A (%x)\n", z80->a);
    break;

case 0x3D: // DEC A
    DEC(z80->a);
    print_debug("DEC A (%x)\n", z80->a);
    break;

case 0x3E: // LD A,n
    z80->a = read_byte(mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD A, 0x%.2x\n", z80->a);
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
    print_debug("LD B, A ($%x)\n", z80->b);
    break;

case 0x57: // LD D,A
    z80->d = z80->a;
    z80->t = 4;
    print_debug("LD D, A ($%x)\n", z80->d);
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
    n = z80->a; // DEBUG
    z80->a ^= z80->a;
    z80->a &= 255;
    z80->f = z80->a ? 0 : 0x80;
    z80->t = 4;
    print_debug("XOR A ($%x -> $%x)\n", n, z80->a);
    break;

case 0xC3: // JP nn
    // No need to increment z80->pc since jump
    op_aux = read_word(mmu, z80->pc);
    z80->pc = op_aux;
    z80->t = 16;
    print_debug("JP %x\n", op_aux);
    break;

case 0xC9: // RET
    // Get address of previous instruction from stack
    op_aux = z80->pc; // DEBUG
    z80->pc = read_word(mmu, z80->sp);
    z80->sp += 2;
    z80->t = 8;
    print_debug("RET (PC: %x -> %x)\n", z80->sp, op_aux, z80->pc);
    break;

case 0xCB: // CB op codes
    n = read_byte(mmu, z80->pc++);
    switch (n & 0xFF) {
        case 0xBF: // RES 7,a
            RES(7, z80->a);
            print_debug("RES 7, a ($%x)\n", z80->a);
            break;

        default:
            print_debug("Undefined CB OP_CODE: %x\n", op_code);
            return 1;
    }
    break;

case 0xCD: // CALL nn
    op_aux = read_word(mmu, z80->pc);
    z80->pc += 2;

    // Push address of next instruction onto stack
    z80->sp -= 2;
    write_word(mmu, z80->sp, z80->pc);

    // Jump to nn (op_aux)
    z80->pc = op_aux;
    z80->t = 16;
    print_debug("CALL $%x\n", op_aux);
    break;

case 0xD0: // RET NC
    // Return if carry flag is reset
    if (!(z80->f & C_FLAG)) {
        z80->pc = read_word(mmu, z80->sp);
        z80->sp += 2;
        z80->t = 20;
    } else {
        z80->t = 8;
    }
    print_debug("RET NC (%x)\n", !(z80->f & C_FLAG));
    break;

// Put A into memory address $FF00+n.
case 0xE0: // LDH (n), A
    op_aux = 0xFF00 + read_byte(mmu, z80->pc++);
    write_byte(mmu, op_aux, z80->a);
    print_debug("LDH $%x, A ($%x)\n", op_aux, z80->a);
    z80->t = 12;
    break;

// Put memory address $FF00+n into A.
case 0xF0: // LDH A, (n)
    op_aux = 0xFF00 + read_byte(mmu, z80->pc++);
    z80->a = read_byte(mmu, op_aux);
    print_debug("LDH A, %x ($%x)\n", z80->a, op_aux);
    z80->t = 12;
    break;

case 0xF3: // DI
    print_debug("DI\n");
    z80->ime = 0;
    z80->t = 4;
    break;

case 0xFB: // EI
    /*
     * This intruction enables interrupts but not immediately.
     * Interrupts are enabled after instruction after EI is
     * executed.
     * TODO: Does this really enable it after next instruction?
     */
    print_debug("EI\n");
    z80->ime = 0;
    z80->t = 4;
    break;

case 0xFE: // CP, n
    /*
     * Compare A with n.
     */
    op_aux = read_byte(mmu, z80->pc++);
    n = z80->a - op_aux;

    // N Flag always set
    z80->f = N_FLAG;

    // C Flag set for no borrow [A < n]
    if (n < 0) {
        z80->f |= C_FLAG;
    }

    // Z Flag set if result is zero [A = n]
    if (n == 0) {
        z80->f |= Z_FLAG;
    }

    // H Flag set if no borrow from bit 4.
    // TODO: Verify this comparison
    if ((z80->a ^ (n & 0xFF) ^ op_aux) & 0x10) {
        z80->f |= H_FLAG;
    }
    print_debug("CP, $%x (A: $%x) (F: $%x)\n", op_aux, z80->a, z80->f);
    z80->t = 4;
    break;
