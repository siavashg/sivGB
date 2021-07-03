#include "debug.h"

#define D16(h,l) ((h << 8) | l)

/* Flags */
#define CLEAR_FLAGS() z80->f = 0
#define set_Z(x) z80->f = ((z80->f & (0xFF - Z_FLAG)) | ((x)<<7))
#define set_N(x) z80->f = ((z80->f & (0xFF - N_FLAG)) | ((x)<<6))
#define set_H(x) z80->f = ((z80->f & (0xFF - H_FLAG)) | ((x)<<5))
#define set_C(x) z80->f = ((z80->f & (0xFF - C_FLAG)) | ((x)<<4))
#define get_Z !!(z80->f & Z_FLAG)
#define get_N !!(z80->f & N_FLAG)
#define get_H !!(z80->f & H_FLAG)
#define get_C !!(z80->f & C_FLAG)

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

#define INC(reg) \
        set_H((reg & 0x0F) == 0x0F); \
        reg++; \
        reg &= 0xFF; \
        set_Z(!reg); \
        set_N(0); \
        z80->t = 4;

#define DEC(reg) \
        set_H(!(reg & 0x0F)); \
        reg--; \
        reg &= 0xFF; \
        set_Z(!reg); \
        set_N(1); \
        z80->t = 4;

#define SUB(reg) \
        set_N(1); \
        set_H((reg & 0x0F) > (z80->a & 0x0F)); \
        set_C(reg > z80->a); \
        z80->a = z80->a - reg; \
        set_Z(!z80->a); \
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

#define ADDHL(a, b) \
        op_aux = D16(z80->h, z80->l) + D16(a, b) & 0xFFFF; \
        z80->h = op_aux >> 8; \
        z80->l = op_aux & 0xFF; \
        set_N(0); \
        set_H((D16(z80->h, z80->l) & 0x0F) + (D16(a, b) & 0x0F) > 0x0F); \
        set_C(op_aux > 0xFFFF); \
        z80->t = 8;

#define XOR(r) \
    r ^= r; \
    r &= 0xFF; \
    CLEAR_FLAGS(); \
    set_Z(!r); \
    z80->t = 4; \

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
    if(l == 0xFF) { \
        h--; \
        h &= 0xFF; \
    }


#define RES(bit, byte)\
        byte &= ~(0x1 << bit); \
        z80->t = 8;

#define SWAP(byte)\
        byte = (byte >> 4 | byte << 4); \
        set_Z(!byte); \
        set_N(0); \
        set_H(0); \
        set_C(0); \
        z80->t = 8;

#define SRL(byte)\
        set_C(byte & 0x01); \
        byte = byte >> 1; \
        set_Z(byte == 0); \
        set_N(0); \
        set_H(0); \
        z80->t = 8;

#define RR(byte)\
        n = byte; \
        byte = (byte >> 1); \
        if (get_C) byte |= 0x80; \
        set_Z(byte == 0); \
        set_N(0); \
        set_H(0); \
        set_C(byte & 0x01); \
        z80->t = 8;


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
    print_debug("LD BC, $%X%X\n", z80->b, z80->c);
    break;

case 0x03: // INC BC
    w = (z80->b << 8) + z80->c; // DEBUG
    INC16(z80->b, z80->c);
    z80->t = 8;
    print_debug("INC BC [0x%.4X -> 0x%.2X%.2X]\n",
        w, z80->b, z80->c);
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

case 0x0B: // DEC BC
    w = D16(z80->b, z80->c); // DEBUG
    DEC16(z80->b, z80->c);
    z80->t = 8;
    print_debug("DEC BC [0x%.4X -> 0x%.4X]\n",
        w, D16(z80->b, z80->c));
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
    print_debug("LD DE, $%.2X%.2X\n", z80->d, z80->e);
    break;

case 0x12: // LD (DE), A
    op_aux = D16(z80->d, z80->e);
    write_byte(mmu, op_aux, z80->a);
    z80->t = 8;
    print_debug("LD (DE), A [DE: 0x%.2X%.2X, A: 0x%.2X]\n", z80->d, z80->e,
        z80->a);
    break;

