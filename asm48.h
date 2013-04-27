/*
 * Assembler for the Intel 8048 microcontroller family.
 * Copyright (c) 2002, 2003 David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * Enhanced in 2012, 2013 by JustBurn and sy2002 of MEGA
 * http://www.adventurevision.net
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ASM48_H
#define ASM48_H

#define VERSION "0.4.1"

#define GEN_POOL_SIZE (4096 * 1024)	/* Size of general object pool. */
#define ASM_POOL_SIZE (512 * 1024)	/* Size of assembled code pool. */

#define GENERAL_REG_MASK 0x7	/* Mask for general purpose register. */
#define DEREF_REG_MASK 0x1	/* Mask for dereference-capable register (R0/R1). */
#define PORT_MASK 0x3		/* Mask for port number. */

#define PAGE_MASK (~(0xFF))	/* Mask for 256 byte "page" in instruction memory. */
#define MAX_ADDR (1<<12)	/* Maximum address for call and jmp instructions. */

/*
 * Memory pool object, for quick allocation.
 */
struct Pool {
	int size, start;
	void *buf;
};

/*
 * Expression node.
 */
struct Expr {
	int op;
	struct Expr *left, *right;
	const char *sym;
	int value;
	int line_num;
	int cur_offset;
	int mustexist;
};

struct Vtable;

/*
 * Instruction - represents either a single assembly instruction,
 * or other directive that expands into data to be assembled.
 */
struct Instruction {
	struct Vtable *vtable;
	int offset, size;
	int src_line;
	unsigned char *buf;
	struct Expr *expr;
	char *cur_file;
	struct Instruction *next;
};

/*
 * Virtual methods for Instruction objects.
 */
struct Vtable {
	void (*assemble)(struct Instruction *ins);
};

/*
 * Symbol table entry.
 */
struct Symbol {
	const char *name;
	int value;
	int type;
	struct Symbol *next;
};

/* Function prototypes. */

/* err.c */
void err_printf(const char *fmt, ...);
void warn_printf(const char *fmt, ...);

/* pool.c */
struct Pool *create_pool(int size);
void *pool_alloc_buf(struct Pool *pool, int size);

/* instruction.c */
struct Instruction *allocate_instruction(int offset, int size);
struct Instruction *ins1(int code);
struct Instruction *ins2(int byte1, int byte2);
struct Instruction *reg_ins(int opcode, int regnum);
struct Instruction *deref_ins(int opcode, int regnum);
struct Instruction *imm_ins(int opcode, struct Expr *imm_val);
struct Instruction *j8_ins(int opcode, struct Expr *addr);
struct Instruction *jmp_ins(int opcode, struct Expr *addr);
struct Instruction *port_ins(int opcode, int portnum);
struct Instruction *port_imm_ins(int opcode, int portnum, struct Expr *imm_val);
struct Instruction *jump_reg_ins(int opcode, int regnum, struct Expr *addr);
struct Instruction *jb_ins(int bit_num, struct Expr *addr);
struct Instruction *reg_imm_ins(int opcode, int regnum, struct Expr *imm_val);
struct Instruction *deref_imm_ins(int opcode, int regnum, struct Expr *imm_val);
struct Instruction *org(int address, int line_num);
struct Instruction *orgfill(int address, int value, int line_num);
struct Instruction *db(int value, int line_num);
struct Instruction *dbr(int value, int line_num);
struct Instruction *db_expr(struct Expr *expr_val, int line_num);
struct Instruction *dw_expr(struct Expr *expr_val, int line_num);
struct Instruction *incbin(char *filename, int line_num);
void append(struct Instruction *ins);
void append_nostat(struct Instruction *ins);

/* expr.c */
struct Expr *mk_const_expr(int ival, int line_num);
struct Expr *mk_symbolic_expr(const char *sym, int line_num, int mustexist);
struct Expr *mk_unary_expr(int op, struct Expr *subexpr, int line_num);
struct Expr *mk_binary_expr(int op, struct Expr *left, struct Expr *right, int line_num);
int eval_expr(char *cur_file, struct Expr *expr);

/* symtab.c */
const char *dup_str(const char *str);
void define_symbol(const char *name, int value, int type);
void redefine_symbol(const char *name, int value, int type);
struct Symbol *lookup_symbol(const char *name);
void export_symbols(const char *filename);

/* ihex.c */
void load_file(char *filename);
void save_file(char *command);

/* Global variables */
extern struct Instruction *ins_head, *ins_tail;
extern struct Pool *gen_pool;
extern struct Pool *asm_pool;
extern int cur_offset;
extern char *cur_file;

/* asm48.c */
void cur_file_set(const char *filename);

#define	BANK_USAGE_MAX	256
extern int bank_usage[BANK_USAGE_MAX];

#define SYMB_CONST	0
#define SYMB_LABEL	1

#endif // ASM48_H
