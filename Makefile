# Makefile for Intel 8048 assembler.
# Requires GNU make, http://www.gnu.org/software/make

# Choose debugging or optimization (or both)
#DEBUG = -g
OPT = -O

# Comment out on Unix
#EXE = .exe

# Uncomment this if you need getopt().
# This is likely if you're on a non-Unix system.
# It should be safe to enable on any system, however.
#NEED_GETOPT = yes

CC = gcc
CFLAGS = $(DEBUG) $(OPT) -Wall $(EXTRA_CFLAGS)
BISON = bison
FLEX = flex

ifeq ($(NEED_GETOPT),yes)
EXTRA_OBJS = getopt.o
else
EXTRA_CFLAGS = -DHAVE_GETOPT
endif

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

OBJS = parse.o lex.o asm48.o instruction.o expr.o symtab.o pool.o err.o ihex.o $(EXTRA_OBJS)

EXES = asm48$(EXE) 8039dasm$(EXE)

all : $(EXES)

asm48$(EXE) : $(OBJS)
	$(CC) -o $@ $(OBJS) -lfl

8039dasm$(EXE) : 8039dasm.o
	$(CC) -o $@ 8039dasm.o

test : $(EXES)
	./runtest

lex.o : parse.o

expr.o : parse.o

VERSION := 0.0
PLATFORM := $(shell uname -s)-$(shell uname -m)
RELNAME := asm48-$(VERSION)-$(PLATFORM)
DISTFILE := $(RELNAME).tar
DISTFILE_GZ := $(DISTFILE).gz

dist : $(DISTFILE_GZ)

$(DISTFILE_GZ) : $(EXES)
	rm -rf $(RELNAME)
	mkdir $(RELNAME)
	cp $(EXES) README $(RELNAME)
	tar cf $(DISTFILE) $(RELNAME)
	gzip --best $(DISTFILE)
	rm -rf $(RELNAME)

clean :
	rm -f *.o *.core $(EXES) parse.tab.c parse.tab.h lex.yy.c

distclean : clean
	rm -rf $(RELNAME) $(DISTFILE) $(DISTFILE_GZ)
