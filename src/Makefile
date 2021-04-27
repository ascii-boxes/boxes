#
# boxes - Command line filter to draw/remove ASCII boxes around text
# Copyright (c) 1999-2021 Thomas Jensen and the boxes contributors
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


CC         = gcc

OUT_DIR    = ../out
SRC_DIR    = ../src
VPATH      = $(SRC_DIR):$(SRC_DIR)/misc

GEN_HDR    = parser.h boxes.h lex.yy.h
GEN_SRC    = parser.c lex.yy.c
GEN_FILES  = $(GEN_SRC) $(GEN_HDR)
ORIG_HDRCL = boxes.in.h config.h
ORIG_HDR   = $(ORIG_HDRCL) cmdline.h discovery.h generate.h input.h list.h parsecode.h parsing.h query.h regulex.h \
             remove.h shape.h tools.h unicode.h
ORIG_GEN   = lexer.l parser.y
ORIG_NORM  = boxes.c cmdline.c discovery.c generate.c input.c list.c parsecode.c parsing.c query.c regulex.c remove.c \
             shape.c tools.c unicode.c
ORIG_SRC   = $(ORIG_GEN) $(ORIG_NORM)
ORIG_FILES = $(ORIG_SRC) $(ORIG_HDR)


.PHONY: check_dir clean build debug package flags_unix flags_win32 flags_

.NOTPARALLEL:

check_dir:
	@if [ "$(shell pwd | sed -e 's/^.*\///')" != "out" ] ; then \
	    echo ERROR: Please call make from the top level directory. ; \
		exit 1 ; \
	fi

$(OUT_DIR):
	mkdir $(OUT_DIR)

build: flags_$(BOXES_PLATFORM) | $(OUT_DIR)
	$(MAKE) -C $(OUT_DIR) -f $(SRC_DIR)/Makefile BOXES_PLATFORM=$(BOXES_PLATFORM) ALL_OBJ="$(ALL_OBJ)" \
	    CFLAGS_ADDTL="-O $(CFLAGS_ADDTL)" STRIP=true flags_$(BOXES_PLATFORM) $(BOXES_EXECUTABLE_NAME)

debug: flags_$(BOXES_PLATFORM) | $(OUT_DIR)
	$(MAKE) -C $(OUT_DIR) -f $(SRC_DIR)/Makefile BOXES_PLATFORM=$(BOXES_PLATFORM) ALL_OBJ="$(ALL_OBJ)" \
	    CFLAGS_ADDTL="-g $(CFLAGS_ADDTL)" STRIP=false flags_$(BOXES_PLATFORM) $(BOXES_EXECUTABLE_NAME)

boxes: $(ALL_OBJ) | check_dir
	$(CC) $(LDFLAGS) $^ -o $@ -lunistring -lpcre2-32
	if [ "$(STRIP)" = "true" ] ; then strip $@ ; fi

boxes.exe: $(ALL_OBJ) | check_dir
	$(CC) $(LDFLAGS) $^ -o $@ -lkernel32 -l:libunistring.a -l:libpcre2-32.a -l:libiconv.a
	if [ "$(STRIP)" = "true" ] ; then strip $@ ; fi


flags_unix:
	$(eval CFLAGS := -I. -I$(SRC_DIR) -Wall -W $(CFLAGS_ADDTL))
	$(eval LDFLAGS := $(LDFLAGS_ADDTL))
	$(eval BOXES_EXECUTABLE_NAME := boxes)
	$(eval ALL_OBJ := $(GEN_SRC:.c=.o) $(ORIG_NORM:.c=.o))

flags_win32:
	$(eval CFLAGS := -Os -s -m32 -I. -I$(SRC_DIR) -Wall -W $(CFLAGS_ADDTL))
	$(eval LDFLAGS := -s -m32)
	$(eval BOXES_EXECUTABLE_NAME := boxes.exe)
	$(eval ALL_OBJ := $(GEN_SRC:.c=.o) $(ORIG_NORM:.c=.o) getopt.o)

flags_:
	@echo Please call make from the top level directory.
	exit 1

parser.c parser.h: parser.y lex.yy.h | check_dir
	$(YACC) --warnings=all --verbose --defines=parser.h --output=parser.c $<

lex.yy.c lex.yy.h: lexer.l | check_dir
	$(LEX) --header-file=lex.yy.h $<

boxes.o:     boxes.c boxes.h cmdline.h discovery.h generate.h input.h list.h remove.h tools.h unicode.h config.h | check_dir
cmdline.o:   cmdline.c cmdline.h boxes.h tools.h config.h | check_dir
discovery.o: discovery.c discovery.h boxes.h tools.h config.h | check_dir
generate.o:  generate.c generate.h boxes.h shape.h tools.h unicode.h config.h | check_dir
getopt.o:    misc/getopt.c misc/getopt.h | check_dir
input.o:     input.c boxes.h input.h regulex.h tools.h unicode.h config.h | check_dir
lex.yy.o:    lex.yy.c parser.h boxes.h parsing.h tools.h shape.h config.h | check_dir
list.o:      list.c list.h boxes.h parsing.h query.h tools.h config.h | check_dir
parsecode.o: parsecode.c parser.h boxes.h tools.h lex.yy.h query.h regulex.h unicode.h config.h | check_dir
parser.o:    parser.c boxes.h lex.yy.h parser.h parsing.h tools.h shape.h discovery.h config.h | check_dir
parsing.o:   parsing.c parsing.h parser.h lex.yy.h boxes.h tools.h config.h | check_dir
query.o:     query.c query.h boxes.h list.h tools.h config.h | check_dir
regulex.o:   regulex.c regulex.h boxes.h tools.h unicode.h config.h | check_dir
remove.o:    remove.c remove.h boxes.h shape.h tools.h unicode.h config.h | check_dir
shape.o:     shape.c shape.h boxes.h tools.h config.h | check_dir
tools.o:     tools.c tools.h boxes.h shape.h unicode.h config.h | check_dir
unicode.o:   unicode.c unicode.h boxes.h tools.h config.h | check_dir

package: $(BOXES_EXECUTABLE_NAME)
	if [ -z "$(PKG_NAME)" ] ; then exit 1 ; fi
	cp -a $(BOXES_EXECUTABLE_NAME) $(PKG_NAME)/

clean:
	rm -rf $(OUT_DIR)
	rm -f core $(GEN_HDR)


#EOF
