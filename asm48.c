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
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_GETOPT
#include <unistd.h>
#endif

#include  "asm48.h"

#ifndef HAVE_GETOPT
extern int opterr, optind, optopt, optreset;
extern char *optarg;
int getopt(int nargc, char * const *nargv, const char *ostr);
#endif

/* The list of generated instructions. */
struct Instruction *ins_head, *ins_tail;

/* Memory pool for internal objects. */
struct Pool *gen_pool;

/* Memory pool for assembled instructions. */
struct Pool *asm_pool;

/* Offset of instruction currently being assembled. */
int cur_offset;

/*
 * Assemble all instructions (to resolve forward references).
 */
static void assemble(void)
{
	struct Instruction *cur = ins_head;
	while (cur != NULL) {
		cur->vtable->assemble(cur);
		cur = cur->next;
	}
}

/*
 * Print command line usage information.
 */
static void usage(void)
{
	const char *msg =
		"Usage: asm48 [options] <input file>\n"
		"Options:\n"
		"  -o <filename>    Specify the name of the output file\n"
		"  -f (bin|hex)     Specify output format (binary or Intel hex; default hex)\n";

	fprintf(stderr, "%s", msg);
}

/*
 * Output a flat binary file.
 */
static void output_bin(const char *filename)
{
	FILE *fp = fopen(filename, "wb");
	if (fp == NULL)
		err_printf("Couldn't open %s for output: %s\n", filename, strerror(errno));

	if (fwrite(asm_pool->buf, 1, cur_offset, fp) != cur_offset)
		err_printf("Failed to write %d bytes of output to %s: %s\n", cur_offset, filename, strerror(errno));

	fclose(fp);
}

/*
 * Output Intel hex format.
 */
static void output_hex(const char *filename)
{
	extern int memory[];
	unsigned char *ptr = (unsigned char *) asm_pool->buf;
	int i;
	char cmd_str[256];

	for (i = 0; i < cur_offset; ++i)
		memory[i] = *ptr++;

	sprintf(cmd_str, "S 0 %x %s", cur_offset - 1, filename);
	save_file(cmd_str);
}

/* Name of input file. */
static const char *input_file;

/* Name of output file. */
static char *output_file = NULL;

/* Output function to emit assembled instructions. */
static void (*output_func)(const char *) = &output_hex;

/* Suffix of output file (if automatically generated from input filename. */
static const char *output_suffix = ".hex";

/*
 * Parse command line options.
 */
static void parse_options(int argc, char **argv)
{
	int opt;

	opterr = 0;

	while ((opt = getopt(argc, argv, "o:f:")) != -1) {
		switch (opt) {
			case 'o':
				input_file = optarg;
				break;
			case 'f':
				if (strcmp(optarg, "bin") == 0) {
					output_func = &output_bin;
					output_suffix = ".bin";
				} else if (strcmp(optarg, "hex") == 0) {
					output_func = &output_hex;
					output_suffix = ".hex";
				} else {
					fprintf(stderr, "Unknown output format \"%s\"\n", optarg);
					usage();
					exit(1);
				}
				break;
			case '?':
				fprintf(stderr, "Unknown option '%c'\n", optopt);
				usage();
				exit(1);
		}
	}

	/*
	 * The last command line argument should be
	 * the input file.
	 */
	if (optind != argc - 1) {
		usage();
		exit(1);
	}
	input_file = argv[optind];

	/*
	 * If no output file was specified, transform the name of the input file,
	 * adding a suitable file extension.
	 */
	if (output_file == NULL) {
		char *input_suffix = strrchr(input_file, '.');
		size_t ilen = strlen(input_file);
		size_t osfxlen = strlen(output_suffix);

		if (input_suffix == NULL) {
			output_file = (char *) malloc(ilen + osfxlen + 1);
			strcpy(output_file, input_file);
			strcat(output_file, output_suffix);
		} else {
			size_t ibaselen = (input_suffix - input_file);
			output_file = (char *) malloc(ibaselen + osfxlen + 1);
			memcpy(output_file, input_file, ibaselen);
			strcpy(output_file + ibaselen, output_suffix);
		}
	}
}

/*
 * main() function.
 */
int main(int argc, char **argv)
{
	extern FILE *yyin;
	extern void yyparse(void);

	parse_options(argc, argv);

	yyin = fopen(input_file, "r");
	if (yyin == NULL) {
		err_printf("Couldn't open input file %s: %s\n", input_file, strerror(errno));
		exit(1);
	}

	gen_pool = create_pool(GEN_POOL_SIZE);
	asm_pool = create_pool(ASM_POOL_SIZE);

	yyparse();
	assemble();
	output_func(output_file);

	return 0;
}
