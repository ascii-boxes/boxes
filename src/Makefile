#
#   File:        Makefile
#   Creation:    March 18, 1999 (Thursday, 15:10h)
#   Author:      Thomas Jensen
#                tsjensen@stud.informatik.uni-erlangen.de
#   Version:     $Id: Makefile,v 1.9 1999/06/28 12:21:34 tsjensen Exp tsjensen $
#   Format:      GNU make
#   Web Site:    http://home.pages.de/~jensen/boxes/
#   Platforms:   sparc/Solaris 2.6
#   Purpose:     Makefile for boxes, the box drawing program
#   Remarks:     ---
#
#   Revision History:
#
#     $Log: Makefile,v $
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
YACC     = bison --yacc
CC       = gcc
CFLAGS   = -ansi -D_POSIX_C_SOURCE -D__EXTENSIONS__ -I. $(CFLAGS_ADDTL)
LDFLAGS  = -L.

SNAPFILE = boxes-SNAP-`date +%Y%m%d`

GENSRC   = y.tab.c y.tab.h lex.yy.c
HSSRC    = regexp.c regexp.h regmagic.h regsub.c
ORIGSRC  = boxes.c boxes.h lexer.l parser.y tools.c tools.h shape.c shape.h generate.c generate.h remove.h remove.c parser.h
ALLSRC   = $(ORIGSRC) $(HSSRC) $(GENSRC)
ALLOBJ   = y.tab.o lex.yy.o boxes.o shape.o tools.o generate.o remove.o

build:
	$(MAKE) CFLAGS_ADDTL=-O boxes
	strip boxes
debug:
	$(MAKE) CFLAGS_ADDTL=-g boxes

boxes: $(ALLOBJ) libregexp.a
	$(CC) $(LDFLAGS) $(ALLOBJ) -o boxes -lregexp

y.tab.c y.tab.h: parser.y boxes.h regexp.h
	$(YACC) -d parser.y

lex.yy.c: lexer.l boxes.h
	$(LEX) -i lexer.l

libregexp.a: regexp.o regsub.o
	ar cr libregexp.a regexp.o regsub.o

regexp.o: regexp.c regmagic.h regexp.h
	gcc -traditional -O -I. -c regexp.c
regsub.o: regsub.c regmagic.h regexp.h
	gcc -traditional -O -I. -c regsub.c

boxes.o: boxes.c boxes.h regexp.h shape.h tools.h generate.h remove.h
tools.o: tools.c tools.h boxes.h shape.h
shape.o: shape.c shape.h boxes.h
generate.o: generate.c generate.h boxes.h shape.h tools.h
remove.o: remove.c remove.h boxes.h shape.h tools.h
lex.yy.o: lex.yy.c y.tab.h tools.h shape.h parser.h
y.tab.o: y.tab.c y.tab.h tools.h shape.h parser.h

.c.o:
	$(CC) $(CFLAGS) -Wall -W -c $<

snap: $(ALLSRC) Makefile
	mkdir $(SNAPFILE)
	cp $(ALLSRC) Makefile $(SNAPFILE)
	gtar cfvz $(SNAPFILE).tar.gz $(SNAPFILE)/*
	rm -rf $(SNAPFILE)/
	cp $(SNAPFILE).tar.gz $(HOME)/d/public_html/software/boxes/download/
	rm -f $(HOME)/d/public_html/software/boxes/download/current-SNAP.tar.gz
	(cd $(HOME)/d/public_html/software/boxes/download/; ln -s $(SNAPFILE).tar.gz current-SNAP.tar.gz)

rcstest:
	-for i in $(ALLSRC) Makefile boxes-config ; do rcsdiff $$i ; done

clean:
	rm -f lex.yy.c y.tab.c y.tab.h
	rm -f *.o core
	rm -f libregexp.a boxes

love:
	@echo "Not in front of the kids, honey!"



#EOF
