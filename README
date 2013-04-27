Asm48 - an 8048 cross assembler

Original Version 0.1
Version 0.1pre
January 29, 2003
by David Hovemeyer <daveho@cs.umd.edu>

Enhanced Version 0.4.1
April 14, 2013
by Adventure Vision Development Team of MEGA <http://www.adventurevision.net>
main enhancements by JustBurn; OSX and multiplatform  by sy2002

Version Status: 0.4.1 is stable

===========
What is it?
===========

Asm48 is an assembler for the Intel 8048 family of microcontrollers.
It is free software; you may use, modify, and redistribute it according to
the terms specified in the various source files.

In case you don't know, the Intel 8048 family was one of the very
first commercially successful microcontroller product lines.  You can
find them in an enormous range of devices, such as cars, computer
keyboards, industrial equipment, etc.  Members of the 8048 family include
the 8021, 8035, 8038, 8039, 8040, 8048, 8049, 8050, 8748, and 8749.
(I believe the 8041 and 8042 are also in the same general family,
although I think the instruction encodings are somewhat different, and
thus I wouldn't necessarily expect this assembler to support those chips.)

You can find the 8048 in various game systems as the Magnavox Odyssey^2 and
the Entex Adventure Vision and certain arcade video games.

================
Acknowledgements
================

This product includes software developed by the University of
California, Berkeley and its contributors.

The file "ihex.c" was written by Paul Stoffregen.

The file "8039dasm.c" was written by Michael Cuddy,
and was shamelessly stolen from the MAME/MESS project.

============
Installation
============

The multiplatform package "asm48-0.4.1-executables-and-src-win-linux-osx.zip"
contains executables for Windows, Linux and OSX, so just unzip, choose the
right subdirectory and start working.

=========
Compiling
=========

The sources and the makefile are tested to compile under Windows, Linux
and OSX - normally without requiring any changes, since the makefile
autodetects the platform.

You need:

* GNU make, http://www.gnu.org/software/make
* GNU Compiler Collection http://www.gnu.org/software/gcc
* FLEX and BISON

Windows:

Recommended to compile using MSYS' Bournce Shell, since it already
contains the GNU Compiler Collection as well as FLEX and BISON.
Use the graphical installer and install MINGW together with MSYS as described here:
http://www.mingw.org/wiki/Getting_Started

Linux:

Use your package manager to install FLEX and BISON.

OSX:

Install XCode plus the optional command line tools, which already
contain FLEX and BISON.


Just run the command "make".  There are a couple configurable options in
the Makefile you might want to change.  I assume you have flex and bison
available on your system.  You should be able to use other lex and yacc
variants, although you might need to change the Makefile a bit.

The resulting executables are called "asm48" and "8039dasm".  They are the
assembler and disassembler, respectively.

===========
Usage notes
===========

To a large extent, I've tried to follow the assembly syntax described in
the Intel 8048 user manual.  However, I've made some gratuitous changes:

  - Lower case is accepted for instruction mnemonics, register names, etc.
  - EQU directives are written differently
  - The '#' prefix indicating an immediate value may be specified or
    omitted at your whim
  - Hex constants may be specified using the standard C notation
    (0x1A) rather than the Intel notation (1Ah)

These directives are allowed:

  .equ IDENTIFIER, VALUE
  .org VALUE
  .org VALUE, FILL-VALUE
  .db VALUE
  .db VALUE, VALUE, ...
  .db accepts forward labels
  .dbr same as .db but with reverse bit order
  .dw 16bit VALUE(S)  
  
  .include "FILENAME" for including assembly files
  .incbin "FILENAME" for including binary files
    
  conditional directives .if, .ifdef, .ifndef, .else, .endif
  
  message directives: .message, .warning, .error
  
  .exit for end-of-assembly
  .set for conditionals

  $ symbol for current offset, e.g. for doing things like  "jmp $ + 3"
  % for modulo, e.g. for doing things like ".equ 50 % 7"
  ~ for flipping bits
  ==, !=, =, &&, || for boolean logics

  Instruction immediates now also accept 8-bit negative values
  
  expression unary <: get lo byte of word; > get hi byte  
  
The special symbol ".here" may be used to refer to the address of the
current instruction.

Values may be expressions using the standard arithmetic operators
("+", "-", "*", "/"), as well as left and right shifts ("<<" and ">>"),
bitwise "&" (and) and "|" (or), and bitwise complement (the unary "~" operator).
All expressions are evaluated using host platform ints.  Operator precedence
follows the equivalent C operators.

=================================================
Dave's Original 2003 Disclaimers and contact info
=================================================

If you actually use this assembler, please send me an email at
<daveho@cs.umd.edu>.  I wrote it on a lark[*], and given how crufty
and ancient the 8048 is, I'd be surprised and amused to find out that
someone else was interested in writing code for it.  I will also
happily accept bug fixes and new features.

There are almost certainly bugs in this software, so use at your own risk.
I accept absolutely no responsibility for any consequences resulting
from the use of this software.

If you do decide to use the assembler, you are STRONGLY ADVISED to run
your assembled code through the included disassembler to check that the
assembler output looks reasonable.

Have fun!

[*] It was really hard to get it to stand still.

======================================================================
Some 2013 final notes by the Adventure Vision Development Team of MEGA
======================================================================

Dave did a great job. His version 0.1 of the assembler from 2003
already worked like a charm. We just added plenty of new directives,
and features and fixed some minor bugs to make the assembler usable
for larger projects.

Now, our version 0.4.1 from 2013 can be considered as stable and as
truely multiplatform, since it runs and compiles on Windows, Linux
and OSX.

The 8048 is a really cool piece of hardware, and it is fascinating
what you can do with such a far more than 30 years old CPU. If you
are into the demo scene, have a look at our 8048 Adventure Vision
demo at <http://www.pouet.net/prod.php?which=61201> or visit the
"Making Of" Homepage at <http://www.adventurevision.net/>. You'll
also find an Adventure Vision 8048 Emulator there.
