---
layout: default
title: "Asm48 - An 8048 Cross Assembler"
---

News
====

-   **14 Dec 2020**: Project migrated to [Github](https://github.com).  The project URL
    is <https://github.com/daveho/asm48>.
-   **14 Apr 2013**: [Version 0.4.1](https://github.com/daveho/asm48/releases/tag/REL_0_4_1)
    binaries and sources available for Windows, Linux and OSX.
    Version 0.4.1 is stable and features conditionals, includes and
    plenty of other new directives. For details, have a look at the
    README.  This 2013 version 0.4.1 of Dave\'s 2003 version 0.0 was done by the
    [Adventure Vision Development Team of MEGA](http://www.adventurevision.net).
-   **4 Feb 2003**: [Version 0.0 binaries available for Windows](https://github.com/daveho/asm48/releases/tag/REL_0_0).
-   **29 Jan 2003**: [Version 0.0](https://github.com/daveho/asm48/releases/tag/REL_0_0)
    released with source and binaries for Linux.
-   **28 Jan 2003**: Fixed some bugs.  Asm48 can now reassemble a file
    produced with the disassembler, producing output identical to the
    original.
-   **24 Jan 2003**: Project created at [SourceForge](http://sourceforge.net/).

Info
====

Asm48 is a cross assembler for the ubiquitous Intel™ 8048 family of
microcontrollers.  The assembler is at an early stage of development. 
However, the major functionality is in place, and with some testing it
will be considered stable very soon.  If you feel slightly adventurous
and don\'t mind fixing some bugs, you could probably use Asm48 right
now.

Download
========

See the [Releases]() for downloadable files.  However, you're probably
better using git to clone [the repository](https://github.com/daveho/asm48)
and then building from source.

Getting the source code
=======================

See the [Github project page](https://github.com/daveho/asm48).
You can clone the repository using the command

```bash
git clone https://github.com/daveho/asm48.git
```

Compiling and Installing
========================

Asm48 is written in standard C, and uses
[flex](http://www.gnu.org/software/flex) and
[bison](http://www.gnu.org/software/bison).  You also will need [GNU
Make](http://www.gnu.org/software/make) (although the Makefile could
easily be modified for other make variants).  Most Unix-like systems
(such as Linux and FreeBSD) and Windows machines running
[Cygwin](http://cygwin.com/) should be able to build Asm48. See the
[README](https://github.com/daveho/asm48/blob/master/README)
for more information on how to build.

Contact
=======

Please send bug reports, flames, etc. to <mailto:david.hovemeyer@gmail.com>
