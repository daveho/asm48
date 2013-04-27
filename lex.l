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

%{
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "asm48.h"
#include "parse.tab.h"

/*
 * Current source line.
 */
int lex_src_line = 1;

/*
 * Return the value of a digit character.
 */
static int digit_value(char ch)
{
	assert(isdigit(ch));
	return ch - '0';
}

/*
 * Return the value of given hex constant.
 * The constant may be of the form of either
 * 0x??? or ???H, where ??? is a sequence of
 * hex digits.
 */
static int hex_const_value(const char *str)
{
    int value;

    if (str[0] == '0' && tolower(str[1]) == 'x') {
            sscanf(str + 2, "%x", &value);
    }
    else if (str[0] == '%' || (str[0] == '0' && tolower(str[1]) == 'b')) {
            value = 0;
            if ( str[0] == '%' ) {
                    str += 1;
            } else {
                    str += 2;
            }
            for (;;) {
                    int ch = *str++;

                    if (ch != '0' && ch != '1' )
                            break;

                    value <<= 1;

                    if ( ch == '1' )
                            value++;
            }
    } else {
            value = 0;
            while (*str) {
                    int ch = tolower(*str++);
                    if (ch == 'h')
                            break;
                    value <<= 4;
                    if (isdigit(ch))
                            value += (ch - '0');
                    else {
                            assert(tolower(ch) >= 'a' && tolower(ch) <= 'f');
                            value += (10 + (tolower(ch) - 'a'));
                    }
            }
    }
    return value;
}

#define MAX_INCLUDE_DEPTH 32
static struct {
	char filename[512];
	YY_BUFFER_STATE state;
	int lineno;
	int if_run;
} include_stack[MAX_INCLUDE_DEPTH];
int include_stack_ptr = 0;

#define MAX_IF_DEPTH 32
int if_stack[MAX_IF_DEPTH];
int if_stack_ptr = 0;
int if_run = 2; /* 0 = Never, 1 = Off, 2 = On */

int include_lex(void);
int eof_lex(void);
int if_push_lex(int state);
int if_pop_lex(void);
int if_else_lex(void);

%}

ZEROONE         [0-1]
DIGIT           [0-9]
HEX             ({DIGIT}|[A-Fa-f])
IDSTART         [A-Za-z_]
IDCHAR          ({IDSTART}|{DIGIT})
HWS             [ \t\r]
SINGLE          [@,:()+\-\*/#\&\|!=<>~]
STRING          \"(\\.|[^\\"])*\"
INCLUDE         ^{HWS}*\.(INCLUDE|include){HWS}+{STRING}
IFD             ^{HWS}*\.(IF|if){HWS}+.*
IFDEFD          ^{HWS}*\.(IFDEF|ifdef){HWS}+.*
IFNDEFD         ^{HWS}*\.(IFNDEF|ifndef){HWS}+.*
IFSETD          ^{HWS}*\.(IFSET|ifset){HWS}+.*
IFNSETD         ^{HWS}*\.(IFNSET|ifnset){HWS}+.*

%x	ifskip

/* This tells flex to read only one input file
   added by sy2002 to achieve a better cross-platform makefile compatibility, since
   otherwise we'd need different link options for win32, linux and OSX */
%option noyywrap

%%

		/* Skip comments. */
";".*"\n"	{ ++lex_src_line; return EOL; }
";".*		{ ++lex_src_line; return EOL; }

		/* Skip horizontal whitespace. */
{HWS}+		{ }

		/* End of line character. */
"\n"		{ ++lex_src_line; return EOL; }

		/* Accumulator register. */
(A|a)		{ return A; }

		/* Dereference-capable register, R0 and R1. */
[Rr][01]	{ yylval.reg_num = digit_value(yytext[1]); return DEREF_REG; }

		/* General register, R2 - R7. */
[Rr][2-7]	{ yylval.reg_num = digit_value(yytext[1]); return GENERAL_REG; }

		/* Port P0. */
[Pp]0		{ yylval.port_num = 0; return P0; }

		/* Ports P1 and P2. */
