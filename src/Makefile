#
#   File:       Makefile
#   Creation:   March 18, 1999 (Thursday, 15:10h)
#   Author:     Copyright (C) 1999 Thomas Jensen
#               tsjensen@stud.informatik.uni-erlangen.de
#   Version:    $Id: Makefile,v 1.11 1999/07/12 18:28:53 tsjensen Exp tsjensen $
#   Format:     GNU make
#   Web Site:   http://home.pages.de/~jensen/boxes/
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
#     Revision 1.9  1999/06/28 12:21:34  tsjensen
#     Added macro SNAPFILE for boxes-SNAP-`date +%Y%m%d`
#     Added target rcstest
#     Now builds optimized code by default
#     Added debug target for non-optimized code w/ debugging info
#
#     Revision 1.8  1999/06/23 19:35:17  tsjensen
#     Added remove.o and generate.o
#     Introduced implicit rule .c.o, simplifying commands for most sources
#
#     Revision 1.7  1999/06/23 12:27:50  tsjensen
#     Added tools.o and shape.o
#     Change in snapshot target directory (download)
#     Added GENSRC, HSSRC, ORIGSRC, ALLSRC, and ALLOBJ macros for more order
#
#     Revision 1.6  1999/06/04 12:15:37  tsjensen
#     Added snap target for automatic snapshot publication
#
#     Revision 1.5  1999/04/04 16:12:40  tsjensen
#     Added rules for new regular expression handling files
#     Added file header
#     Straightened out clean commands
#
#     Revision 1.4  1999/04/03 12:01:01  tsjensen
#     Added build target for optimized compilation
#     Now linking with flex library instead of lex library
#
#     Revision 1.3  1999/03/22 10:57:14  tsjensen
#     Added -D__EXTENSIONS__
#     Compile everything but the parser with -Wall -W
#
#     Revision 1.2  1999/03/19 17:58:14  tsjensen
#     Added boxes.o to clean target
#     Switched language to ANSI C / POSIX_SOURCE
#     Added -Wall -W to boxes core compilation
#
#     Revision 1.1  1999/03/18 15:10:41  tsjensen
#     Initial revision
#____________________________________________________________________________
#============================================================================


LEX      = flex
YACC     = bison
CC       = gcc
CFLAGS   = -ansi -I. -Iregexp $(CFLAGS_ADDTL)
LDFLAGS  = -Lregexp

SNAPFILE = boxes-SNAP-$(shell date +%Y%m%d)
CLOGFILE = $(HOME)/d/public_html/software/boxes/changelogs.html

GENSRC   = parser.c parser.h lex.yy.c
HS_CL    = regexp/regexp.c regexp/regsub.c
HSSRC    = $(HS_CL) regexp/regexp.h regexp/regmagic.h
ORIG_CL  = boxes.c boxes.h lexer.l parser.y tools.c shape.c generate.c remove.c lexer.h
ORIGSRC  = $(ORIG_CL) tools.h shape.h generate.h remove.h config.h
ALL_CL   = $(ORIG_CL) $(HS_CL) Makefile boxes-config
ALLSRC   = $(ORIGSRC) $(HSSRC) $(GENSRC)
ALLOBJ   = parser.o lex.yy.o boxes.o shape.o tools.o generate.o remove.o

build:
	$(MAKE) CFLAGS_ADDTL=-O boxes
	strip boxes
debug:
	$(MAKE) CFLAGS_ADDTL=-g boxes

boxes: $(ALLOBJ) regexp/libregexp.a
	$(CC) $(LDFLAGS) $(ALLOBJ) -o boxes -lregexp

parser.c parser.h: parser.y boxes.h regexp/regexp.h
	$(YACC) -o parser.c -d parser.y

lex.yy.c: lexer.l boxes.h
	$(LEX) -i -t lexer.l > lexer.tmp.c
	echo '#include "config.h"' > lex.yy.c
	cat lexer.tmp.c >> lex.yy.c
	rm lexer.tmp.c

regexp/libregexp.a: regexp/regexp.o regexp/regsub.o
	ar cr regexp/libregexp.a regexp/regexp.o regexp/regsub.o

regexp/regexp.o: regexp/regexp.c regexp/regmagic.h regexp/regexp.h config.h
	gcc -traditional -O -Iregexp -I. -o regexp/regexp.o -c regexp/regexp.c
regexp/regsub.o: regexp/regsub.c regexp/regmagic.h regexp/regexp.h config.h
	gcc -traditional -O -Iregexp -I. -o regexp/regsub.o -c regexp/regsub.c

boxes.o: boxes.c boxes.h regexp/regexp.h shape.h tools.h generate.h remove.h config.h
tools.o: tools.c tools.h boxes.h shape.h config.h
shape.o: shape.c shape.h boxes.h config.h
generate.o: generate.c generate.h boxes.h shape.h tools.h config.h
remove.o: remove.c remove.h boxes.h shape.h tools.h config.h
lex.yy.o: lex.yy.c parser.h tools.h shape.h lexer.h config.h
parser.o: parser.c parser.h tools.h shape.h lexer.h config.h

.c.o:
	$(CC) $(CFLAGS) -Wall -W -c $<

snap: COPYING Makefile boxes-config $(ALLSRC)
	mkdir $(SNAPFILE)
	cp $(ALLSRC) Makefile $(SNAPFILE)
	gtar cfvz $(SNAPFILE).tar.gz $(SNAPFILE)/*
	rm -rf $(SNAPFILE)/
	cp $(SNAPFILE).tar.gz $(HOME)/d/public_html/software/boxes/download/
	rm -f $(HOME)/d/public_html/software/boxes/download/current-SNAP.tar.gz
	(cd $(HOME)/d/public_html/software/boxes/download/; ln -s $(SNAPFILE).tar.gz current-SNAP.tar.gz)

rcstest:
	-for i in $(ALLSRC) Makefile boxes-config ; do rcsdiff $$i ; done

logpage: $(ALL_CL)
	doc/create_changelog.pl $(ALL_CL) > $(CLOGFILE)

clean:
	rm -f $(ALLOBJ)
	rm -f core regexp/libregexp.a boxes

love:
	@echo "Not in front of the kids, honey!"



#EOF
