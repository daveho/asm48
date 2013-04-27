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

#include <stdlib.h>
#include "asm48.h"

/*
 * Create a memory pool with given number of bytes.
 */
struct Pool *create_pool(int size)
{
	struct Pool *pool;

	pool = malloc(sizeof(struct Pool));
	if (pool == NULL)
		err_printf("Unable to allocate pool of size %d bytes\n", size);
	pool->buf = malloc(size);

	if (pool->buf == NULL) {
		free(pool);
		err_printf("Unable to allocate pool of size %d bytes\n", size);
	}

	pool->size = size;
	pool->start = 0;

	return pool;
}

/**
 * Allocate a buffer from given memory pool.
 */
void *pool_alloc_buf(struct Pool *pool, int size)
{
	int next_start = pool->start + size;
	void *result;

	if (next_start > pool->size)
		err_printf("Unable to satisfy allocation of %d bytes\n", size);

	result = (char*)pool->buf + pool->start;
	pool->start = next_start;

	return result;
}
