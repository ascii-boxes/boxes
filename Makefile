#
# boxes - Command line filter to draw/remove ASCII boxes around text
# Copyright (c) 1999-2023 Thomas Jensen and the boxes contributors
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


# The following line (GLOBALCONF) is the only line you should need to edit!
GLOBALCONF = /usr/share/boxes
GIT_STATUS = $(shell git describe --dirty --always 2>/dev/null || echo "n/a")
BVERSION   = 2.2.0

ALL_FILES  = LICENSE README.md boxes-config
DOC_FILES  = doc/boxes.1 doc/boxes.el
PKG_NAME   = boxes-$(BVERSION)
OUT_DIR    = out

# Set some defaults for BX_LEX and BX_YACC but allow env variables to override them.
BX_LEX  ?= flex
BX_YACC ?= bison

PCRE2_VERSION          = 10.40
PCRE2_DIR              = vendor/pcre2-$(PCRE2_VERSION)
LIBUNISTRING_VERSION   = 1.0
LIBUNISTRING_DIR       = vendor/libunistring-$(LIBUNISTRING_VERSION)
LIBNCURSES_VERSION     = 6.4
LIBNCURSES_DIR         = vendor/ncurses-$(LIBNCURSES_VERSION)
LIBNCURSES_WIN_INCLUDE = /c/MinGW/include/ncurses
WIN_FLEX_BISON_VERSION = 2.5.24
WIN_FLEX_BISON_DIR     = vendor/flex_bison_$(WIN_FLEX_BISON_VERSION)
WIN_CMOCKA_VERSION     = 1.1.0
WIN_CMOCKA_DIR         = vendor/cmocka-$(WIN_CMOCKA_VERSION)

.PHONY: clean cleanall build cov win32 debug win32.debug win32.pcre infomsg replaceinfos test covtest \
        package win32.package package_common utest win32.utest static


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#    Build
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

build cov debug: infomsg replaceinfos
	$(MAKE) -C src BOXES_PLATFORM=unix LEX=$(BX_LEX) YACC=$(BX_YACC) $@

win32: infomsg replaceinfos
	$(MAKE) -C src BOXES_PLATFORM=win32 C_INCLUDE_PATH=../$(PCRE2_DIR)/src LDFLAGS=-L../$(PCRE2_DIR)/.libs \
	    LEX=../$(WIN_FLEX_BISON_DIR)/win_flex.exe YACC=../$(WIN_FLEX_BISON_DIR)/win_bison.exe \
	    LIBNCURSES_WIN_INCLUDE=$(LIBNCURSES_WIN_INCLUDE) build

win32.debug: infomsg replaceinfos
	$(MAKE) -C src BOXES_PLATFORM=win32 C_INCLUDE_PATH=../$(PCRE2_DIR)/src LDFLAGS=-L../$(PCRE2_DIR)/.libs \
	    LEX=../$(WIN_FLEX_BISON_DIR)/win_flex.exe YACC=../$(WIN_FLEX_BISON_DIR)/win_bison.exe \
	    LIBNCURSES_WIN_INCLUDE=$(LIBNCURSES_WIN_INCLUDE) debug

win32.prereq: $(PCRE2_DIR)/.libs/libpcre2-32.a vendor/win_flex_bison-$(WIN_FLEX_BISON_VERSION).zip \
              vendor/cmocka-$(WIN_CMOCKA_VERSION)-mingw.zip
	unzip -n vendor/win_flex_bison-$(WIN_FLEX_BISON_VERSION).zip -d $(WIN_FLEX_BISON_DIR)
	unzip -n vendor/cmocka-$(WIN_CMOCKA_VERSION)-mingw.zip -d vendor

vendor/win_flex_bison-$(WIN_FLEX_BISON_VERSION).zip: | vendor
	curl -L https://downloads.sourceforge.net/project/winflexbison/win_flex_bison-$(WIN_FLEX_BISON_VERSION).zip --output $@

vendor/cmocka-$(WIN_CMOCKA_VERSION)-mingw.zip: | vendor
	curl -L https://cmocka.org/files/$(WIN_CMOCKA_VERSION:%.0=%)/cmocka-$(WIN_CMOCKA_VERSION)-mingw.zip --output $@

