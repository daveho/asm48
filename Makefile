# Makefile for Intel 8048 assembler.
# Requires GNU make, http://www.gnu.org/software/make

# Choose debugging or optimization (or both)
#DEBUG = -g
OPT = -O

# Compiling on Windows:
# We use cygwin with the -mno-cygwin option, so the resulting
# executable will run without the cygwin DLL.  MINGW_LIB_DIR
# should contain the extra mingw headers and libraries: see
#   http://www.nanotech.wisc.edu/~khan/software/gnu-win32/mno-cygwin-howto.txt
ifeq ($(OS),Windows_NT)
MINGW_LIB_DIR := /cygdrive/w/mingw-lib
MINGW_FLAG := -mno-cygwin
EXTRA_CFLAGS := $(MINGW_FLAG) -I$(MINGW_LIB_DIR)/include $(EXTRA_CFLAGS)
LDFLAGS := -L$(MINGW_LIB_DIR)/lib
EXE := .exe
NEED_GETOPT := yes
endif

# If NEED_GETOPT is set to "yes", then compile the included implementation
# of getopt().
ifeq ($(NEED_GETOPT),yes)
EXTRA_OBJS := getopt.o
else
EXTRA_CFLAGS := $(EXTRA_CFLAGS) -DHAVE_GETOPT
endif

CC := gcc
CFLAGS := $(DEBUG) $(OPT) -Wall $(EXTRA_CFLAGS)
BISON := bison
FLEX := flex

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
	$(CC) $(MINGW_FLAG) -o $@ $(OBJS) $(LDFLAGS)

8039dasm$(EXE) : 8039dasm.o
	$(CC) $(MINGW_FLAG) -o $@ 8039dasm.o $(LDFLAGS)

test : $(EXES)
	bash runtest

lex.o : parse.o

expr.o : parse.o

VERSION := 0.0

ifeq ($(OS),Windows_NT)
PLATFORM := Windows-i686
else
PLATFORM := $(shell uname -s)-$(shell uname -m)
endif
RELNAME := asm48-$(VERSION)-$(PLATFORM)

ifeq ($(OS),Windows_NT)
DISTFILE_GZ := $(RELNAME).zip
else
DISTFILE := $(RELNAME).tar
DISTFILE_GZ := $(DISTFILE).gz
endif

dist : $(DISTFILE_GZ)

$(DISTFILE_GZ) : $(EXES)
	rm -rf $(RELNAME)
	mkdir $(RELNAME)
	cp $(EXES) README $(RELNAME)
ifeq ($(OS),Windows_NT)
	zip -9r $@ $(RELNAME)
else
	tar cf $(DISTFILE) $(RELNAME)
	gzip --best $(DISTFILE)
	rm -rf $(RELNAME)
endif

clean :
	rm -f *.o *.core $(EXES) parse.tab.c parse.tab.h lex.yy.c

distclean : clean
	rm -rf $(RELNAME) $(DISTFILE) $(DISTFILE_GZ)
