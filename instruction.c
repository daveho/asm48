/*
 * Assembler for the Intel 8048 microcontroller family.
 * Copyright (c) 2002,2003 David H. Hovemeyer <daveho@cs.umd.edu>
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
 * Assemble() method for 8-bits constant. 
 */
static void assemble_db(struct Instruction *ins)
{
	int imm_val = eval_expr(ins->cur_file, ins->expr);
	if (imm_val < -128 || imm_val > 255)
		warn_printf("[%s] Line %d: immediate value %d exceeds range\n", ins->cur_file, ins->src_line, imm_val);
	ins->buf[0] = imm_val;
}

/*
 * Vtable for 8-bits constant. 
 */
static struct Vtable db_vtable = {
	&assemble_db,
};

/*
 * Assemble() method for 16-bits constant. 
 */
static void assemble_dw(struct Instruction *ins)
{
	int imm_val = eval_expr(ins->cur_file, ins->expr);
	if (imm_val < -32768 || imm_val > 65535)
		warn_printf("[%s] Line %d: immediate value %d exceeds range\n", ins->cur_file, ins->src_line, imm_val);
	ins->buf[0] = imm_val & 0xFF;
	ins->buf[1] = imm_val >> 8;
}

/*
 * Vtable for 16-bits constant. 
 */
static struct Vtable dw_vtable = {
	&assemble_dw,
};

/*
 * Assemble() method for instructions with immedate constants. 
 */
static void assemble_imm_ins(struct Instruction *ins)
{
	int imm_val = eval_expr(ins->cur_file, ins->expr);
	/*printf("imm_val = %d\n", imm_val);*/
	if (imm_val < -128 || imm_val > 255)
		warn_printf("[%s] Line %d: immediate value %d exceeds range\n", ins->cur_file, ins->src_line, imm_val);
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
	int address = eval_expr(ins->cur_file, ins->expr);
	/*printf("address = %d\n", address);*/
	/*
	 * Ensure that address is in range.
	 */
	if (address < 0 || address >= MAX_ADDR)
		warn_printf("[%s] Line %d: address %d is out of range\n", ins->cur_file, ins->src_line, address);
	ins->buf[0] |= ((address >> 3) & 0xE0);
	ins->buf[1] = address & 0xFF;
	/*printf("Call to address %x - %2.2x %2.2x\n", address, ins->buf[0], ins->buf[1]);*/
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
	int address = eval_expr(ins->cur_file, ins->expr);
	/*printf("address = %d\n", address);*/
	if ((address & PAGE_MASK) != ((ins->offset+1) & PAGE_MASK))
		warn_printf("[%s] Line %d: jump offset not in same page\n", ins->cur_file, ins->src_line);
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
	ins->cur_file = cur_file;
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
		err_printf("[%s] Line %d: org directive of address %d less than current address %d\n",
			cur_file, line_num, address, cur_offset);
	}

	fill = allocate_instruction(fill_size, cur_offset);
	memset(fill->buf, '\0', fill_size);
	return fill;
}

/*
 * Return an Instruction to act as filler to implement an .orgfill directive.
 */
struct Instruction *orgfill(int address, int value, int line_num)
{
	int fill_size = address - cur_offset;
	struct Instruction *fill;

	if (fill_size < 0) {
		err_printf("[%s] Line %d: orgfill directive of address %d less than current address %d\n",
			cur_file, line_num, address, cur_offset);
	}

	fill = allocate_instruction(fill_size, cur_offset);
	memset(fill->buf, value, fill_size);
	return fill;
}

/*
 * Assemble a literal byte value.
 */
struct Instruction *db(int value, int line_num)
{
	struct Instruction *db_ins = ins1(value);
	if (value < -128 || value > 255)
		warn_printf("[%s] Line %d: value %d is out of range for byte\n", cur_file, line_num, value);
	return db_ins;
}

/*
 * Assemble a literal byte value with bits reversed.
 */
struct Instruction *dbr(int value, int line_num)
{
	struct Instruction *db_ins;
	int value2 = 0;
	if (value < -128 || value > 255)
		warn_printf("[%s] Line %d: value %d is out of range for byte\n", cur_file, line_num, value);
	if (value & 0x01) value2 |= 0x80;
	if (value & 0x02) value2 |= 0x40;
	if (value & 0x04) value2 |= 0x20;
	if (value & 0x08) value2 |= 0x10;
	if (value & 0x10) value2 |= 0x08;
	if (value & 0x20) value2 |= 0x04;
	if (value & 0x40) value2 |= 0x02;
	if (value & 0x80) value2 |= 0x01;
	db_ins = ins1(value2);
	return db_ins;
}

/*
 * Assemble a literal byte value.
 */
struct Instruction *db_expr(struct Expr *expr_val, int line_num)
{
	struct Instruction *db_ins = allocate_instruction(1, cur_offset);
	db_ins->vtable = &db_vtable;
	db_ins->expr = expr_val;
	db_ins->src_line = line_num;
	return db_ins;
}

/*
 * Assemble a literal word value.
 */
struct Instruction *dw_expr(struct Expr *expr_val, int line_num)
{
	struct Instruction *dw_ins = allocate_instruction(2, cur_offset);
	dw_ins->vtable = &dw_vtable;
	dw_ins->expr = expr_val;
	dw_ins->src_line = line_num;
	return dw_ins;
}

/*
 * Include a binary
 */
struct Instruction *incbin(char *filename, int line_num)
{
	FILE *f;
	int file_size;
	struct Instruction *data;

	filename[strlen(filename)-1] = '\0'; ++filename;
	f = fopen(filename, "rb");

	if(f == NULL) {
		warn_printf("[%s] Line %d: unable to open file %s\n", cur_file, line_num, filename);
		data = allocate_instruction(0, cur_offset);
		return data;
	}

	fseek(f, 0, SEEK_END);
	file_size = ftell(f);
	data = allocate_instruction(file_size, cur_offset);
	fseek(f, 0, SEEK_SET);
	fread(data->buf, 1, file_size, f);
	fclose(f);
	return data;
}

/*
 * Append given Instruction onto the end of the
 * instruction list.
 */
void append(struct Instruction *ins)
{
	int i;
	assert(ins->next == NULL);
	if (ins_head == NULL) {
		ins_head = ins_tail = ins;
	} else {
		assert(ins->offset >= ins_tail->offset);
		assert(ins->offset != ins_tail->offset || ins_tail->size == 0);
		ins_tail->next = ins;
		ins_tail = ins;
	}
	if (cur_offset < (BANK_USAGE_MAX * 256)) {
		for (i=0; i<ins->size; i++) bank_usage[(cur_offset+i)>>8]++;
	}
	cur_offset += ins->size;
}

/*
 * Append given Instruction onto the end of the
 * instruction list. It doesn't add into the bank usage statistic
 */
void append_nostat(struct Instruction *ins)
{
	assert(ins->next == NULL);
	if (ins_head == NULL) {
		ins_head = ins_tail = ins;
	} else {
		assert(ins->offset >= ins_tail->offset);
		assert(ins->offset != ins_tail->offset || ins_tail->size == 0);
		ins_tail->next = ins;
		ins_tail = ins;
	}
	cur_offset += ins->size;
}
