#include "debug.h"

#define D16(h,l) ((h << 8) | l)

/* Flags */
#define set_Z(x) z80->f = ((z80->f & (0xFF - Z_FLAG)) | ((x)<<7))
#define set_N(x) z80->f = ((z80->f & (0xFF - N_FLAG)) | ((x)<<6))
#define set_H(x) z80->f = ((z80->f & (0xFF - H_FLAG)) | ((x)<<5))
#define set_C(x) z80->f = ((z80->f & (0xFF - C_FLAG)) | ((x)<<4))

// TODO: Verify H flag
#define INC(reg) \
        reg++; \
        reg &= 0xFF; \
        set_Z(!reg); \
        set_N(0); \
        set_H(reg == 0x10); \
        z80->t = 4;

// TODO: Verify H flag
#define DEC(reg) \
        reg--; \
        reg &= 0xFF; \
        set_Z(!reg); \
        set_N(1); \
        set_H(reg == 0xF); \
        z80->t = 4;

// TODO: Verify H and C flags
#define ADD(reg, n) \
        reg += n; \
        reg &= 0xFF; \
        set_Z(!reg); \
        set_N(0); \
        set_H(reg & 0x10); \
        set_C(reg & 0x100); \
        z80->t = 4;


#define INC16(h, l) \
    l++; \
    l &= 0xFF; \
    if(!l) { \
        h++; \
        h &= 0xFF; \
    }

