/*
 * Assembler for the Intel 8048 microcontroller family.
 * Copyright (c) 2002,2003 David H. Hovemeyer <daveho@cs.umd.edu>
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "asm48.h"

/***********************************************************************
 * Private functions
 ***********************************************************************/

/*
 * No-op function for vtable slots.
 */
static void noop(struct Instruction *ins)
{
}

/*
 * No-op vtable.
 */
static struct Vtable noop_vtable = {
	&noop,
};

/*
 * Assemble() method for instructions with immedate constants. 
 */
static void assemble_imm_ins(struct Instruction *ins)
{
	int imm_val = eval_expr(ins->expr);
	printf("imm_val = %d\n", imm_val);
	if (imm_val < 0 || imm_val > 255)
		warn_printf("line %d: immediate value exceeds range\n", ins->src_line);
	ins->buf[1] = imm_val;
}

/*
 * Vtable for instructions with immedate constants.
 */
static struct Vtable imm_ins_vtable = {
	&assemble_imm_ins,
};

/*
 * Assemble() function for jmp and call instructions.
 */
static void assemble_jmp(struct Instruction *ins)
{
	int address = eval_expr(ins->expr);
	printf("address = %d\n", address);
	/* FIXME: ensure that address is in range. */
	ins->buf[0] |= ((address >> 3) & 0xE);
	ins->buf[1] = address & 0xFF;
}

/*
 * Vtable for jmp and call instructions.
 */
static struct Vtable jmp_vtable = {
	&assemble_jmp,
};

/*
 * Assemble() function for jump instructions with 8 bit target addresses.
 * The target must be in the same 256-byte "page".
 */
static void assemble_j8(struct Instruction *ins)
{
	int address = eval_expr(ins->expr);
	printf("address = %d\n", address);
	if ((address & PAGE_MASK) != (ins->offset & PAGE_MASK))
		warn_printf("line %d: jump offset not in same page\n", ins->src_line);
	ins->buf[1] = address;
}

/*
 * Vtable for jump instructions with an 8 bit target address.
 */
static struct Vtable j8_ins_vtable = {
	&assemble_j8,
};

/***********************************************************************
 * Public functions
 ***********************************************************************/

/*
 * Allocate an Instruction of given size.
 */
struct Instruction *allocate_instruction(int size, int offset)
{
	void *buf = pool_alloc_buf(asm_pool, size);
	struct Instruction *ins = pool_alloc_buf(gen_pool, sizeof(struct Instruction));

	ins->vtable = &noop_vtable;
	ins->size = size;
	ins->offset = offset;
	ins->src_line = -1;
	ins->buf = buf;
	ins->expr = NULL;
	ins->next = NULL;

	return ins;
}

/*
 * Allocate a one-byte instruction with given opcode.
 */
struct Instruction *ins1(int code)
{
	struct Instruction *ins = allocate_instruction(1, cur_offset);
	ins->buf[0] = code;
	return ins;
}

/*
 * Allocate a two-byte instruction with given opcodes.
 */
struct Instruction *ins2(int byte1, int byte2)
{
	struct Instruction *ins = allocate_instruction(2, cur_offset);
	ins->buf[0] = byte1;
	ins->buf[1] = byte2;
	return ins;
}

/*
 * Create an instruction specifying a general purpose register.
 */
struct Instruction *reg_ins(int opcode, int regnum)
{
	return ins1(opcode | (regnum & GENERAL_REG_MASK));
}

/*
 * Create an instruction with a dereference of R0 or R1.
 */
struct Instruction *deref_ins(int opcode, int regnum)
{
	return ins1(opcode | (regnum & DEREF_REG_MASK));
}

/*
 * Create an instruction with an immedate operand.
 */
struct Instruction *imm_ins(int opcode, struct Expr *imm_val)
{
	struct Instruction *ins = ins2(opcode, 0);
	ins->vtable = &imm_ins_vtable;
	ins->expr = imm_val;
	return ins;
}

/*
 * Create an instruction with an 8 bit jump.
 * Such instructions must target a label in the same
 * 256-byte "page".
 */
struct Instruction *j8_ins(int opcode, struct Expr *addr)
{
	struct Instruction *ins = ins2(opcode, 0);
	ins->vtable = &j8_ins_vtable;
	ins->expr = addr;
	return ins;
}

/*
 * Create a jmp or call instruction.
 */
struct Instruction *jmp_ins(int opcode, struct Expr *addr)
{
	struct Instruction *ins = ins2(opcode, 0);
	ins->vtable = &jmp_vtable;
	ins->expr = addr;
	return ins;
}

/*
 * Create a port instruction.
 */
struct Instruction *port_ins(int opcode, int portnum)
{
	return ins1(opcode | (portnum & PORT_MASK));
}

/*
 * Create a port instruction with an immedate operand value.
 */
struct Instruction *port_imm_ins(int opcode, int portnum, struct Expr *imm_val)
{
	struct Instruction *ins = ins2(opcode | (portnum & PORT_MASK), 0);
	ins->vtable = &imm_ins_vtable;
	ins->expr = imm_val;
	return ins;
}

/*
 * Create a test register and jump instruction (e.g., DJNZ).
 */
struct Instruction *jump_reg_ins(int opcode, int regnum, struct Expr *addr)
{
	struct Instruction *ins = ins2(opcode | (regnum & GENERAL_REG_MASK), 0);
	ins->vtable = &j8_ins_vtable;
	ins->expr = addr;
	return ins;
}

/*
 * Create a JB instruction (jump if accumulator bit is set).
 */
struct Instruction *jb_ins(int bit_num, struct Expr *addr)
{
	struct Instruction *ins = ins2(0x12 | ((bit_num & 0x7) << 5), 0);
	ins->vtable = &j8_ins_vtable;
	ins->expr = addr;
	return ins;
}

/*
 * Create an instruction with both register and immedate value (e.g., MOV Rr, #imm).
 */
struct Instruction *reg_imm_ins(int opcode, int regnum, struct Expr *imm_val)
{
	struct Instruction *ins = ins2(opcode | (regnum & GENERAL_REG_MASK), 0);
	ins->vtable = &imm_ins_vtable;
	ins->expr = imm_val;
	return ins;
}

/*
 * Create an instructino with both a register dereference and
 * immediate value (e.g., MOV @R0, #imm).
 */
struct Instruction *deref_imm_ins(int opcode, int regnum, struct Expr *imm_val)
{
	struct Instruction *ins = ins2(opcode | (regnum & DEREF_REG_MASK), 0);
	ins->vtable = &imm_ins_vtable;
	ins->expr = imm_val;
	return ins;
}

/*
 * Return an Instruction to act as filler to implement an .org directive.
 */
struct Instruction *org(int address, int line_num)
{
	int fill_size = address - cur_offset;
	struct Instruction *fill;

	if (fill_size < 0) {
		err_printf("Line %d: org directive of address %d less than current address %d\n",
			line_num, address, cur_offset);
	}

	fill = allocate_instruction(fill_size, cur_offset);
	memset(fill->buf, '\0', fill_size);
	return fill;
}

/*
 * Append given Instruction onto the end of the
 * instruction list.
 */
void append(struct Instruction *ins)
{
	assert(ins->next == NULL);
	if (ins_head == NULL) {
		ins_head = ins_tail = ins;
	} else {
		assert(ins->offset > ins_tail->offset);
		ins_tail->next = ins;
		ins_tail = ins;
	}
	cur_offset += ins->size;
}
