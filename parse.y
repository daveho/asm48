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

%{
#include <stdio.h>
#include <stdlib.h>
#include "asm48.h"

void yyerror(const char *msg);
int yylex(void);
extern int lex_src_line;
static int parse_src_line;
%}

%token A BUS PSW C I TCNTI CLK T TCNT CNT
%token<port_num> P0 P12 P47
%token<reg_num> DEREF_REG GENERAL_REG 
%token<bit_num> JB F MB RB
%token<identifier> IDENTIFIER
%token<ival> INT_VALUE
%token EOL

%token ADD ADDC ANL ANLD CALL CLR CPL DA DEC DIS DJNZ EN
%token ENT0 IN INC INS JC JF0 JF1 JMP JMPP JNC JNI JNT0 JNT1 JNZ
%token JTF JT0 JT1 JZ MOV MOVD MOVP MOVP3 MOVX NOP ORL ORLD OUTL
%token RET RETR RL RLC RR RRC SEL STOP STRT SWAP XCH XCHD
%token XRL

%token EQU ORG DB
%token LSHIFT RSHIFT
%token UMINUS

%type<reg_num> any_reg
%type<expr> imm_val address
%type<expr> expr bitwise_or_expr bitwise_and_expr shift_expr additive_expr mult_expr unary_expr primary_expr

%union {
	int reg_num;
	int port_num;
	int bit_num;
	int ival;
	const char *identifier;
	struct Expr *expr;
}

%%

instruction_list :
	  { parse_src_line = lex_src_line; } instruction instruction_list
	| /* epsilon */
	;

instruction :
	   instruction_expr { ins_tail->src_line = parse_src_line; } EOL
	| equate_directive EOL
	| org_directive EOL
	| db_directive EOL
	| label
	| EOL
	;

label :
	  IDENTIFIER ':' { define_symbol($1, cur_offset); }
	;

equate_directive :
	  EQU IDENTIFIER ',' expr { define_symbol($2, eval_expr($4)); }
	;

org_directive :
	  ORG INT_VALUE			{ append(org($2, parse_src_line)); }
	;

db_directive :
	  DB expr			{ append(db(eval_expr($2), parse_src_line)); }
	;

