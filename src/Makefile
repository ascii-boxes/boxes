#
#   File:        Makefile
#   Creation:    March 18, 1999 (Thursday, 15:10h)
#   Author:      Thomas Jensen
#                tsjensen@stud.informatik.uni-erlangen.de
#   Version:     $Id: Makefile,v 1.4 1999/04/03 12:01:01 tsjensen Exp tsjensen $
#   Format:      GNU make
#   Web Site:    http://home.pages.de/~jensen/boxes/
#   Platforms:   sparc/Solaris 2.6
#   Purpose:     Makefile for boxes, the box drawing program
#   Remarks:     ---
#
#   Revision History:
#
#     $Log: Makefile,v $
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


LEX     = flex
YACC    = yacc
CC      = gcc
CFLAGS  = -ansi -D_POSIX_C_SOURCE -D__EXTENSIONS__ -I. $(CFLAGS_ADDTL)
LDFLAGS = -L. -L/local/GNU/lib


boxes: y.tab.o lex.yy.o boxes.o libregexp.a
	$(CC) $(LDFLAGS) -o boxes y.tab.o lex.yy.o boxes.o -lregexp -lfl

build:
	$(MAKE) CFLAGS_ADDTL=-O boxes

libregexp.a: regexp.o regsub.o
	ar cr libregexp.a regexp.o regsub.o

regexp.o: regexp.c regmagic.h regexp.h
	gcc -traditional -O -I. -c regexp.c
regsub.o: regsub.c regmagic.h regexp.h
	gcc -traditional -O -I. -c regsub.c

lex.yy.o: lex.yy.c y.tab.h
	$(CC) $(CFLAGS) -DYY_NO_UNPUT -Wall -W -c lex.yy.c

y.tab.o: y.tab.c y.tab.h

boxes.o: boxes.c boxes.h regexp.h
	$(CC) $(CFLAGS) -Wall -W -c boxes.c

y.tab.c y.tab.h: parser.y boxes.h regexp.h
	$(YACC) -d parser.y

lex.yy.c: lexer.l boxes.h
	$(LEX) -i lexer.l

clean:
	rm -f lex.yy.c y.tab.c y.tab.h
	rm -f *.o core
	rm -f libregexp.a boxes

love:
	@echo "Not in front of the kids, honey!"



#EOF
