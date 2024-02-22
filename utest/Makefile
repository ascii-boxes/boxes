#
# boxes - Command line filter to draw/remove ASCII boxes around text
# Copyright (c) 1999-2024 Thomas Jensen and the boxes contributors
#
# This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
# License, version 3, as published by the Free Software Foundation.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
# You should have received a copy of the GNU General Public License along with this program.
# If not, see <https://www.gnu.org/licenses/>.
#____________________________________________________________________________________________________________________
#====================================================================================================================

CC         = gcc

OUT_DIR    = ../out
SRC_DIR    = ../src
UTEST_DIR  = ../utest
VPATH      = $(SRC_DIR):$(SRC_DIR)/misc:$(UTEST_DIR)

UTEST_NORM = global_mock.c bxstring_test.o cmdline_test.c logging_test.c tools_test.c regulex_test.o remove_test.o \
             main.o unicode_test.o utest_tools.o

.PHONY: check_dir flags_unix flags_win32 flags_ utest

.NOTPARALLEL:

check_dir:
	@if [ "$(shell pwd | sed -e 's/^.*\///')" != "out" ] ; then \
	    echo ERROR: Please call make from the top level directory. ; \
	    exit 1 ; \
	fi

$(OUT_DIR):
	mkdir $(OUT_DIR)

flags_unix:
	$(eval CFLAGS := -I. -I$(SRC_DIR) -O -Wall -W -Wno-stringop-overflow $(CFLAGS_ADDTL))
	$(eval LDFLAGS := $(LDFLAGS) --coverage $(LDFLAGS_ADDTL))
	$(eval UTEST_EXECUTABLE_NAME := unittest)
	$(eval UTEST_OBJ := $(UTEST_NORM:.c=.o))

flags_win32:
	$(eval CFLAGS := -Os -s -std=c99 -m32 -I. -I$(SRC_DIR) -Wall -W $(CFLAGS_ADDTL))
	$(eval LDFLAGS := $(LDFLAGS) -s -std=c99 -m32 $(LDFLAGS_ADDTL))
	$(eval UTEST_EXECUTABLE_NAME := unittest.exe)
	$(eval UTEST_OBJ := $(UTEST_NORM:.c=.o))

flags_:
	@echo Please call make from the top level directory.
	exit 1

utest: flags_$(BOXES_PLATFORM) | $(OUT_DIR)
	$(MAKE) -C $(OUT_DIR) -f $(UTEST_DIR)/Makefile BOXES_PLATFORM=$(BOXES_PLATFORM) UTEST_OBJ="$(UTEST_OBJ)" \
	    CFLAGS_ADDTL="$(CFLAGS_ADDTL)" flags_$(BOXES_PLATFORM) $(UTEST_EXECUTABLE_NAME)
	rm -f $(OUT_DIR)/*.gcda
	cd $(OUT_DIR) ; ./$(UTEST_EXECUTABLE_NAME)
	@OUT_DIR=$(OUT_DIR) SRC_DIR=$(SRC_DIR) ./report.sh

unittest: $(UTEST_OBJ) | check_dir
	$(CC) $(LDFLAGS) $^ $(shell cat modules.txt) -o $@ -lunistring -lpcre2-32 -lcmocka

unittest.exe: $(UTEST_OBJ) | check_dir
	$(CC) $(LDFLAGS) $^ $(shell cat modules.txt) -o $@ \
	    -lkernel32 -l:libunistring.a -l:libpcre2-32.a -l:libiconv.a -l:libcmocka.dll.a


global_mock.o:   global_mock.c global_mock.h boxes.h unicode.h tools.h config.h | check_dir
bxstring_test.o: bxstring_test.c bxstring_test.h boxes.h bxstring.h global_mock.h tools.h unicode.h utest_tools.h config.h | check_dir
cmdline_test.o:  cmdline_test.c cmdline_test.h boxes.h cmdline.h global_mock.h tools.h config.h | check_dir
logging_test.o:  logging_test.c logging_test.h boxes.h global_mock.h logging.h tools.h config.h | check_dir
tools_test.o:    tools_test.c tools_test.h tools.h unicode.h config.h | check_dir
regulex_test.o:  regulex_test.c regulex_test.h boxes.h global_mock.h regulex.h config.h | check_dir
remove_test.o:   remove_test.c remove_test.h boxes.h remove.h shape.h tools.h unicode.h global_mock.h utest_tools.h config.h | check_dir
main.o:          main.c bxstring_test.h cmdline_test.h global_mock.h tools_test.h regulex_test.h unicode_test.h config.h | check_dir
unicode_test.o:  unicode_test.c unicode_test.h boxes.h tools.h unicode.h config.h | check_dir
utest_tools.o:   utest_tools.c utest_tools.h config.h | check_dir