[Pp][12]	{ yylval.port_num = digit_value(yytext[1]); return P12; }

		/* Ports P4 - P7. */
[Pp][4567]	{ yylval.port_num = digit_value(yytext[1]); return P47; }

		/* Flags (F0 and F1). */
[Ff][01]	{ yylval.bit_num = digit_value(yytext[1]); return F; }

		/* Program status word. */
(PSW|psw)	{ return PSW; }

		/* BUS. */
(BUS|bus)	{ return BUS; }

		/* Carry flag. */
(C|c)		{ return C; }

		/* Interrupt flag. */
(I|i)		{ return I; }

		/* Time counter interrupt. */
(TCNTI|tcnti)	{ return TCNTI; }

		/* Clock output. */
(CLK|clk)	{ return CLK; }

		/* Timer. */
(T|t)		{ return T; }

		/* Timer and event counter. (STOP TCNT instruction). */
(TCNT|tcnt)	{ return TCNT; }

		/* Event counter. */
(CNT|cnt)	{ return CNT; }

		/* Memory banks (MB0 and MB1). */
(MB|mb)[01]	{ yylval.bit_num = digit_value(yytext[2]); return MB; }

		/* Register banks (RB0 and RB1). */
(RB|rb)[01]	{ yylval.bit_num = digit_value(yytext[2]); return RB; }

		/* Decimal constant. */
{DIGIT}+	{ yylval.ival = atoi(yytext); return INT_VALUE; }

		/* Hex constant. */
{DIGIT}{HEX}*[Hh] { yylval.ival = hex_const_value(yytext); return INT_VALUE; }

		/* Hex constant. */
0[Xx]{HEX}+	{ yylval.ival = hex_const_value(yytext); return INT_VALUE; }

		/* Hex constant. This is the numeric format produced by the disassembler. */
${HEX}+		{ sscanf(yytext+1, "%x", &yylval.ival); return INT_VALUE; }

		/* Current address */
"$"		{ yylval.ival = cur_offset; return INT_VALUE; }

		/* Binary constant. */
0[Bb]{ZEROONE}+ { yylval.ival = hex_const_value(yytext); return INT_VALUE; }

		/* Binary constant. */
\%{ZEROONE}+	{ yylval.ival = hex_const_value(yytext); return INT_VALUE; }

		/* Left shift. */
"<<"		{ return LSHIFT; }

		/* Right shift. */
">>"		{ return RSHIFT; }

		/* Compare equal */
"=="		{ return EQUAL; }

		/* Compare different */
"!="		{ return DIFF; }

		/* Compare less than */
"<="		{ return LESSTHAN; }

		/* Compare greater than */
">="		{ return GREATERTHAN; }

		/* Logical-AND */
"&&"		{ return LAND; }

		/* Logical-OR */
"||"		{ return LOR; }

		/* Modulo. */
\%[ \t]+	{ return MOD; }

		/* Single-character tokens. */
{SINGLE}	{ return yytext[0]; }

		/* Equate keyword. */
"."(EQU|equ)	{ return EQU; }

		/* Set symbol keyword. */
"."(SET|set)	{ return SET; }

		/* Define symbol keyword. */
"."(DEFINE|define)	{ return SET; }

		/* .org directive  */
"."(ORG|org)	{ return ORG; }

		/* .db directive */
"."(DB|db)	{ return DB; }

		/* .dw directive */
"."(DW|dw)	{ return DW; }

		/* .dbr directive */
"."(DBR|dbr)	{ return DBR; }

		/* .incbin directive */
"."(INCBIN|incbin)	{ return INCBIN; }

		/* .end directive */
"."(END|end)	{ return eof_lex(); }

		/* .exit directive */
"."(EXIT|exit)	{ return TEOF; }

			/* .message directive */
"."(MESSAGE|message)	{ return MESSAGE; }

			/* .warning directive */
"."(WARNING|warning)	{ return WARNING; }

			/* .error directive */
"."(ERROR|error)	{ return ERROR; }

			/* .if directive */