case 0x13: // INC DE
    w = (z80->d << 8) + z80->e; // DEBUG
    INC16(z80->d, z80->e);
    z80->t = 8;
    print_debug("INC DE [0x%.4X -> 0x%.2X%.2X]\n",
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

case 0x16: // LD D,n
    z80->d = read_byte(mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD D, 0x%.2X\n", z80->d);
    break;

case 0x18: // JR n
    n = read_byte(mmu, z80->pc++);
    z80->pc += n;
    z80->t = 12;
    print_debug("JR 0x%.2X\n", n);
    break;

case 0x19: // ADD HL,DE
    ADDHL(z80->d, z80->e);
    print_debug("ADD HL, DE 0x%.4X\n", D16(z80->h, z80->l));
    break;

case 0x1A: // LD A, (DE)
    op_aux = D16(z80->d, z80->e);
    z80->a = read_byte(mmu, op_aux);
    z80->t = 8;
    print_debug("LD A, (DE) [A: $%.2X, DE %.4X]\n",
        z80->a, op_aux);
    break;

case 0x1C: // INC E
    INC(z80->e);
    print_debug("INC E (%X)\n", z80->e);
    break;

case 0x1D: // DEC E
    DEC(z80->e);
    print_debug("DEC E (%X)\n", z80->e);
    break;

case 0x1E: // LD E, n
    z80->e = read_byte(mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD E, 0x%.2X\n", z80->e);
    break;

case 0x1F: // RRA
    n = z80->a & 0xFF;
    z80->a = z80->a >> 1 | (get_C << 7);
    set_Z(0);
    set_N(0);
    set_H(0);
    set_C(n & 0x01);
    print_debug("RRA [A: 0x%.2X -> 0x%.2X]\n", n & 0xFF, z80->a);
    break;

case 0x20: // JR NZ, n
    // Jump to n if Z flag is reset
    n = read_byte(mmu, z80->pc++);
    if (!get_Z) {
        z80->pc += n;
        z80->t = 12;
    }
    else {
        z80->t = 8;
    }
    print_debug("JR NZ [n: %d]\n", n);
    break;

case 0x21: // LD HL,nn
    z80->l = read_byte(mmu, z80->pc++);
    z80->h = read_byte(mmu, z80->pc++);
    z80->t = 12;
    print_debug("LD HL, 0x%.2X%.2X\n", z80->h, z80->l);
    break;

case 0x22: // LDI HL, A
    op_aux = D16(z80->h, z80->l);
    write_byte(mmu, op_aux, z80->a);
    INC16(z80->h, z80->l);
    w = D16(z80->h, z80->l); // DEBUG
    z80->t = 8;
    print_debug("LDI HL, A [HL: 0x%.4X -> 0x%.4X, A: 0x%.2X]\n",
        op_aux, w, z80->a);
    break;

case 0x23: // INC HL
    w = (z80->h << 8) + z80->l; // DEBUG
    INC16(z80->h, z80->l);
    z80->t = 8;
    print_debug("INC HL [0x%.4X -> 0x%.2X%.2X]\n",
        w, z80->h, z80->l);
    break;

case 0x24: // INC H
    INC(z80->h);
    print_debug("INC H (%X)\n", z80->h);
    break;

case 0x25: // DEC H
    DEC(z80->h);
    print_debug("DEC H (%X)\n", z80->h);
    break;

case 0x26: // LD H,n
    z80->h = read_byte(mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD H, 0x%.2X\n", z80->h);
    break;

case 0x29: // ADD HL, HL
    op_aux = D16(z80->h, z80->l) + D16(z80->h, z80->l);
    set_N(0);
    set_H(op_aux & 0x1000);
    set_C(op_aux & 0xFFFF0000);
    z80->h = (op_aux & 0x0000FF00) >> 8;
    z80->l = (op_aux & 0x000000FF);
    print_debug("ADD HL, HL [HL: 0x%.4X]\n", D16(z80->h, z80->l));
    break;

case 0x2A: // LDI A, HL
    n = z80->a; // DEBUG
    op_aux = D16(z80->h, z80->l);
    z80->a = read_byte(mmu, op_aux);
    INC16(z80->h, z80->l);
    w = D16(z80->h, z80->l); // DEBUG
    z80->t = 8;
    print_debug("LDI A, HL"
        "[HL: 0x%.4X -> 0x%.4X]"
        "[A: 0x%.2X -> 0x%.2X]\n", op_aux, w, n, z80->a);
    break;

case 0x2B: // DEC HL
    w = D16(z80->h, z80->l); // DEBUG
    DEC16(z80->h, z80->l);
    z80->t = 8;
    print_debug("DEC HL [0x%.4X -> 0x%.4X]",
        w, D16(z80->h, z80->l));
    break;

case 0x28: // JR Z, n
    // Jump to n if Z flag is set
    n = read_byte(mmu, z80->pc++);
    if (get_Z) {
        z80->pc += n;
        z80->t = 12;
    }
    else {
        z80->t = 8;
    }
    print_debug("JR Z (%X), 0x%.2X\n", (z80->f & Z_FLAG), n);
    break;

case 0x2C: // INC L
    INC(z80->l);
    print_debug("INC L (%X)\n", z80->l);
    break;

case 0x2D: // DEC L
    DEC(z80->l);
    print_debug("DEC L (%X)\n", z80->l);
    break;

case 0x2E: // LD L, n
    z80->l = read_byte(mmu, z80->pc++);
    z80->t = 8;
    print_debug("LD L, 0x%.2X\n", z80->l);
    break;

case 0x2F: // CPL
    w = z80->a; // DEBUG
    z80->a ^= 0xFF;
    set_N(1);
    set_H(1);
    z80->t = 4;
    print_debug("CPL (%X -> %X)\n", w, z80->a);
    break;

case 0x30: // JR NC, n
    // Jump to n if C flag is reset
    n = read_byte(mmu, z80->pc++);
    if (!get_C) {
        z80->pc += n;
        z80->t = 12;
    }
    else {
        z80->t = 8;
    }
    print_debug("JR NC [C: %X, n: %d, pc: %X]\n", get_C, n, z80->pc);
    break;


case 0x31: // LD SP,nn
    z80->sp = read_word(mmu, z80->pc);
    z80->pc += 2;
    z80->t = 12;
    print_debug("LD SP, $%hx\n", z80->sp);
    break;

case 0x32: // LD (HL-),A
    op_aux = D16(z80->h, z80->l);
    write_byte(mmu, D16(z80->h, z80->l), z80->a);
    DEC16(z80->h, z80->l);
    z80->t = 8;
    print_debug(
        "LD (HL-)(0x%.4X -> 0x%.4X), A ($%X)\n",
        op_aux, D16(z80->h, z80->l), z80->a);
    break;

case 0x35: // DEC (HL)
    op_aux = D16(z80->h, z80->l);
    n = read_byte(mmu, op_aux);
    DEC(n);
    write_byte(mmu, op_aux, n);
    z80->t = 12;
    print_debug("DEC (HL) [HL: %x, (HL): %x]\n", op_aux, n);
    break;

case 0x36: // LD (HL),n
    op_aux = D16(z80->h, z80->l);
    n = read_byte(mmu, z80->pc++);
    write_byte(mmu, op_aux, n);
    z80->t = 12;
    print_debug("LD (HL), n "
        "[HL: 0x%.4X, n: 0x%.2X]\n", op_aux, n);
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
    print_debug("LD A, 0x%.2X\n", z80->a);
    break;

case 0x46: // LD B, (HL)
    op_aux = D16(z80->h, z80->l);
    z80->b = read_byte(mmu, op_aux);
    z80->t = 8;
    print_debug("LD (HL), B ($%X)\n", z80->b);
    break;

case 0x47: // LD B,A
    z80->b = z80->a;
    z80->t = 4;
    print_debug("LD B, A ($%X)\n", z80->b);
    break;

case 0x4E: // LD C, (HL)
    op_aux = D16(z80->h, z80->l);
    z80->c = read_byte(mmu, op_aux);
    z80->t = 8;
    print_debug("LD (HL), B ($%X)\n", z80->c);
    break;

case 0x4F: // LD C,A
    z80->c = z80->a;
    z80->t = 4;
    print_debug("LD C, A ($%X)\n", z80->c);
    break;

case 0x56: // LD D, (HL)
    op_aux = D16(z80->h, z80->l);
    z80->d = read_byte(mmu, op_aux);
    z80->t = 8;
    print_debug("LD (HL), D ($%X)\n", z80->d);
    break;

case 0x57: // LD D,A
    z80->d = z80->a;
    z80->t = 4;
    print_debug("LD D, A ($%X)\n", z80->d);
    break;

case 0x5F: // LD E,A
    z80->e = z80->a;
    z80->t = 4;
    print_debug("LD E, A [E: %x]\n", z80->e);
    break;

case 0x5E: // LD E, (HL)
    op_aux = D16(z80->h, z80->l);
    z80->e = read_byte(mmu, op_aux);
    z80->t = 8;
    print_debug("LD E, (HL) [l: $%X]\n", z80->e);
    break;

case 0x60: // LD H, B
    z80->h = z80->b;
    z80->t = 4;
    print_debug("LD H, B [H: %.2x]\n", z80->h);
    break;

case 0x61: // LD H, C
    z80->h = z80->c;
    z80->t = 4;
    print_debug("LD H, C [H: %.2x]\n", z80->h);
    break;

case 0x62: // LD H, D
    z80->h = z80->d;
    z80->t = 4;
    print_debug("LD H, D [H: %.2x]\n", z80->h);
    break;

case 0x63: // LD H, E
    z80->h = z80->e;
    z80->t = 4;
    print_debug("LD H, E [H: %.2x]\n", z80->h);
    break;

case 0x65: // LD H, L
    z80->h = z80->l;
    z80->t = 4;
    print_debug("LD H, L [H: %.2x]\n", z80->h);
    break;

case 0x66: // LD H, (HL)
    op_aux = D16(z80->h, z80->l);
    z80->h = read_byte(mmu, op_aux);
    z80->t = 8;
    print_debug("LD H, (HL) [H: $%X]\n", z80->h);
    break;

case 0x67: // LD H, A
    z80->h = z80->a;
    z80->t = 4;
    print_debug("LD H, A [H: %.2x]\n", z80->h);
    break;

case 0x68: // LD L, B
    z80->l = z80->b;
    z80->t = 4;
    print_debug("LD L, B [L: %.2x]\n", z80->l);
    break;

case 0x69: // LD L, C
    z80->l = z80->c;
    z80->t = 4;
    print_debug("LD L, C [L: %.2x]\n", z80->l);
    break;

case 0x6A: // LD L, D
    z80->l = z80->d;
    z80->t = 4;
    print_debug("LD L, D [L: %.2x]\n", z80->l);
    break;

case 0x6B: // LD L, E
    z80->l = z80->e;
    z80->t = 4;
    print_debug("LD L, E [L: %.2x]\n", z80->l);
    break;

case 0x6C: // LD L, H
    z80->l = z80->h;
    z80->t = 4;
    print_debug("LD L, H [L: %.2x]\n", z80->l);
    break;

case 0x6E: // LD L, (HL)
    op_aux = D16(z80->h, z80->l);
    z80->l = read_byte(mmu, op_aux);
    z80->t = 8;
    print_debug("LD L, (HL) [L: $%X]\n", z80->l);
    break;

case 0x6F: // LD L, A
    z80->l = z80->a;
    z80->t = 4;
    print_debug("LD L, A [L: %.2x]\n", z80->l);
    break;

case 0x70: // LD (HL), B
    op_aux = D16(z80->h, z80->l);
    write_byte(mmu, op_aux, z80->b);
    z80->t = 8;
    print_debug("LD (HL), B "
        "[HL: 0x%.4X, B: 0x%.2X]\n", op_aux, z80->b);
    break;

case 0x71: // LD (HL), C
    op_aux = D16(z80->h, z80->l);
    write_byte(mmu, op_aux, z80->c);
    z80->t = 8;
    print_debug("LD (HL), C "
        "[HL: 0x%.4X, C: 0x%.2X]\n", op_aux, z80->c);
    break;

case 0x72: // LD (HL), D
    op_aux = D16(z80->h, z80->l);
    write_byte(mmu, op_aux, z80->d);
    z80->t = 8;
    print_debug("LD (HL), D "
        "[HL: 0x%.4X, D: 0x%.2X]\n", op_aux, z80->d);
    break;

case 0x73: // LD (HL), E
    op_aux = D16(z80->h, z80->l);
    write_byte(mmu, op_aux, z80->e);
    z80->t = 8;
    print_debug("LD (HL), E "
        "[HL: 0x%.4X, E: 0x%.2X]\n", op_aux, z80->e);
    break;

case 0x74: // LD (HL), H
    op_aux = D16(z80->h, z80->l);
    write_byte(mmu, op_aux, z80->h);
    z80->t = 8;
    print_debug("LD (HL), H "
        "[HL: 0x%.4X, H: 0x%.2X]\n", op_aux, z80->h);
    break;

case 0x75: // LD (HL), L
    op_aux = D16(z80->h, z80->l);
    write_byte(mmu, op_aux, z80->l);
    z80->t = 8;
    print_debug("LD (HL), L "
        "[HL: 0x%.4X, L: 0x%.2X]\n", op_aux, z80->l);
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
    //debug_step = true;
    break;

case 0x77: // LD (HL), A
    op_aux = D16(z80->h, z80->l);
    write_byte(mmu, op_aux, z80->a);
    z80->t = 8;
    print_debug("LD (HL), A [HL: 0x%.4X, A: 0x%.2X]\n", op_aux, z80->a);
    break;

case 0x78: // LD A,B
    z80->a = z80->b;
    z80->t = 4;
    print_debug("LD A, B ($%X)\n", z80->a);
    break;

case 0x79: // LD A,C
    z80->a = z80->c;
    z80->t = 4;
    print_debug("LD A, C ($%X)\n", z80->a);
    break;

case 0x7A: // LD A,D
    z80->a = z80->d;
    z80->t = 4;
    print_debug("LD A, D ($%X)\n", z80->a);
    break;

case 0x7B: // LD A,E
    z80->a = z80->e;
    z80->t = 4;
    print_debug("LD A, E ($%X)\n", z80->a);
    break;

case 0x7C: // LD A,H
    z80->a = z80->h;
    z80->t = 4;
    print_debug("LD A, H ($%X)\n", z80->a);
    break;

case 0x7D: // LD A,L
    z80->a = z80->l;
    z80->t = 4;
    print_debug("LD A, L ($%X)\n", z80->a);
    break;

case 0x80: // ADD A,B
    ADD(z80->a, z80->b);
    print_debug("ADD A, B ($%X)\n", z80->a);
    break;

case 0x81: // ADD A,C
    ADD(z80->a, z80->c);
    print_debug("ADD A, C ($%X)\n", z80->a);
    break;

case 0x82: // ADD A,D
    ADD(z80->a, z80->d);
    print_debug("ADD A, D ($%X)\n", z80->a);
    break;

case 0x83: // ADD A,E
    ADD(z80->a, z80->e);
    print_debug("ADD A, E ($%X)\n", z80->a);
    break;

case 0x84: // ADD A,H
    ADD(z80->a, z80->h);
    print_debug("ADD A, H ($%X)\n", z80->a);
    break;

case 0x85: // ADD A,L
    ADD(z80->a, z80->l);
    print_debug("ADD A, L ($%X)\n", z80->a);
    break;

case 0x87: // ADD A,A
    ADD(z80->a, z80->a);
    print_debug("ADD A, A ($%X)\n", z80->a);
    break;

case 0x90: // SUB B
    SUB(z80->b);
    print_debug("SUB B [A: %x, B: %x]\n", z80->a, z80->b);
    break;

case 0x91: // SUB C
    SUB(z80->c);
    print_debug("SUB C [A: %x, C: %x]\n", z80->a, z80->c);
    break;

case 0x92: // SUB D
    SUB(z80->d);
    print_debug("SUB D [A: %x, D: %x]\n", z80->a, z80->c);
    break;

case 0x93: // SUB E
    SUB(z80->e);
    print_debug("SUB E [A: %x, E: %x]\n", z80->a, z80->e);
    break;

case 0x94: // SUB H
    SUB(z80->h);
    print_debug("SUB H [A: %x, H: %x]\n", z80->a, z80->h);
    break;

case 0x95: // SUB L
    SUB(z80->l);
    print_debug("SUB L [A: %x, L: %x]\n", z80->a, z80->l);
    break;

case 0x97: // SUB A
    SUB(z80->a);
    print_debug("SUB A [A: %x, A: %x]\n", z80->a, z80->a);
    break;

case 0xA1: // AND C
    n = z80->a; // DEBUG
    z80->a &= z80->c;
    z80->a &= 255;
    set_Z(!z80->a);
    set_N(0);
    set_H(1);
    set_C(0);
    z80->t = 4;
    print_debug("AND C (A: $%X -> $%X)\n", n, z80->a);
    break;

case 0xA9: // XOR C
    n = z80->c; // DEBUG
    XOR(z80->c);
    print_debug("XOR C ($%X -> $%X)\n", n, z80->c);
    break;

case 0xAE: // XOR (HL)
    op_aux = D16(z80->h, z80->l);
    n = read_byte(mmu, op_aux);
    XOR(n);
    z80->t = 8;
    print_debug("XOR (HL) [HL: %X, (HL): %X]\n", op_aux, n);

case 0xAF: // XOR A
    n = z80->a; // DEBUG
    XOR(z80->a);
    print_debug("XOR A ($%X -> $%X)\n", n, z80->a);
    break;

case 0xB0: // OR B
    z80->a = z80->a | z80->b;
    set_Z(!z80->a);
    set_N(0);
    set_H(0);
    set_C(0);
    z80->t = 4;
    print_debug("OR B ($%X: %X)\n", z80->b, z80->a);
    break;

case 0xB1: // OR C
    z80->a = z80->a | z80->c;
    set_Z(!z80->a);
    set_N(0);
    set_H(0);
    set_C(0);
    z80->t = 4;
    print_debug("OR C ($%X: %X)\n", z80->c, z80->a);
    break;

case 0xB2: // OR D
    z80->a = z80->a | z80->d;
    set_Z(!z80->a);
    set_N(0);
    set_H(0);
    set_C(0);
    z80->t = 4;
    print_debug("OR D ($%X: %X)\n", z80->d, z80->a);
    break;

case 0xB3: // OR E
    z80->a = z80->a | z80->e;
    set_Z(!z80->a);
    set_N(0);
    set_H(0);
    set_C(0);
    z80->t = 4;
    print_debug("OR E ($%X: %X)\n", z80->e, z80->a);
    break;

case 0xB6: // OR (HL)
    op_aux = D16(z80->h, z80->l);
    n = read_byte(mmu, op_aux);
    z80->a = z80->a | n;
    set_Z(!z80->a);
    set_N(0);
    set_H(0);
    set_C(0);
    z80->t = 8;
    print_debug("OR (HL) ($%X: %X)\n", op_aux, z80->a);
    break;

case 0xB7: // OR A
    set_Z(!z80->a);
    set_N(0);
    set_H(0);
    set_C(0);
    z80->t = 4;
    print_debug("OR A [A: %X]\n", z80->a);
    break;

case 0xC0: // RET NZ
    // Return if Z flag is reset
    if (!get_Z) {
        z80->pc = read_word(mmu, z80->sp);
        z80->sp += 2;
        z80->t = 20;
    }
    else {
        z80->t = 8;
    }
    print_debug("RET NZ [Z: %X, PC: 0x%.4X]\n", get_Z, z80->pc);
    break;

case 0xC1: // POP BC
    z80->c = read_byte(mmu, z80->sp++);
    z80->b = read_byte(mmu, z80->sp++);
    print_debug("POP BC [BC: 0x%.4X]\n", D16(z80->b, z80->c));
    z80->t = 12;
    break;

case 0xC2: // JP NZ, nn
    // Return if Z flag is reset
    if (!get_Z) {
        z80->pc = read_word(mmu, z80->pc);
        z80->t = 16;
    }
    else {
        z80->t = 12;
    }
    print_debug("JP NZ, nn [Z: %X, PC: 0x%.4X]\n", get_Z, z80->pc);
    break;

case 0xC3: // JP nn
    // No need to increment z80->pc since jump
    op_aux = read_word(mmu, z80->pc);
    z80->pc = op_aux;
    z80->t = 16;
    print_debug("JP 0x%.4X\n", op_aux);
    break;

case 0xC4: // CALL nn
    op_aux = read_word(mmu, z80->pc);
    z80->pc += 2;

    if (!get_Z) {
        // Push address of next instruction onto stack
        z80->sp -= 2;
        write_word(mmu, z80->sp, z80->pc);
        // Jump to nn (op_aux)
        z80->pc = op_aux;
    }

    z80->t = 16;
    print_debug("CALL NZ $%X [Z: %X]\n", op_aux, get_Z);
    break;

case 0xC5: // PUSH BC
    z80->sp--;
    write_byte(mmu, z80->sp, z80->b);
    z80->sp--;
    write_byte(mmu, z80->sp, z80->c);
    print_debug("PUSH BC\n");
    z80->t = 16;
    break;

case 0xC6: // ADD A,n
    n = read_byte(mmu, z80->pc++);
    ADD(z80->a, n);
    break;

case 0xC8: // RET Z
    // Return if Z flag is set
    if (get_Z) {
        z80->pc = read_word(mmu, z80->sp);
        z80->sp += 2;
        z80->t = 20;
    }
    else {
        z80->t = 8;
    }
    print_debug("RET Z [Z: %X, PC: 0x%.4X]\n", get_Z, z80->pc);
    break;

case 0xC9: // RET
    // Get address of previous instruction from stack
    op_aux = z80->pc; // DEBUG
    z80->pc = read_word(mmu, z80->sp);
    z80->sp += 2;
    z80->t = 8;
    print_debug("RET (PC: %X -> %X)\n", op_aux, z80->pc);
    break;

case 0xCB: // CB op codes
    n = read_byte(mmu, z80->pc++);
    switch (n & 0xFF) {
    case 0xBF: // RES 7,a
        RES(7, z80->a);
        print_debug("RES 7, a ($%X)\n", z80->a);
        break;

    case 0x18: // RR B
        RR(z80->b);
        print_debug("RR B [b: %x]\n", z80->b);
        break;

    case 0x19: // RR C
        RR(z80->c);
        print_debug("RR C [c: %x]\n", z80->c);
        break;

    case 0x1A: // RR D
        RR(z80->d);
        print_debug("RR D [d: %x]\n", z80->d);
        break;

    case 0x1B: // RR E
        RR(z80->e);
        print_debug("RR E [e: %x]\n", z80->e);
        break;

    case 0x1C: // RR H
        RR(z80->h);
        print_debug("RR H [h: %x]\n", z80->h);
        break;

    case 0x1D: // RR L
        RR(z80->l);
        print_debug("RR L [l: %x]\n", z80->l);
        break;

    case 0x1F: // RR A
        RR(z80->a);
        print_debug("RR A [l: %x]\n", z80->a);
        break;

    case 0x37: // SWAP A
        SWAP(z80->a);
        print_debug("SWAP A ($%X)\n", z80->a);
        break;

    case 0x38: // SRL B
        SRL(z80->b);
        print_debug("SRL B [B: %X]\n", z80->b);
        break;

    case 0x3F: // SRL A
        SRL(z80->a);
        print_debug("SRL A [A: %X]\n", z80->a);
        break;

    default:
        print_debug("Undefined CB OP_CODE: CB %X\n", n);
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

case 0xCE: // ADC A,n
    n = read_byte(mmu, z80->pc++);
    op_aux = z80->a + n + get_C;
    set_Z((op_aux & 0xFF) == 0);
    set_N(0);
    set_H((z80->a ^ n ^ op_aux) & 0x10 ? 1 : 0);
    set_C((op_aux & 0xFF00) ? 1 : 0);
    z80->a = op_aux & 0xFF;
    print_debug("ADC A,%.2x [A: 0x%.2x]\n", z80->pc, z80->a);
    break;

case 0xD0: // RET NC
    // Return if carry flag is reset
    if (!get_C) {
        z80->pc = read_word(mmu, z80->sp);
        z80->sp += 2;
        z80->t = 20;
    }
    else {
        z80->t = 8;
    }
    print_debug("RET NC (%X)\n", !get_C);
    break;

case 0xD1: // POP DE
    z80->e = read_byte(mmu, z80->sp++);
    z80->d = read_byte(mmu, z80->sp++);
    z80->t = 12;
    print_debug("POP DE [DE: 0x%.4X]\n", D16(z80->d, z80->e));
    break;

case 0xD5: // PUSH DE
    z80->sp--;
    write_byte(mmu, z80->sp, z80->d);
    z80->sp--;
    write_byte(mmu, z80->sp, z80->e);
    z80->t = 16;
    print_debug("PUSH DE\n");
    break;

case 0xD6: // SUB d8
    n = read_byte(mmu, z80->pc++);
    set_N(1); // N Flag always set.
    set_C(n > z80->a); // Set if no borrow [A < n]
    // H Flag set if no borrow from bit 4.
    // TODO: Verify this comparison
    set_H((n & 0x0F) > (z80->a & 0x0F));
    z80->a -= n;
    set_Z(z80->a == 0);
    z80->t = 8;
    print_debug("SUB d8, %X (A: %d)\n", n, z80->a);
    break;

    // Put A into memory address $FF00+n.
case 0xE0: // LDH (n), A
    op_aux = 0xFF00 + read_byte(mmu, z80->pc++);
    write_byte(mmu, op_aux, z80->a);
    z80->t = 12;
    print_debug("LDH $%X, A ($%X)\n", op_aux, z80->a);
    break;

case 0xE1: // POP HL
    z80->l = read_byte(mmu, z80->sp++);
    z80->h = read_byte(mmu, z80->sp++);
    print_debug("POP HL [HL: 0x%.4X]\n", D16(z80->h, z80->l));
    z80->t = 12;
    break;

    // Put A into memory address $FF00 + register C
case 0xE2: // LD (C), A
    op_aux = 0xFF00 + z80->c;
    write_byte(mmu, op_aux, z80->a);
    z80->t = 8;
    print_debug("LD (C), A ($%X)\n", z80->a);
    break;

case 0xE5: // PUSH HL
    z80->sp--;
    write_byte(mmu, z80->sp, z80->h);
    z80->sp--;
    write_byte(mmu, z80->sp, z80->l);
    print_debug("PUSH HL (0x%.4X)\n", D16(z80->h, z80->l));
    z80->t = 16;
    break;

case 0xE6: // AND n
    n = read_byte(mmu, z80->pc++);
    z80->a &= n;
    set_Z(!z80->a);
    set_N(0);
    set_H(1);
    set_C(0);
    z80->t = 8;
    print_debug("AND 0x%.2X [A: 0x%.2X]\n", n, z80->a);
    break;

case 0xE9: // JP (HL)
    // No need to increment z80->pc since jump
    z80->pc = D16(z80->h, z80->l);
    z80->t = 4;
    print_debug("JP (HL) 0x%.4X\n", z80->pc);
    break;

    // Put A into memory address nn
case 0xEA: // LD (nn), A
    op_aux = read_word(mmu, z80->pc);
    z80->pc += 2;
    write_byte(mmu, op_aux, z80->a);
    z80->t = 16;
    print_debug("LD (nn), A [nn: $%.4X, A %.2X]\n", op_aux, z80->a);
    break;

    // TODO: Untested
case 0xEF: // RST 28H
    op_aux = read_word(mmu, z80->pc);
    z80->sp -= 2;
    write_word(mmu, z80->sp, op_aux);
    z80->pc = 0x28;
    z80->t = 16;
    print_debug("RST 28HA\n");
    break;

    // Put memory address $FF00+n into A.
case 0xF0: // LDH A, (n)
    op_aux = 0xFF00 + read_byte(mmu, z80->pc++);
    z80->a = read_byte(mmu, op_aux);
    z80->t = 12;
    print_debug("LDH A, 0x%.4X (0x%.2X)\n", op_aux, z80->a);
    break;

case 0xF1: // POP AF
    z80->f = read_byte(mmu, z80->sp++);
    z80->a = read_byte(mmu, z80->sp++);
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

case 0xF6: // OR n
    n = read_byte(mmu, z80->pc++);
    z80->a |= n;
    set_Z(!z80->a);
    set_N(0);
    set_H(0);
    set_C(0);
    z80->t = 8;
    print_debug("OR 0x%.2X [A: 0x%.2X]\n", n, z80->a);
    break;

case 0xF8: // LD HL, SP+n
    n = read_byte(mmu, z80->pc++);
    op_aux = z80->sp + n;
    set_C(op_aux & 0xFFFF0000);
    set_H((z80->sp & 0x0F) + (n & 0x0F) > 0x0F);
    set_Z(0);
    set_N(0);
    z80->h = op_aux >> 8;
    z80->l = op_aux & 0xFF;
    z80->t = 12;
    print_debug("LD HL, SP+n [HL: 0x%.4X, SP: 0x%.2X, n: 0x%.2X]\n", D16(z80->h, z80->l), z80->sp, n);
    break;

case 0xFA: // LD A, (nn)
    op_aux = read_word(mmu, z80->pc);
    z80->pc += 2;
    z80->a = read_byte(mmu, op_aux);
    z80->t = 16;
    print_debug("LD A, (nn) [A: $%.2X, nn %.4X]\n", z80->a, op_aux);
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
     * This is basically an A - n subtraction (0xD6) instruction but the
     * results are thrown away.
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