vendor/pcre2-$(PCRE2_VERSION).tar.gz: | vendor
	curl -L https://github.com/PCRE2Project/pcre2/releases/download/pcre2-$(PCRE2_VERSION)/pcre2-$(PCRE2_VERSION).tar.gz --output $@

$(PCRE2_DIR)/.libs/libpcre2-32.a: vendor/pcre2-$(PCRE2_VERSION).tar.gz
	tar -xzf vendor/pcre2-$(PCRE2_VERSION).tar.gz -C vendor
	cd $(PCRE2_DIR) ; \
	./configure --disable-pcre2-8 --disable-pcre2-16 --enable-pcre2-32 --enable-static --disable-shared \
	            --enable-never-backslash-C --enable-newline-is-anycrlf ; \
	$(MAKE)

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

infomsg:
	@echo "| For compilation info please refer to the boxes compilation FAQ"
	@echo "| at https://boxes.thomasjensen.com/faq.html#q5"

$(OUT_DIR):
	mkdir $(OUT_DIR)

replaceinfos: $(OUT_DIR)/boxes.h doc/boxes.1

$(OUT_DIR)/boxes.h: src/boxes.in.h Makefile | $(OUT_DIR)
	$(eval VERSION_STRING := $(shell if [ "${GIT_STATUS}" = "n/a" ]; then echo "${BVERSION}"; else echo "${BVERSION} (${GIT_STATUS})"; fi))
	sed -e 's/--BVERSION--/$(VERSION_STRING)/; s/--GLOBALCONF--/$(subst /,\/,$(GLOBALCONF))/' src/boxes.in.h > $(OUT_DIR)/boxes.h

doc/boxes.1: doc/boxes.1.in Makefile
	sed -e 's/--BVERSION--/$(BVERSION)/; s/--GLOBALCONF--/$(subst /,\/,$(GLOBALCONF))/' doc/boxes.1.in > doc/boxes.1

doc/boxes.1.html: doc/boxes.1
	cat doc/boxes.1 | groff -mandoc -Thtml > doc/boxes.1.raw.html
	sed -E -e 's/&lt;URL:([^&]+)&gt;/<a href="\1">\1<\/a>/g' < doc/boxes.1.raw.html > doc/boxes.1.html
	rm -f doc/boxes.1.raw.html
	@echo "Conversion complete. Excessive manual work remains."


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#    Static Build (Unix)
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

vendor:
	mkdir -p vendor

vendor/libunistring-$(LIBUNISTRING_VERSION).tar.gz: | vendor
	curl -L https://ftp.gnu.org/gnu/libunistring/libunistring-$(LIBUNISTRING_VERSION).tar.gz --output $@

$(LIBUNISTRING_DIR)/lib/.libs/libunistring.a: vendor/libunistring-$(LIBUNISTRING_VERSION).tar.gz
	tar -C vendor -xzf vendor/libunistring-$(LIBUNISTRING_VERSION).tar.gz
	cd $(LIBUNISTRING_DIR) ; ./configure --enable-static ; $(MAKE)

vendor/libncurses-$(LIBNCURSES_VERSION).tar.gz: | vendor
	curl -L https://invisible-mirror.net/archives/ncurses/ncurses-$(LIBNCURSES_VERSION).tar.gz --output $@

$(LIBNCURSES_DIR)/lib/libncurses.a: vendor/libncurses-$(LIBNCURSES_VERSION).tar.gz
	tar -C vendor -xzf vendor/libncurses-$(LIBNCURSES_VERSION).tar.gz
	cd $(LIBNCURSES_DIR) ; ./configure --enable-static ; $(MAKE)

static: infomsg replaceinfos $(LIBUNISTRING_DIR)/lib/.libs/libunistring.a $(PCRE2_DIR)/.libs/libpcre2-32.a $(LIBNCURSES_DIR)/lib/libncurses.a
	$(MAKE) -C src BOXES_PLATFORM=static LEX=$(BX_LEX) YACC=$(BX_YACC) LIBUNISTRING_DIR=$(LIBUNISTRING_DIR) \
        PCRE2_DIR=$(PCRE2_DIR) LIBNCURSES_DIR=$(LIBNCURSES_DIR) LIBNCURSES_WIN_INCLUDE=$(LIBNCURSES_WIN_INCLUDE) $@


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#    Standalone Packaging (Unix / Windows)
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