"."(IF|if)		{ return IF; }

			/* .ifdef directive */
"."(IFDEF|ifdef)	{ return IFDEF; }

			/* .ifndef directive */
"."(IFNDEF|ifndef)	{ return IFNDEF; }

			/* .ifset directive */
"."(IFSET|ifset)	{ return IFDEF; }

			/* .ifnset directive */
"."(IFNSET|ifnset)	{ return IFNDEF; }

			/* .else directive */
"."(ELSE|else)		{ return if_else_lex(); }

			/* .endif directive */
"."(ENDIF|endif)	{ return if_pop_lex(); }

		/* Instruction mnemonics. */
(ADD|add)	{ return ADD; }
(ADDC|addc)	{ return ADDC; }
(ANL|anl)	{ return ANL; }
(ANLD|anld)	{ return ANLD; }
(CALL|call)	{ return CALL; }
(CLR|clr)	{ return CLR; }
(CPL|cpl)	{ return CPL; }
(DA|da)		{ return DA; }
(DEC|dec)	{ return DEC; }
(DIS|dis)	{ return DIS; }
(DJNZ|djnz)	{ return DJNZ; }
(EN|en)		{ return EN; }
(ENT0|ent0)	{ return ENT0; }
(IN|in)		{ return IN; }
(INC|inc)	{ return INC; }
(INS|ins)	{ return INS; }
(JB|jb)[0-7]	{ yylval.bit_num = digit_value(yytext[2]); return JB; }
(JC|jc)		{ return JC; }
(JF0|jf0)	{ return JF0; }
(JF1|jf1)	{ return JF1; }
(JMP|jmp)	{ return JMP; }
(JMPP|jmpp)	{ return JMPP; }
(JNC|jnc)	{ return JNC; }
(JNI|jni)	{ return JNI; }
(JNT0|jnt0)	{ return JNT0; }
(JNT1|jnt1)	{ return JNT1; }
(JNZ|jnz)	{ return JNZ; }
(JTF|jtf)	{ return JTF; }
(JT0|jt0)	{ return JT0; }
(JT1|jt1)	{ return JT1; }
(JZ|jz)		{ return JZ; }
(MOV|mov)	{ return MOV; }
(MOVD|movd)	{ return MOVD; }
(MOVP|movp)	{ return MOVP; }
(MOVP3|movp3)	{ return MOVP3; }
(MOVX|movx)	{ return MOVX; }
(NOP|nop)	{ return NOP; }
(ORL|orl)	{ return ORL; }
(ORLD|orld)	{ return ORLD; }
(OUTL|outl)	{ return OUTL; }
(RET|ret)	{ return RET; }
(RETR|retr)	{ return RETR; }
(RL|rl)		{ return RL; }
(RLC|rlc)	{ return RLC; }
(RR|rr)		{ return RR; }
(RRC|rrc)	{ return RRC; }
(SEL|sel)	{ return SEL; }
(STOP|stop)	{ return STOP; }
(STRT|strt)	{ return STRT; }
(SWAP|swap)	{ return SWAP; }
(XCH|xch)	{ return XCH; }
(XCHD|xchd)	{ return XCHD; }
(XRL|xrl)	{ return XRL; }

		/* Identifier. */
\.?{IDSTART}{IDCHAR}* { yylval.identifier = dup_str(yytext); return IDENTIFIER; }
		/* string literal */
{STRING}	{ yylval.identifier = dup_str(yytext); return STRING_LITERAL; }

.		{ err_printf("[%s] Line %d: Unexpected character '%c'\n", cur_file, lex_src_line, yytext[0]); }

		/* .include directive */
{INCLUDE}	{ return include_lex(); }

<<EOF>>		{ return eof_lex(); }

		/*
		 * IF ignore state
		 */
				/* Skip comments. */
<ifskip>";".*"\n"		{ ++lex_src_line; return EOL; }
<ifskip>";".*			{ ++lex_src_line; return EOL; }

				/* Skip horizontal whitespace. */
<ifskip>{HWS}+			{ }

				/* End of line character. */
