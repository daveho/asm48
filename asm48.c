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

/* Filename of currently source being assembled. */
char *cur_file;

/* Bank usage */
int bank_display = 0;
int bank_usage[BANK_USAGE_MAX];

/*
 * Modify current filename
 */
void cur_file_set(const char *filename)
{
	const char *s = filename + strlen(filename);
	while (s != filename) {
		if ((*s == '\\') || (*s == '/')) { s++; break; }
		s--;
	}
	cur_file = strdup(s);	// It will leak but is required
}

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
		"  -v               Print version number only and exit\n"
		"  -t               Print ROM bank usage table\n"
		"  -s <filename>    Export symbols list\n"
		"  -o <filename>    Specify the name of the output file\n"
		"  -f (bin|hex)     Specify output format (binary or Intel hex; default bin)\n";

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

	printf("   Assembled %d bytes.\n", cur_offset);

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
static void (*output_func)(const char *) = &output_bin;

/* Suffix of output file (if automatically generated from input filename. */
static const char *output_suffix = ".bin";

/* Name of symbols file. */
static char *symbols_file = NULL;

/*
 * Parse command line options.
 */
static void parse_options(int argc, char **argv)
{
	int opt;

	opterr = 0;

	while ((opt = getopt(argc, argv, "vts:o:f:")) != -1) {
		switch (opt) {
			case 'v':
				exit(0);
			case 't':
				bank_display = 1;
				break;
			case 's':
				symbols_file = optarg;
				break;
			case 'o':
				output_file = optarg;
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
	int i;

	fprintf(stderr, "*** asm48 v" VERSION " ***\n");
	parse_options(argc, argv);

	yyin = fopen(input_file, "r");
	if (yyin == NULL) {
		err_printf("Couldn't open input file %s: %s\n", input_file, strerror(errno));
		exit(1);
	}

	memset(bank_usage, 0, sizeof(bank_usage));
	gen_pool = create_pool(GEN_POOL_SIZE);
	asm_pool = create_pool(ASM_POOL_SIZE);

	cur_file_set(input_file);
	yyparse();
	assemble();
	if (symbols_file) export_symbols(symbols_file);
	output_func(output_file);

	if (bank_display) {
		printf("\n   ROM banks usage:\n");
		for (i=0; i<BANK_USAGE_MAX; i++) {
			if (bank_usage[i]) {
				printf(" bank%3d, %4d occupied, %4d free, %3d%% usage\n",
				 i, bank_usage[i], 256 - bank_usage[i], bank_usage[i] * 100 / 256);
			}
		}
	}

	return 0;
}
