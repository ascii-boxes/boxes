# Makefile for boxes on Solaris 2.6/sparc

LEX    = flex
YACC   = yacc
CC     = gcc
CFLAGS = -ansi -D_POSIX_C_SOURCE -D__EXTENSIONS__ # -O


boxes: y.tab.o lex.yy.o boxes.o
	$(CC) $(LDFLAGS) -o boxes y.tab.o lex.yy.o boxes.o -ll

lex.yy.o: lex.yy.c y.tab.h
	$(CC) $(CFLAGS) -DYY_NO_UNPUT -Wall -W -c lex.yy.c

y.tab.o: y.tab.c y.tab.h

boxes.o: boxes.c boxes.h
	$(CC) $(CFLAGS) -Wall -W -c boxes.c

y.tab.c y.tab.h: parser.y boxes.h
	$(YACC) -d parser.y

lex.yy.c: lexer.l boxes.h
	$(LEX) -i lexer.l

clean:
	rm -f lex.yy.c y.tab.c y.tab.h
	rm -f lex.yy.o y.tab.o
	rm -f boxes.o boxes

love:
	@echo "Not in front of the kids, honey!"



#EOF
