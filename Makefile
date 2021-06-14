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


# The following line (GLOBALCONF) is the only line you should need to edit!
GLOBALCONF = /usr/share/boxes
GIT_STATUS = $(shell git describe --dirty --always 2>/dev/null || echo "n/a")
BVERSION   = 2.1.1

ALL_FILES  = LICENSE README.md boxes-config
DOC_FILES  = doc/boxes.1 doc/boxes.el
PKG_NAME   = boxes-$(BVERSION)
OUT_DIR    = out

WIN_PCRE2_VERSION      = 10.36
WIN_PCRE2_DIR          = pcre2-$(WIN_PCRE2_VERSION)
WIN_FLEX_BISON_VERSION = 2.5.24
WIN_FLEX_BISON_DIR     = flex_bison_$(WIN_FLEX_BISON_VERSION)

.PHONY: clean build win32 debug win32.debug win32.pcre infomsg replaceinfos test package win32.package package_common

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

build debug: infomsg replaceinfos
	$(MAKE) -C src BOXES_PLATFORM=unix LEX=flex YACC=bison $@

win32: infomsg replaceinfos
	$(MAKE) -C src BOXES_PLATFORM=win32 C_INCLUDE_PATH=../$(WIN_PCRE2_DIR)/src LDFLAGS=-L../$(WIN_PCRE2_DIR)/.libs \
	    LEX=../$(WIN_FLEX_BISON_DIR)/win_flex.exe YACC=../$(WIN_FLEX_BISON_DIR)/win_bison.exe build

win32.debug: infomsg replaceinfos
	$(MAKE) -C src BOXES_PLATFORM=win32 C_INCLUDE_PATH=../$(WIN_PCRE2_DIR)/src LDFLAGS=-L../$(WIN_PCRE2_DIR)/.libs \
	    LEX=../$(WIN_FLEX_BISON_DIR)/win_flex.exe YACC=../$(WIN_FLEX_BISON_DIR)/win_bison.exe debug

win32.prereq:
	# download components
	curl -LO https://ftp.pcre.org/pub/pcre/pcre2-$(WIN_PCRE2_VERSION).tar.gz
	curl -LO https://downloads.sourceforge.net/project/winflexbison/win_flex_bison-$(WIN_FLEX_BISON_VERSION).zip
	# unpack components
	tar xfz $(WIN_PCRE2_DIR).tar.gz
	unzip win_flex_bison-$(WIN_FLEX_BISON_VERSION).zip -d $(WIN_FLEX_BISON_DIR)
	# build the pcre2 dependency (only needed on Windows MinGW)
	cd $(WIN_PCRE2_DIR) ; \
	./configure --disable-pcre2-8 --disable-pcre2-16 --enable-pcre2-32 --disable-shared \
				--enable-never-backslash-C --enable-newline-is-anycrlf ; \
	$(MAKE)
	# remove downloaded archives
	rm pcre2-$(WIN_PCRE2_VERSION).tar.gz
	rm win_flex_bison-$(WIN_FLEX_BISON_VERSION).zip

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

choco: tools/LICENSE.txt tools/boxes.cfg tools/boxes.exe
	choco pack --version=$(BVERSION)

tools/LICENSE.txt: LICENSE
	unix2dos -n LICENSE tools/LICENSE.txt

tools/boxes.cfg: boxes-config
	unix2dos -n boxes-config tools/boxes.cfg

tools/boxes.exe: $(OUT_DIR)/boxes.exe
	cp $(OUT_DIR)/boxes.exe tools/

$(OUT_DIR)/boxes.exe: win32

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

clean:
	rm -f $(OUT_DIR)/boxes.h tools/boxes.cfg tools/LICENSE.txt tools/boxes.exe tools/README*.md boxes.portable.*.nupkg
	rm -f doc/boxes.1 doc/boxes.1.raw.html doc/boxes.1.html
	$(MAKE) -C src clean

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

test:
	cd test; ./testrunner.sh -suite


#EOF