$(OUT_DIR)/$(PKG_NAME).tar.gz:
	mkdir -p $(OUT_DIR)/$(PKG_NAME)/doc
	cp $(ALL_FILES) $(OUT_DIR)/$(PKG_NAME)/
	cp $(DOC_FILES) $(OUT_DIR)/$(PKG_NAME)/doc/
	$(MAKE) -C $(OUT_DIR) -f ../src/Makefile PKG_NAME=$(PKG_NAME) BOXES_PLATFORM=$(BOXES_PLATFORM) flags_$(BOXES_PLATFORM) package
	if which gtar >/dev/null 2>&1 ; then cd $(OUT_DIR) ; gtar cfvz $(PKG_NAME).tar.gz $(PKG_NAME)/* ; \
	    else cd $(OUT_DIR) ; tar cfvz $(PKG_NAME).tar.gz $(PKG_NAME)/* ; fi
	rm -rf $(OUT_DIR)/$(PKG_NAME)/

$(OUT_DIR)/zip/$(PKG_NAME).zip:
	mkdir -p $(OUT_DIR)/zip/$(PKG_NAME)
	unix2dos -n LICENSE $(OUT_DIR)/zip/$(PKG_NAME)/LICENSE.txt
	unix2dos -n boxes-config $(OUT_DIR)/zip/$(PKG_NAME)/boxes.cfg
	unix2dos -n README.md $(OUT_DIR)/zip/$(PKG_NAME)/README.md
	unix2dos -n README.Win32.md $(OUT_DIR)/zip/$(PKG_NAME)/README.Win32.md
	unix2dos -n doc/boxes.1 $(OUT_DIR)/zip/$(PKG_NAME)/boxes.1
	unix2dos -n doc/boxes.1.html $(OUT_DIR)/zip/$(PKG_NAME)/boxes-man-1.html
	cp -a $(OUT_DIR)/boxes.exe $(OUT_DIR)/zip/$(PKG_NAME)/
	cd $(OUT_DIR)/zip ; zip $(PKG_NAME).zip $(PKG_NAME)/*
	@echo Windows ZIP file created at $(OUT_DIR)/zip/$(PKG_NAME).zip

package: build
	$(MAKE) BOXES_PLATFORM=unix $(PKG_NAME).tar.gz

win32.package: win32 $(OUT_DIR)/zip/$(PKG_NAME).zip


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#    Chocolatey Packaging (Windows)
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

choco: tools/LICENSE.txt tools/boxes.cfg tools/boxes.exe
	choco pack --version=$(BVERSION)

tools/LICENSE.txt: LICENSE
	unix2dos -n LICENSE tools/LICENSE.txt

tools/boxes.cfg: boxes-config
	unix2dos -n boxes-config tools/boxes.cfg

tools/boxes.exe: $(OUT_DIR)/boxes.exe
	cp $(OUT_DIR)/boxes.exe tools/

$(OUT_DIR)/boxes.exe: win32


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#    Test
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

test:
	cd test; ./testrunner.sh --suite

covtest:
	cd test; ./testrunner.sh --suite --coverage

utest:
	$(MAKE) -C utest BOXES_PLATFORM=unix utest

debugutest:
	$(MAKE) -C utest BOXES_PLATFORM=unix debugutest

win32.utest: $(OUT_DIR)
	cp $(WIN_CMOCKA_DIR)/bin/cmocka.dll $(OUT_DIR)/
	$(MAKE) -C utest BOXES_PLATFORM=win32 C_INCLUDE_PATH=../$(PCRE2_DIR)/src:../$(WIN_CMOCKA_DIR)/include \
	    LDFLAGS_ADDTL="-L../$(PCRE2_DIR)/.libs -L../$(WIN_CMOCKA_DIR)/lib" utest


# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#    Cleanup
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

clean:
	rm -f tools/boxes.cfg tools/LICENSE.txt tools/boxes.exe tools/README*.md boxes.portable.*.nupkg
	rm -f doc/boxes.1 doc/boxes.1.raw.html doc/boxes.1.html
	$(MAKE) -C src clean

cleanall: clean
	rm -rf vendor


#EOF
