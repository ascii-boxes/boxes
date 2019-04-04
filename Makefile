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


# The following line (GLOBALCONF) is the only line you should need to edit!
GLOBALCONF = /usr/share/boxes
GIT_STATUS = ($(shell git rev-parse --short HEAD)$(shell if [ $$(git status -s -uall | wc -l) -ge 1 ] ; then echo ", dirty" ; fi))
BVERSION   = 1.3.1-SNAPSHOT

ALL_FILES  = LICENSE README.md README.Win32.md boxes-config
DOC_FILES  = doc/boxes.1 doc/boxes.el
PKG_NAME   = boxes-$(BVERSION)

.PHONY: clean build win32 debug win32.debug infomsg replaceinfos test package win32.package package_common

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

build debug: infomsg replaceinfos
	$(MAKE) -C src BOXES_PLATFORM=unix $@

win32: infomsg replaceinfos
	$(MAKE) -C src BOXES_PLATFORM=win32 build

win32.debug: infomsg replaceinfos
	$(MAKE) -C src BOXES_PLATFORM=win32 debug

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

infomsg:
	@echo "| For compilation info please refer to the boxes compilation FAQ"
	@echo "| at https://boxes.thomasjensen.com/docs/faq.html#q5"

replaceinfos: src/boxes.h doc/boxes.1

src/boxes.h: src/boxes.h.in src/regexp/regexp.h Makefile
	sed -e 's/--BVERSION--/$(BVERSION) $(GIT_STATUS)/; s/--GLOBALCONF--/$(subst /,\/,$(GLOBALCONF))/' src/boxes.h.in > src/boxes.h

doc/boxes.1: doc/boxes.1.in Makefile
	sed -e 's/--BVERSION--/$(BVERSION)/; s/--GLOBALCONF--/$(subst /,\/,$(GLOBALCONF))/' doc/boxes.1.in > doc/boxes.1

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

$(PKG_NAME).tar.gz:
	mkdir -p $(PKG_NAME)/doc
	cp $(ALL_FILES) $(PKG_NAME)
	cp $(DOC_FILES) $(PKG_NAME)/doc
	$(MAKE) -C src PKG_NAME=$(PKG_NAME) BOXES_PLATFORM=$(BOXES_PLATFORM) flags_$(BOXES_PLATFORM) package
	if which gtar >/dev/null 2>&1 ; then gtar cfvz $(PKG_NAME).tar.gz $(PKG_NAME)/* ; \
	    else tar cfvz $(PKG_NAME).tar.gz $(PKG_NAME)/* ; fi
	rm -rf $(PKG_NAME)/

package: build
	$(MAKE) BOXES_PLATFORM=unix $(PKG_NAME).tar.gz

win32.package: win32
	$(MAKE) BOXES_PLATFORM=win32 $(PKG_NAME).tar.gz

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

choco: tools/LICENSE.txt tools/boxes.cfg tools/boxes.exe
	choco pack --version=$(BVERSION)

tools/LICENSE.txt: LICENSE
	cp LICENSE tools/LICENSE.txt

tools/boxes.cfg: boxes-config
	cp boxes-config tools/boxes.cfg

tools/boxes.exe: src/boxes.exe
	cp src/boxes.exe tools/

src/boxes.exe: win32

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

clean:
	rm -f doc/boxes.1 src/boxes.h tools/boxes.cfg tools/LICENSE.txt tools/boxes.exe tools/README*.md boxes.portable.*.nupkg
	$(MAKE) -C src clean

# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

test:
	cd test; ./testrunner.sh -suite


#EOF
