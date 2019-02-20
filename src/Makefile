#
# boxes - Command line filter to draw/remove ASCII boxes around text
# Copyright (C) 1999  Thomas Jensen and the boxes contributors
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License, version 2, as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#____________________________________________________________________________
#============================================================================


LEX        = flex
YACC       = bison
CC         = gcc

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

ALL_FILES  = $(ORIG_FILES) $(GEN_FILES) $(OTH_FILES)


.PHONY: clean build debug package flags_unix flags_win32 flags_


build: flags_$(BOXES_PLATFORM)
	$(MAKE) BOXES_PLATFORM=$(BOXES_PLATFORM) ALL_OBJ="$(ALL_OBJ)" CFLAGS_ADDTL="-O $(CFLAGS_ADDTL)" STRIP=true flags_$(BOXES_PLATFORM) $(BOXES_EXECUTABLE_NAME)

debug: flags_$(BOXES_PLATFORM)
	$(MAKE) BOXES_PLATFORM=$(BOXES_PLATFORM) ALL_OBJ="$(ALL_OBJ)" CFLAGS_ADDTL="-g $(CFLAGS_ADDTL)" STRIP=false flags_$(BOXES_PLATFORM) $(BOXES_EXECUTABLE_NAME)

boxes: $(ALL_OBJ)
	$(MAKE) -C regexp CC=$(CC) libregexp.a
	$(CC) $(LDFLAGS) $(ALL_OBJ) -o $(BOXES_EXECUTABLE_NAME) -lregexp
	if [ "$(STRIP)" = "true" ] ; then strip $(BOXES_EXECUTABLE_NAME) ; fi

boxes.exe: $(ALL_OBJ)
	$(CC) $(LDFLAGS) $(ALL_OBJ) -o $(BOXES_EXECUTABLE_NAME) -lkernel32
	if [ "$(STRIP)" = "true" ] ; then strip $(BOXES_EXECUTABLE_NAME) ; fi


flags_unix:
	$(eval CFLAGS := -I. -Iregexp -Wall -W $(CFLAGS_ADDTL))
	$(eval LDFLAGS := -Lregexp $(LDFLAGS_ADDTL))
	$(eval BOXES_EXECUTABLE_NAME := boxes)
	$(eval ALL_OBJ := $(GEN_SRC:.c=.o) $(ORIG_NORM:.c=.o))

flags_win32:
	$(eval CFLAGS := -Os -s -m32 -I. -Iregexp -Wall -W $(CFLAGS_ADDTL))
	$(eval LDFLAGS := -s -m32)
	$(eval BOXES_EXECUTABLE_NAME := boxes.exe)
	$(eval ALL_OBJ := $(GEN_SRC:.c=.o) $(ORIG_NORM:.c=.o) regexp/regexp.o regexp/regsub.o misc/getopt.o)

flags_:
	@echo Please call make from the top level directory.
	exit 1

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
regexp/regexp.o: regexp/regexp.c
regexp/regsub.o: regexp/regsub.c
misc/getopt.o: misc/getopt.c


package: $(BOXES_EXECUTABLE_NAME)
	if [ -z "$(PKG_NAME)" ] ; then exit 1 ; fi
	cp $(BOXES_EXECUTABLE_NAME) ../$(PKG_NAME)

clean: flags_unix
	rm -f $(ALL_OBJ)
	rm -f $(GEN_FILES)
	rm -f core boxes boxes.exe
	$(MAKE) -C regexp clean


#EOF
