# Makefile for Intel 8048 assembler
#
# Originally done by daveho in 2003
# Multiplatform extensions by sy2002/MEGA in 2013
#
# Compiles under Windows, Linux and OSX:
# Requires GNU make, http://www.gnu.org/software/make
# Requires GNU Compiler Collection http://www.gnu.org/software/gcc
# Requires FLEX and BISON
#
# Linux: Use your package manager to install FLEX and BISON before trying
# to compile.
#
# OSX only: Install XCode plus the optional command line tools (which already
# contain FLEX and BISON).
#
# Windows only: Recommended to compile using MSYS' Bournce Shell, since it already
# contains the GNU Compiler Collection as well as FLEX and BISON.
# Use the graphical installer and install MINGW together with MSYS as described here:
# http://www.mingw.org/wiki/Getting_Started

#if WINTEST != "" then Windows else unixoid (e.g. OSX, Linux, ...)
UNAME := $(shell uname -s)
WINDOWSID := CYGWIN MINGW UWIN
WINTEST = $(strip $(foreach win, $(WINDOWSID), $(findstring $(win), $(UNAME))))
ifneq ($(WINTEST), ) 

EXE = .exe

else

UNIXOID = -DUNIXOID

endif

# Choose debugging or optimization (or both)
#DEBUG = -g
OPT = -O

CC = gcc $(UNIXOID)
CFLAGS = $(DEBUG) $(OPT) -Wall $(EXTRA_CFLAGS)
BISON = bison
FLEX = flex

.SUFFIXES:
.SUFFIXES: .y .l .c .o

.y.o:
	$(BISON) -d $*.y
	$(CC) $(CFLAGS) -c $*.tab.c -o $*.o

.l.o:
	$(FLEX) $*.l
	$(CC) $(CFLAGS) -c $*.yy.c -o $*.o

.c.o:
	$(CC) $(CFLAGS) -c $<

OBJS = parse.o lex.o asm48.o instruction.o expr.o symtab.o pool.o err.o ihex.o getopt.o

EXES = asm48$(EXE) 8039dasm$(EXE)

all : $(EXES)

asm48$(EXE) : $(OBJS)
	$(CC) -o $@ $(OBJS)

8039dasm$(EXE) : 8039dasm.o
	$(CC) -o $@ 8039dasm.o

lex.o : parse.o

expr.o : parse.o


clean :
	rm asm48$(EXE) 8039dasm$(EXE) lex.yy.c *.o parse.tab.*