instruction_expr :
	  ADD A ',' any_reg		{ append(reg_ins(0x68, $4)); }
	| ADD A ',' '@' DEREF_REG	{ append(deref_ins(0x60, $5)); }
	| ADD A ',' imm_val		{ append(imm_ins(0x03, $4)); }
	| ADDC A ',' any_reg		{ append(reg_ins(0x78, $4)); }
	| ADDC A ',' '@' DEREF_REG	{ append(deref_ins(0x70, $5)); }
	| ADDC A ',' imm_val		{ append(imm_ins(0x13, $4)); }
	| ANL A ',' any_reg		{ append(reg_ins(0x58, $4)); }
	| ANL A ',' '@' DEREF_REG	{ append(deref_ins(0x50, $5)); }
	| ANL A ',' imm_val		{ append(imm_ins(0x53, $4)); }
	| ANL BUS ',' imm_val		{ append(imm_ins(0x98, $4)); }
	| ANL P12 ',' imm_val		{ append(port_imm_ins(0x98, $2, $4)); }
	| ANLD P47 ',' A		{ append(port_ins(0x9C, $2)); }
	| CALL address			{ append(jmp_ins(0x14, $2)); }
	| CLR A				{ append(ins1(0x27)); }
	| CLR C				{ append(ins1(0x97)); }
	| CLR F				{ append(ins1(0x85 | (($2 & 0x1) << 5))); }
	| CPL A				{ append(ins1(0x37)); }
	| CPL C				{ append(ins1(0xA7)); }
	| CPL F				{ append(ins1(0x95 | (($2 & 0x1) << 5))); }
	| DA A				{ append(ins1(0x57)); }
	| DEC A				{ append(ins1(0x07)); }
	| DEC any_reg			{ append(reg_ins(0xC8, $2)); }
	| DIS I				{ append(ins1(0x15)); }
	| DIS TCNTI			{ append(ins1(0x35)); }
	| DJNZ any_reg ',' address 	{ append(jump_reg_ins(0xE8, $2, $4)); }
	| EN I				{ append(ins1(0x05)); }
	| EN TCNTI			{ append(ins1(0x25)); }
	| ENT0 CLK			{ append(ins1(0x75)); }
	| IN A ',' P12			{ append(port_ins(0x8, $4)); }
	| INC A				{ append(ins1(0x17)); }
	| INC any_reg			{ append(reg_ins(0x18, $2)); }
	| INC '@' DEREF_REG		{ append(deref_ins(0x10, $3)); }
	| IN A ',' P0			{ append(ins1(0x08)); } /* FIXME: is this right? Manual doesn't list opcode!??? */
	| INS A ',' BUS			{ append(ins1(0x08)); }
	| JB address			{ append(jb_ins($1, $2)); }
	| JC address			{ append(j8_ins(0xF6, $2)); }
	| JF0 address			{ append(j8_ins(0xB6, $2)); }
	| JF1 address			{ append(j8_ins(0x76, $2)); }
	| JMP address			{ append(jmp_ins(0x04, $2)); }
	| JMPP '@' A			{ append(ins1(0xB3)); }
	| JNC address			{ append(j8_ins(0xE6, $2)); }
	| JNI address			{ append(j8_ins(0x86, $2)); }
	| JNT0 address			{ append(j8_ins(0x26, $2)); }
	| JNT1 address			{ append(j8_ins(0x46, $2)); }
	| JNZ address			{ append(j8_ins(0x96, $2)); }
	| JTF address			{ append(j8_ins(0x16, $2)); }
	| JT0 address			{ append(j8_ins(0x36, $2)); }
	| JT1 address			{ append(j8_ins(0x56, $2)); }
	| JZ address			{ append(j8_ins(0xC6, $2)); }
	| MOV A ',' imm_val		{ append(imm_ins(0x23, $4)); }
	| MOV A ',' PSW			{ append(ins1(0xC7)); }
	| MOV A ',' any_reg		{ append(reg_ins(0xF8, $4)); }
	| MOV A ',' '@' DEREF_REG	{ append(deref_ins(0xF0, $5)); }
	| MOV A ',' T			{ append(ins1(0x42)); }
	| MOV PSW ',' A			{ append(ins1(0xD7)); }
	| MOV any_reg ',' A		{ append(reg_ins(0xA8, $2)); }
	| MOV any_reg ',' imm_val	{ append(reg_imm_ins(0xB8, $2, $4)); }
	| MOV '@' DEREF_REG ',' A	{ append(deref_ins(0xA0, $3)); }
	| MOV '@' DEREF_REG ',' imm_val	{ append(deref_imm_ins(0xB0, $3, $5)); }
	| MOV T ',' A			{ append(ins1(0x62)); }
	| MOVD A ',' P47		{ append(port_ins(0x0C, $4)); }
	| MOVD P47 ',' A		{ append(port_ins(0x3C, $2)); }
	| MOVP A ',' '@' A		{ append(ins1(0xA3)); }
	| MOVP3 A ',' '@' A		{ append(ins1(0xE3)); }
	| MOVX A ',' '@' DEREF_REG	{ append(deref_ins(0x80, $5)); }
	| MOVX '@' DEREF_REG ',' A	{ append(deref_ins(0x90, $3)); }
	| NOP				{ append(ins1(0x00)); }
	| ORL A ',' any_reg		{ append(reg_ins(0x48, $4)); }
	| ORL A ',' '@' DEREF_REG	{ append(deref_ins(0x40, $5)); }
	| ORL A ',' imm_val		{ append(imm_ins(0x43, $4)); }
	| ORL BUS ',' imm_val		{ append(imm_ins(0x88, $4)); }
	| ORL P12 ',' imm_val		{ append(port_imm_ins(0x88, $2, $4)); }
	| ORLD P47 ',' A		{ append(port_ins(0x8C, $2)); }
	| OUTL P0 ',' A			{ append(ins1(0x90)); }
	| OUTL BUS ',' A		{ append(ins1(0x02)); }
	| OUTL P12 ',' A		{ append(port_ins(0x38, $2)); }
	| RET				{ append(ins1(0x83)); }
	| RETR				{ append(ins1(0x93)); }
	| RL A				{ append(ins1(0xE7)); }
	| RLC A				{ append(ins1(0xF7)); }
	| RR A				{ append(ins1(0x77)); }
	| RRC A				{ append(ins1(0x67)); }
	| SEL MB			{ append(ins1(0xE5 | (($2 & 0x1) << 4))); }
	| SEL RB			{ append(ins1(0xC5 | (($2 & 0x1) << 4))); }
	| STOP TCNT			{ append(ins1(0x65)); }
	| STRT CNT			{ append(ins1(0x45)); }
	| STRT T			{ append(ins1(0x55)); }
	| SWAP A			{ append(ins1(0x47)); }
	| XCH A ',' any_reg		{ append(reg_ins(0x28, $4)); }
	| XCH A ',' '@' DEREF_REG	{ append(deref_ins(0x20, $5)); }
	| XCHD A ',' '@' DEREF_REG	{ append(deref_ins(0x30, $5)); }
	| XRL A ',' any_reg		{ append(reg_ins(0xD8, $4)); }
	| XRL A ',' '@' DEREF_REG	{ append(deref_ins(0xD0, $5)); }
	| XRL A ',' imm_val		{ append(imm_ins(0xD3, $4)); }
	;

