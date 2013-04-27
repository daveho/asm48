/*
 * Assembler for the Intel 8048 microcontroller family.
 * Copyright (c) 2003 David H. Hovemeyer <daveho@cs.umd.edu>
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
#include <string.h>
#include <assert.h>
#include "parse.tab.h"
#include "asm48.h"

/*
 * Make an Expr object with given field values.
 */
static struct Expr *mk_expr(int op, struct Expr *left, struct Expr *right, const char *sym, int value, int line_num, int mustexist)
{
	struct Expr *expr = (struct Expr*) pool_alloc_buf(gen_pool, sizeof(struct Expr));
	expr->op = op;
	expr->left = left;
	expr->right = right;
	expr->sym = sym;
	expr->value = value;
	expr->line_num = line_num;
	expr->cur_offset = cur_offset;
	expr->mustexist = mustexist;
	return expr;
}

/*
 * Make a constant expression with given value.
 */
struct Expr *mk_const_expr(int ival, int line_num)
{
	return mk_expr(INT_VALUE, NULL, NULL, NULL, ival, line_num, 1);
}

/*
 * Make a symbolic expression referring to given symbol.
 */
struct Expr *mk_symbolic_expr(const char *sym, int line_num, int mustexist)
{
	return mk_expr(IDENTIFIER, NULL, NULL, sym, -1, line_num, mustexist);
}

/*
 * Make a unary expression.
 */
struct Expr *mk_unary_expr(int op, struct Expr *subexpr, int line_num)
{
	return mk_expr(op, subexpr, NULL, NULL, -1, line_num, 1);
}

/*
 * Make a binary expression.
 */
struct Expr *mk_binary_expr(int op, struct Expr *left, struct Expr *right, int line_num)
{
	return mk_expr(op, left, right, NULL, -1, line_num, 1);
}

/*
 * Evaluate an expression.
 */
int eval_expr(char *cur_file, struct Expr *expr)
{
	struct Symbol *symbol;
	int lval, rval;

	switch (expr->op) {
		case INT_VALUE:
			return expr->value;

		case IDENTIFIER:
			if (strcmp(expr->sym, ".here") == 0)	/* addr of current instruction */
				return expr->cur_offset;
			symbol = lookup_symbol(expr->sym);
			if (symbol == NULL) {
				if (expr->mustexist)
					err_printf("[%s] Line %d: Unknown symbol '%s'\n", cur_file, expr->line_num, expr->sym);
				else
					return 0;
			}
			return symbol->value;

		case UMINUS:
			return 0 - eval_expr(cur_file, expr->left);

		case UNOTLOGIC:
			return ~eval_expr(cur_file, expr->left);

		case ULOW:
			return eval_expr(cur_file, expr->left) & 255;

		case UHIGH:
			return (eval_expr(cur_file, expr->left) >> 8) & 255;
	}

	lval = eval_expr(cur_file, expr->left);
	rval = eval_expr(cur_file, expr->right);

	switch (expr->op) {
		case '=':
			return lval == rval;

		case '!':
			return lval != rval;

		case '<':
			return lval < rval;

		case '>':
			return lval > rval;

		case 'l':
			return lval <= rval;

		case 'g':
			return lval >= rval;

		case 'a':
			return lval && rval;

		case 'o':
			return lval || rval;

		case '+':
			return lval + rval;

		case '-':
			return lval - rval;

		case '*':
			return lval * rval;

		case '/':
			if (rval == 0)
				err_printf("[%s] Line %d: Attempt to divide by zero\n", cur_file, expr->line_num);
			return lval / rval;

		case '%':
			if (rval == 0)
				err_printf("[%s] Line %d: Attempt to modulo by zero\n", cur_file, expr->line_num);
			return lval % rval;

		case LSHIFT:
			return lval << rval;

		case RSHIFT:
			return lval >> rval;

		case '&':
			return lval & rval;

		case '|':
			return lval | rval;

		case '^':
			return lval ^ rval;

		default:
			assert(0); /* Unknown operation! */
	}

	return -1;
}
