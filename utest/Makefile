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
UTEST_DIR  = ../utest
VPATH      = $(SRC_DIR):$(SRC_DIR)/misc:$(UTEST_DIR)

UTEST_NORM = global_mock.c tools_test.c

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
	$(eval CFLAGS := -I. -I$(SRC_DIR) -O -Wall -W $(CFLAGS_ADDTL))
	$(eval LDFLAGS := $(LDFLAGS) --coverage $(LDFLAGS_ADDTL))
	$(eval UTEST_EXECUTABLE_NAME := unittest)
	$(eval UTEST_OBJ := $(UTEST_NORM:.c=.o))

flags_win32:
	$(eval CFLAGS := -Os -s -m32 -I. -I$(SRC_DIR) -Wall -W $(CFLAGS_ADDTL))
	$(eval LDFLAGS := $(LDFLAGS) --coverage -s -m32 $(LDFLAGS_ADDTL))
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


global_mock.o: global_mock.c boxes.h
tools_test.o: tools_test.c tools.h
