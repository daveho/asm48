/*
 * Assembler for the Intel 8048 microcontroller family.
 * Copyright (c) 2002, David H. Hovemeyer <daveho@cs.umd.edu>
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
#include <errno.h>
#include "asm48.h"

/* FIXME: currently a linked list. Should change to a hash table. */
static struct Symbol *sym_head;

/*
 * Allocate a duplicate of given string from the string pool.
 */
const char *dup_str(const char *str)
{
	size_t len = strlen(str);
	char *buf = pool_alloc_buf(gen_pool, len + 1);
	strcpy(buf, str);
	return buf;
}

/*
 * Define a symbol.
 */
void define_symbol(const char *name, int value, int type)
{
	struct Symbol *sym;

	sym = lookup_symbol(name);
	if (sym != NULL)
		err_printf("Redefinition of symbol %s\n", name);
	sym = pool_alloc_buf(gen_pool, sizeof(struct Symbol));
	sym->name = dup_str(name);
	sym->value = value;
	sym->type = type;

	sym->next = sym_head;
	sym_head = sym;
}

/*
 * Re-define a symbol.
 */
void redefine_symbol(const char *name, int value, int type)
{
	struct Symbol *sym;

	sym = lookup_symbol(name);
	if (sym == NULL) {
		sym = pool_alloc_buf(gen_pool, sizeof(struct Symbol));
		sym->name = dup_str(name);
		sym->value = value;
		sym->type = type;

		sym->next = sym_head;
		sym_head = sym;
	} else {
		if (sym->type != type)
			err_printf("Symbol %s type conflict\n", name);

		sym->value = value;
	}
}

/*
 * Look up symbol with given name.
 * Returns NULL if no such symbol exists.
 */
struct Symbol *lookup_symbol(const char *name)
{
	struct Symbol *cur = sym_head;

	while (cur != NULL) {
		if (strcmp(cur->name, name) == 0)
			break;
		cur = cur->next;
	}

	return cur;
}

/*
 * Export symbols into a file stream.
 */
void export_symbols(const char *filename)
{
	struct Symbol *cur;
	FILE *f = fopen(filename, "w");
	if (!f) {
		err_printf("Couldn't open symbols file %s: %s\n", filename, strerror(errno));
	}

	fprintf(f, "; *** asm48 v" VERSION " ***\n");
	fprintf(f, "\n; Constants\n");
	cur = sym_head;
	while (cur != NULL) {
		if (cur->type == SYMB_CONST) fprintf(f, "%08X\t%s\n", cur->value, cur->name);
		cur = cur->next;
	}
	fprintf(f, "\n; Labels\n");
	cur = sym_head;
	while (cur != NULL) {
		if (cur->type == SYMB_LABEL) fprintf(f, "%08X\t%s\n", cur->value, cur->name);
		cur = cur->next;
	}

	fclose(f);
}
