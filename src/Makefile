#
#   File:       Makefile
#   Creation:   March 18, 1999 (Thursday, 15:10h)
#   Author:     Copyright (C) 1999 Thomas Jensen <boxes@thomasjensen.com>
#   Version:    $Id: Makefile,v 1.17 2000-03-17 15:51:43-08 tsjensen Exp tsjensen $
#   Format:     GNU make
#   Web Site:   http://boxes.thomasjensen.com/
#   Platforms:  sparc/Solaris 2.6 and others
#   Purpose:    Makefile for boxes, the box drawing program
# 
#   Remarks:  o This program is free software; you can redistribute it and/or
#               modify it under the terms of the GNU General Public License as
#               published by the Free Software Foundation; either version 2 of
#               the License, or (at your option) any later version.
#             o This program is distributed in the hope that it will be useful,
#               but WITHOUT ANY WARRANTY; without even the implied warranty of
#               MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#               GNU General Public License for more details.
#             o You should have received a copy of the GNU General Public
#               License along with this program; if not, write to the Free
#               Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#               MA 02111-1307  USA
#
#   Revision History:
#
#     $Log: Makefile,v $
#     Revision 1.17  2000-03-17 15:51:43-08  tsjensen
#     Added Makefile.Win32 and misc/* to snap generation commands
#
#     Revision 1.16  1999/11/08 10:55:13  tsjensen
#     Moved config.h into ORIG_HDRCL group, so it appears in the change log Web
#     page
#
#     Revision 1.15  1999/08/21 23:35:44  tsjensen
#     boxes.h is now generated from boxes.h.in by top level Makefile
#     Added -Wall -W to CFLAGS and removed .c.o rule
#
#     Revision 1.14  1999/08/21 16:10:41  tsjensen
#     Removed case insensitivity option (-i) from flex call (-> lexer.l)
#
#     Revision 1.13  1999/08/13 23:55:03  tsjensen
#     Given regexp stuff its own Makefile in regexp.
#     Major Makefile overhaul, maily implementation of new directory structure
#     and consolidation of file macros
#     Use more GNU make features
#
#     Revision 1.12  1999/07/20 18:49:58  tsjensen
#     Added GNU GPL disclaimer
#     Now evaluating date command from SNAPFILE only once
#     Changed order of files in snapshot archive (who cares)
#
#     Revision 1.11  1999/07/12 18:28:53  tsjensen
#     Removed --yacc option from bison call
#     Added logpage target to generate the chronologically sorted global change
#     log in HTML format. This requires a Perl script (create_changelog.pl).
#     Added config.h to dependencies and stuff
#     Removed special targets for yacc and lex files. Thanks to Andreas Heiduk,
#     it is now possible to compile all files with -W -Wall and no extra
#     defines.
#     Moved regexp library to a subdirectory regexp
#     Some modifications on the source file macros which now differentiate
#     between files which are listed in the HTML change log and files which
#     aren't.
#
#     Revision 1.10  1999/07/02 11:50:35  tsjensen
#     Added parser.h header file for lexer-parser communication
#     Moved from yacc to GNU "bison --yacc"
#     No longer linking with flex library
#     Activated -Wall -W for parser.y
#     Removed -DYY_NO_UNPUT from lexer compilation (now done via file option)
#
#     Revision 1.1  1999/03/18 15:10:41  tsjensen
#     Initial revision
#____________________________________________________________________________
#============================================================================


LEX        = flex
YACC       = bison
CC         = gcc
CFLAGS     = -ansi -I. -Iregexp -Wall -W $(CFLAGS_ADDTL)
LDFLAGS    = -Lregexp

GEN_HDR    = parser.h boxes.h
GEN_SRC    = parser.c lex.yy.c
GEN_FILES  = $(GEN_SRC) $(GEN_HDR)
ORIG_HDRCL = boxes.h.in config.h
ORIG_HDR   = $(ORIG_HDRCL) lexer.h tools.h shape.h generate.h remove.h
ORIG_GEN   = lexer.l parser.y
ORIG_NORM  = boxes.c tools.c shape.c generate.c remove.c
ORIG_SRC   = $(ORIG_GEN) $(ORIG_NORM)
ORIG_FILES = $(ORIG_SRC) $(ORIG_HDR)
OTH_FILES  = Makefile

ALL_CL     = $(ORIG_SRC) $(ORIG_HDRCL) $(OTH_FILES)
ALL_FILES  = $(ORIG_FILES) $(GEN_FILES) $(OTH_FILES)
ALL_OBJ    = $(GEN_SRC:.c=.o) $(ORIG_NORM:.c=.o)


build:
	$(MAKE) CFLAGS_ADDTL=-O boxes
	strip boxes
debug:
	$(MAKE) CFLAGS_ADDTL=-g boxes

boxes: $(ALL_OBJ)
	$(MAKE) -C regexp CC=$(CC) libregexp.a
	$(CC) $(LDFLAGS) $(ALL_OBJ) -o boxes -lregexp

parser.c parser.h: parser.y boxes.h regexp/regexp.h
	$(YACC) -o parser.c -d parser.y

lex.yy.c: lexer.l boxes.h
	$(LEX) -t lexer.l > lexer.tmp.c
	echo '#include "config.h"' > lex.yy.c
	cat lexer.tmp.c >> lex.yy.c
	rm lexer.tmp.c


boxes.o: boxes.c boxes.h regexp/regexp.h shape.h tools.h generate.h remove.h config.h
tools.o: tools.c tools.h boxes.h shape.h config.h
shape.o: shape.c shape.h boxes.h config.h tools.h
generate.o: generate.c generate.h boxes.h shape.h tools.h config.h
remove.o: remove.c remove.h boxes.h shape.h tools.h config.h
lex.yy.o: lex.yy.c parser.h tools.h shape.h lexer.h config.h
parser.o: parser.c parser.h tools.h shape.h lexer.h config.h


snap: $(ALL_FILES)
	if [ -z "$(SNAPFILE)" ] ; then exit 1 ; fi
	mkdir $(SNAPFILE)/src
	cp $(ALL_FILES) $(SNAPFILE)/src
	cp Makefile.Win32 $(SNAPFILE)/src
	mkdir $(SNAPFILE)/src/misc
	cp misc/getopt.c misc/getopt.h $(SNAPFILE)/src/misc
	touch $(patsubst %,$(SNAPFILE)/src/%,$(GEN_FILES))
	$(MAKE) -C regexp SNAPFILE=../$(SNAPFILE) snap

rcstest:
	-for i in $(ORIG_FILES) $(OTH_FILES) ; do rcsdiff $$i ; done
	$(MAKE) -C regexp rcstest

logpage: $(ALL_CL)
	@echo $(ALL_CL) $(shell $(MAKE) -C regexp -s logpage)

clean:
	rm -f $(ALL_OBJ)
	rm -f $(GEN_FILES)
	rm -f core boxes
	$(MAKE) -C regexp clean

love:
	@echo "Not in front of the kids, honey!"



#EOF