<ifskip>"\n"			{ ++lex_src_line; return EOL; }

				/* .if directive */
<ifskip>{IFD}			{ return if_push_lex(0); }

				/* .ifdef directive */
<ifskip>{IFDEFD}		{ return if_push_lex(0); }

				/* .ifndef directive */
<ifskip>{IFNDEFD}		{ return if_push_lex(0); }

				/* .ifset directive */
<ifskip>{IFSETD}		{ return if_push_lex(0); }

				/* .ifnset directive */
<ifskip>{IFNSETD}		{ return if_push_lex(0); }

				/* .else directive */
<ifskip>"."(ELSE|else)		{ return if_else_lex(); }

				/* .endif directive */
<ifskip>"."(ENDIF|endif)	{ return if_pop_lex(); }

				/* Anything else... */
<ifskip>.			{ return EOL; }

%%

#if 0
void yyerror(const char *msg)
{
	printf("error: %s\n", msg);
	exit(1);
}

int main(int argc, char *argv)
{
	int t;
	while ((t = yylex()) > 0) {
		printf("%s (%d)\n", yytext, t);
	}
	return 0;
}
#endif

int include_lex(void)
{
	char *fname, *p = NULL;

	if (include_stack_ptr >= MAX_INCLUDE_DEPTH) {
		err_printf("[%s] Line %d: Includes nest too deep.\n", cur_file, lex_src_line);
		return TEOF;
	}

	if ((fname = strchr(yytext, '"')) != NULL) {
		if ((p = strchr(++fname, '"')) != NULL) *p = '\0';

	//on unixoid systems (e.g. Linux, OSX, ...)
	//change back-slashes "\" to forward slashes "/"
#ifdef UNIXOID
	char* fs = strchr(fname, '\\');
	while (fs != NULL)
	{
		*fs = '/';
		fs = strchr(fname, '\\');
	}
#endif

		yyin = fopen(fname, "r");
		if (!yyin) {
			err_printf("[%s] Line %d: Couldn't include file %s\n", cur_file, lex_src_line, fname);
			return TEOF;
		}

 		strcpy(include_stack[include_stack_ptr].filename, cur_file);
 		include_stack[include_stack_ptr].state = YY_CURRENT_BUFFER;
 		include_stack[include_stack_ptr].lineno = lex_src_line;
		include_stack_ptr++;

		lex_src_line = 1;
		cur_file_set(fname);
		yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));
	}

	return EOL;
}

int eof_lex(void)
{
	if (include_stack_ptr <= 0) return TEOF;
	include_stack_ptr--;

	yy_delete_buffer(YY_CURRENT_BUFFER);
	yy_switch_to_buffer(include_stack[include_stack_ptr].state);
	lex_src_line = include_stack[include_stack_ptr].lineno;
	cur_file_set(include_stack[include_stack_ptr].filename);

	return EOL;
}

int if_push_lex(int state)
{
	if (if_stack_ptr >= MAX_IF_DEPTH) {
		err_printf("[%s] Line %d: Conditional directive nest too deep.\n", cur_file, lex_src_line);
		return TEOF;
	}

 	if_stack[if_stack_ptr] = if_run;
	if_stack_ptr++;
	if (!state) if_run = 0;
	else if_run = state;

	if (if_run == 2) BEGIN(INITIAL);
	else BEGIN(ifskip);

	return EOL;
}

int if_pop_lex(void)
{
	if (if_stack_ptr <= 0) {
		err_printf("[%s] Line %d: Conditional directive missing.\n", cur_file, lex_src_line);
		return TEOF;
	}

	if_stack_ptr--;
 	if_run = if_stack[if_stack_ptr];

	if (if_run == 2) BEGIN(INITIAL);
	else BEGIN(ifskip);

	return EOL;
}

int if_else_lex(void)
{
	if (if_run == 1) if_run = 2;
	else if (if_run == 2) if_run = 1;

	if (if_run == 2) BEGIN(INITIAL);
	else BEGIN(ifskip);

	return EOL;
}
