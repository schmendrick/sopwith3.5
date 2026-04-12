This archive contains the bits of source code that I, Andrew Jenner, have been
messing about with. Dave Clark's unmodified source is in the archive
dlcsrc.zip. Questions about the code in this archive should be directed to me,
andrew@reenigne.org

Andrew Jenner stuff:
  General stuff:
    README.TXT   - This file.
    LICENSE.TXT  - The Sopwith license agreement.
    MAKEFILE     - Make file. Works with Borland make. Doesn't work with GNU
                   make for reasons which escape me (please tell me if you
                   know). Don't know about other flavours of make.
    RESPONSE.RSP - Linker command line (response) file to link Sopwith. Works
                   with Borland TLINK 2.0.
    RESPONS2.RSP - Linker command line (response) file to link Sopwith 2. Works
                   with Borland TLINK 2.0.
    SOPWITH.PRJ  - Turbo C 2.01 project file for Sopwith.
    SOPWITH2.PRJ - Turbo C 2.01 project file for Sopwith 2.

  Code:
    SOPWITH.C    - Some of the reverse engineered code for Sopwith 1.
    SAME.C       - The rest of the reverse engineered code for Sopwith 1 (the
                   routines which are the same in Sopwith 2).
    SOPWITH2.C   - Reverse engineered code for Sopwith 2. Now also includes
                   7.F15 code.
    DEF.H        - Generic header file
    SOPASM.ASM   - Assembler support routines, common to all three versions.
                   Works with A86.
    SOPASM.H     - Header file declaring the functions in SOPASM.ASM.
    GROUND.C     - Landscape height field, common to all three versions.

  MKG:
    SOPWITH.SPR  - All the raw graphics data for Sopwith
    MKG.TXT      - Data file for MKG describing what's in SOPWITH.SPR.
    MKG.CPP      - C++ program for converting the graphics data in SOPWITH.SPR
                   to the files SPRITES.C and COLL.C which are linked in to the
                   final executable. Works with the GNU C compiler.
