# Makefile for boxes

LEX    = flex
YACC   = yacc
CC     = gcc
CFLAGS = -O


boxes: y.tab.o lex.yy.o boxes.o
	$(CC) $(CFLAGS) -o boxes y.tab.o lex.yy.o boxes.o -ll

lex.yy.o: lex.yy.c y.tab.h

y.tab.o: y.tab.c y.tab.h

boxes.o: boxes.c boxes.h

y.tab.c y.tab.h: parser.y boxes.h
	$(YACC) -d parser.y

lex.yy.c: lexer.l boxes.h
	$(LEX) -i lexer.l

clean:
	rm -f lex.yy.c y.tab.c y.tab.h
	rm -f lex.yy.o y.tab.o
	rm -f boxes

love:
	@echo "Not in front of the kids, honey!"



#EOF
