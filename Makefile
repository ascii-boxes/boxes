#
#   File:       Makefile
#   Creation:   August 14, 1999 (Saturday, 01:08h)
#   Author:     Copyright (C) 1999 Thomas Jensen <boxes@thomasjensen.com>
#   Format:     GNU make
#   Purpose:    Makefile for boxes, the box drawing program
# 
#   License:  o This program is free software; you can redistribute it and/or
#               modify it under the terms of the GNU General Public License as
#               published by the Free Software Foundation; either version 2 of
#               the License, or (at your option) any later version.
#             o This program is distributed in the hope that it will be useful,
#               but WITHOUT ANY WARRANTY; without even the implied warranty of
#               MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#               GNU General Public License for more details.
#             o You should have received a copy of the GNU General Public
#               License along with this program; if not, write to the Free
#               Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#               MA 02111-1307  USA
#____________________________________________________________________________
#============================================================================


# The following line (GLOBALCONF) is the only line you should need to edit!
GLOBALCONF = /usr/share/boxes
BVERSION   = 1.2.1-SNAPSHOT

ALL_FILES  = LICENSE README.md README.Win32.txt boxes-config
DOC_FILES  = doc/boxes.1 doc/boxes.el
PKG_NAME   = boxes-$(BVERSION)

.PHONY: clean build win32 debug win32.debug infomsg replaceinfos test package win32.package package_common


build debug: infomsg replaceinfos
	$(MAKE) -C src BOXES_PLATFORM=unix $@

win32: infomsg replaceinfos
	$(MAKE) -C src BOXES_PLATFORM=win32 build

win32.debug: infomsg replaceinfos
	$(MAKE) -C src BOXES_PLATFORM=win32 debug


infomsg:
	@echo "| For compilation info please refer to the boxes compilation FAQ"
	@echo "| at http://boxes.thomasjensen.com/docs/faq.html#q5"

replaceinfos: src/boxes.h doc/boxes.1

src/boxes.h: src/boxes.h.in src/regexp/regexp.h Makefile
	sed -e 's/--BVERSION--/$(BVERSION)/; s/--GLOBALCONF--/$(subst /,\/,$(GLOBALCONF))/' src/boxes.h.in > src/boxes.h

doc/boxes.1: doc/boxes.1.in Makefile
	sed -e 's/--BVERSION--/$(BVERSION)/; s/--GLOBALCONF--/$(subst /,\/,$(GLOBALCONF))/' doc/boxes.1.in > doc/boxes.1


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


clean:
	rm -f doc/boxes.1 src/boxes.h
	$(MAKE) -C src clean


test:
	cd test; ./testrunner.sh -suite


#EOF