#define DEC16(h, l) \
    l--; \
    l &= 0xFF; \
    if(!l) { \
        h--; \
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

case 0x01: // LD BC,nn
    z80->c = read_byte(mmu, z80->pc++);
    z80->b = read_byte(mmu, z80->pc++);
    z80->t = 12;
    print_debug("LD BC, $%X%X\n", z80->c, z80->b);
    break;

case 0x04: // INC B
    INC(z80->b);
    print_debug("INC B (%X)\n", z80->b);
    break;

case 0x05: // DEC B
    DEC(z80->b);
    print_debug("DEC B (%X)\n", z80->b);
    break;

case 0x06: // LD B,n
    z80->b = read_byte(mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD B, $%X\n", z80->b);
    break;

case 0x07: // RLCA
    // C - Contains old bit 7 data.
    set_C(z80->a & Z_FLAG);
    // Rotate a
    n = z80->a;
    z80->a = (z80->a << 1) | (z80->a >> 7);
    set_Z(!z80->a);
    set_N(0);
    set_H(0);
    z80->t = 4;
    print_debug("RLCA ($%X -> $%X)\n", n, z80->a);
    break;

case 0x0C: // INC C
    INC(z80->c);
    print_debug("INC C (%X)\n", z80->c);
    break;

case 0x0D: // DEC C
    DEC(z80->c);
    print_debug("DEC C (%X)\n", z80->c);
    break;

case 0x0E: // LD C,n
    z80->c = read_byte(mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD C, $%X\n", z80->c);
    break;

case 0x11: // LD DE,nn
    z80->e = read_byte(mmu, z80->pc++);
    z80->d = read_byte(mmu, z80->pc++);
    z80->t = 12;
    print_debug("LD DE, $%X%X\n", z80->e, z80->d);
    break;

case 0x12: // LD DE,A
    z80->e = read_byte(mmu, z80->a);
    z80->d = read_byte(mmu, z80->a+1);
    /* TODO: Potentially more optimized as:
        op_aux = read_word(mmu, z80->a);
        z80->e = op_aux & 0xFF;
        z80->d = (op_aux >> 8) & 0xFF;
    */
    z80->t = 8;
    print_debug("LD DE, A [DE: 0x%.2x%.2x]\n", z80->d, z80->e);
    break;

case 0x13: // INC DE
    w = (z80->d << 8) + z80->e; // DEBUG
    INC16(z80->d, z80->e);
    z80->t = 8;
    print_debug("INC DE [0x%.4x -> 0x%.2x%.2x]\n",
                w, z80->d, z80->e);
    break;

case 0x14: // INC D
    INC(z80->d);
    print_debug("INC D (%X)\n", z80->d);
    break;

case 0x15: // DEC D
    DEC(z80->d);
    print_debug("DEC D (%X)\n", z80->d);
    break;

case 0x18: // JR n
    n = read_byte(mmu, z80->pc++);
    z80->pc += n;
    z80->t = 12;
    print_debug("JR 0x%.2x\n", n);
    break;

case 0x1C: // INC E
    INC(z80->e);
    print_debug("INC E (%X)\n", z80->e);
    break;

case 0x1D: // DEC E
    DEC(z80->e);
    print_debug("DEC E (%X)\n", z80->e);
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
    print_debug("JR NZ (%X), 0x%.2x\n", !(z80->f & Z_FLAG), n);
    break;

case 0x21: // LD HL,nn
    z80->l = read_byte(mmu, z80->pc++);
    z80->h = read_byte(mmu, z80->pc++);
    z80->t = 12;
    print_debug("LD HL, 0x%.2x%.2x\n", z80->h, z80->l);
    break;

case 0x22: // LDI HL, A
    op_aux = (z80->h << 8) + z80->l;
    write_byte(mmu, op_aux, z80->a);
    INC16(z80->h, z80->l);
    w = (z80->h << 8) + z80->l; // DEBUG
    z80->t = 8;
    print_debug("LDI HL, A"
                "[HL: 0x%.4x -> 0x%.4x]"
                "[A: 0x%.2x]\n", op_aux, w, z80->a);
    break;

case 0x24: // INC H
    INC(z80->h);
    print_debug("INC H (%X)\n", z80->h);
    break;

case 0x25: // DEC H
    DEC(z80->h);
    print_debug("DEC H (%X)\n", z80->h);
    break;

case 0x2A: // LDI A, HL
    n = z80->a; // DEBUG
    op_aux = (z80->h << 8) + z80->l;
    z80->a = read_byte(mmu, op_aux);
    INC16(z80->h, z80->l);
    w = (z80->h << 8) + z80->l; // DEBUG
    z80->t = 8;
    print_debug("LDI A, HL"
                "[HL: 0x%.4x -> 0x%.4x]"
                "[A: 0x%.2x -> 0x%.2x]\n", op_aux, w, n, z80->a);
    break;

case 0x2C: // INC L
    INC(z80->l);
    print_debug("INC L (%X)\n", z80->l);
    break;

case 0x2D: // DEC L
    DEC(z80->l);
    print_debug("DEC L (%X)\n", z80->l);
    break;

case 0x31: // LD SP,nn
    z80->sp = read_word(mmu, z80->pc);
    z80->pc += 2;
    z80->t = 12;
    print_debug("LD SP, $%hx\n", z80->sp);
    break;

case 0x3C: // INC A
    INC(z80->a);
    print_debug("INC A (%X)\n", z80->a);
    break;

case 0x3D: // DEC A
    DEC(z80->a);
    print_debug("DEC A (%X)\n", z80->a);
    break;

case 0x3E: // LD A,n
    z80->a = read_byte(mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD A, 0x%.2x\n", z80->a);
    break;

case 0x32: // LD (HL-),A
    print_debug("H: %X\n", z80->h);
    print_debug("L: %X\n", z80->l);
    print_debug("HL: %X\n", D16(z80->h, z80->l));

    write_byte(mmu, D16(z80->h, z80->l), z80->a);
    DEC16(z80->h, z80->l);
    z80->t = 8;
    print_debug("LD (HL-), A ($%X)\n", z80->a);
    break;

case 0x47: // LD B,A
    z80->b = z80->a;
    z80->t = 4;
    print_debug("LD B, A ($%X)\n", z80->b);
    break;

case 0x57: // LD D,A
    z80->d = z80->a;
    z80->t = 4;
    print_debug("LD D, A ($%X)\n", z80->d);
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
    debug_step = true;
    break;

case 0x7A: // LD A,D
    z80->a = z80->d;
    z80->t = 4;
    print_debug("LD A, D ($%X)\n", z80->a);
    break;

case 0x80: // ADD A,B
    ADD(z80->a, z80->b);
    break;

case 0x81: // ADD A,C
    ADD(z80->a, z80->c);
    break;

case 0x82: // ADD A,D
    ADD(z80->a, z80->d);
    break;

case 0x83: // ADD A,E
    ADD(z80->a, z80->e);
    break;

case 0x84: // ADD A,H
    ADD(z80->a, z80->h);
    break;

case 0x85: // ADD A,L
    ADD(z80->a, z80->l);
    break;

case 0x87: // ADD A,A
    ADD(z80->a, z80->a);
    break;

case 0xAF: // XOR A
    n = z80->a; // DEBUG
    z80->a ^= z80->a;
    z80->a &= 255;
    set_Z(!z80->a);
    z80->t = 4;
    print_debug("XOR A ($%X -> $%X)\n", n, z80->a);
    break;

case 0xC3: // JP nn
    // No need to increment z80->pc since jump
    op_aux = read_word(mmu, z80->pc);
    z80->pc = op_aux;
    z80->t = 16;
    print_debug("JP %X\n", op_aux);
    break;

case 0xC6: // ADD A,n
    n = read_byte(mmu, z80->pc++);
    ADD(z80->a, n);
    break;

case 0xC9: // RET
    // Get address of previous instruction from stack
    op_aux = z80->pc; // DEBUG
    z80->pc = read_word(mmu, z80->sp);
    z80->sp += 2;
    z80->t = 8;
    print_debug("RET (PC: %X -> %X)\n", z80->sp, op_aux);
    break;

case 0xCB: // CB op codes
    n = read_byte(mmu, z80->pc++);
    switch (n & 0xFF) {
        case 0xBF: // RES 7,a
            RES(7, z80->a);
            print_debug("RES 7, a ($%X)\n", z80->a);
            break;

        default:
            print_debug("Undefined CB OP_CODE: %X\n", op_code);
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
    print_debug("CALL $%X\n", op_aux);
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
    print_debug("RET NC (%X)\n", !(z80->f & C_FLAG));
    break;

// Put A into memory address $FF00+n.
case 0xE0: // LDH (n), A
    op_aux = 0xFF00 + read_byte(mmu, z80->pc++);
    write_byte(mmu, op_aux, z80->a);
    z80->t = 12;
    print_debug("LDH $%X, A ($%X)\n", op_aux, z80->a);
    break;

case 0xE6: // AND n
    n = read_byte(mmu, z80->pc++);
    z80->a &= n;
    set_Z(!z80->a);
    set_N(0);
    set_H(1);
    set_C(0);
    z80->t = 8;
    print_debug("AND 0x%.2x [A: 0x%.2x]\n", n, z80->a);
    break;

// Put A into memory address nn
case 0xEA: // LD (nn), A
    op_aux = read_word(mmu, z80->pc++);
    z80->pc++;
    write_byte(mmu, op_aux, z80->a);
    z80->t = 16;
    print_debug("LD (nn), A "
                "[nn: $%.4x] "
                "[A %.2x]\n", op_aux, z80->a);
    break;

// Put memory address $FF00+n into A.
case 0xF0: // LDH A, (n)
    op_aux = 0xFF00 + read_byte(mmu, z80->pc++);
    z80->a = read_byte(mmu, op_aux);
    z80->t = 12;
    print_debug("LDH A, 0x%.4x (0x%.2x)\n", op_aux, z80->a);
    break;

case 0xF1: // POP AF
    z80->a = read_byte(mmu, z80->sp++);
    z80->f = read_byte(mmu, z80->sp++);
    print_debug("POP AF\n");
    z80->t = 12;
    break;

case 0xF3: // DI
    z80->ime = 0;
    z80->t = 4;
    print_debug("DI\n");
    break;

case 0xF5: // PUSH AF
    z80->sp--;
    write_byte(mmu, z80->sp, z80->a);
    z80->sp--;
    write_byte(mmu, z80->sp, z80->f);
    print_debug("PUSH AF\n");
    z80->t = 16;
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

    set_Z(!n);  // Z Flag set if result is zero [A = n]
    set_N(1);  // N Flag always set
    set_C(n < 0);  // C Flag set for no borrow [A < n]

    // H Flag set if no borrow from bit 4.
    // TODO: Verify this comparison
    if ((z80->a ^ (n & 0xFF) ^ op_aux) & 0x10) {
        set_H(1);
    }
    print_debug("CP, $%X (A: $%X) (F: $%X)\n", op_aux, z80->a, z80->f);
    z80->t = 4;
    break;