any_reg :
	  DEREF_REG
	| GENERAL_REG
	;

imm_val : expr ;

address : expr ;

/*
 * Expression grammar.
 * Operators accepted are bitwise & and |, left and right shift,
 * plus/minus/times/divide, and unary plus/minus.
 * Precedence levels are like the corresponding operators in C.
 * All operations are done using host platform ints.
 */

expr :
	  bitwise_or_expr
	;

bitwise_or_expr :
	  bitwise_and_expr
	| bitwise_or_expr '|' bitwise_and_expr { $$ = mk_binary_expr('|', $1, $3, parse_src_line); }
	;

bitwise_and_expr :
	  shift_expr
	| bitwise_and_expr '&' shift_expr { $$ = mk_binary_expr('&', $1, $3, parse_src_line); }
	;

shift_expr :
	  additive_expr
	| shift_expr LSHIFT additive_expr { $$ = mk_binary_expr(LSHIFT, $1, $3, parse_src_line); }
	| shift_expr RSHIFT additive_expr { $$ = mk_binary_expr(RSHIFT, $1, $3, parse_src_line); }
	;

additive_expr :
	  mult_expr
	| additive_expr '+' mult_expr { $$ = mk_binary_expr('+', $1, $3, parse_src_line); }
	| additive_expr '-' mult_expr { $$ = mk_binary_expr('-', $1, $3, parse_src_line); }
	;

mult_expr :
	  unary_expr
	| mult_expr '*' unary_expr { $$ = mk_binary_expr('*', $1, $3, parse_src_line); }
	| mult_expr '/' unary_expr { $$ = mk_binary_expr('/', $1, $3, parse_src_line); }
	;

unary_expr :
	  '+' unary_expr { $$ = $2; }
	| '-' unary_expr { $$ = mk_unary_expr(UMINUS, $2, parse_src_line); }
	| primary_expr
	;

/*
 * We allow the immediate prefix '#' on numeric and symbolic constant
 * values, but we don't require it.  Therefore, the following are
 * equivalent:
 *     MOV A, #ABh
 *     MOV A, ABh
 */
primary_expr :
	  IDENTIFIER { $$ = mk_symbolic_expr($1, parse_src_line); }
	| '#' IDENTIFIER { $$ = mk_symbolic_expr($2, parse_src_line); }
	| INT_VALUE { $$ = mk_const_expr($1, parse_src_line); }
	| '#' INT_VALUE { $$ = mk_const_expr($2, parse_src_line); }
	| '(' expr ')' { $$ = $2; }
	;

%%

void yyerror(const char *msg)
{
	err_printf("At line %d: %s\n", parse_src_line, msg);
}
