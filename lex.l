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
	} else {
		value = 0;
		for (;;) {
			int ch = tolower(*str++);
			if (ch == 'h')
				break;
			value <<= 4;
			if (isdigit(ch))
				value += (ch - '0');
			else {
				assert(ch >= 'a' && ch <= 'f');
				value += (10 + (ch - 'a'));
			}
		}
	}
	return value;
}
%}

DIGIT		[0-9]
HEX		({DIGIT}|[A-Fa-f])
IDSTART		[A-Za-z_]
IDCHAR		({IDSTART}|{DIGIT})
HWS		[ \t\r]
SINGLE		[@,:()+\-\*/#\&\|]

%%

		/* Skip comments. */
";".*"\n"	{ ++lex_src_line; }

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

		/* Single-character tokens. */
{SINGLE}	{ return yytext[0]; }

		/* Left shift. */
"<<"		{ return LSHIFT; }

		/* Right shift. */
">>"		{ return RSHIFT; }

		/* Equate keyword. */
"."(EQU|equ)	{ return EQU; }

		/* .org directive  */
"."(ORG|org)	{ return ORG; }

		/* .db directive */
"."(DB|db)	{ return DB; }

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

.		{ err_printf("Unexpected character '%c' at line %d\n", yytext[0], lex_src_line); }


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
