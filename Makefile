#
#   File:       Makefile
#   Creation:   August 14, 1999 (Saturday, 01:08h)
#   Author:     Copyright (C) 1999 Thomas Jensen
#               tsjensen@stud.informatik.uni-erlangen.de
#   Version:    $Id: Makefile,v 1.5 2000/03/17 23:52:29 tsjensen Exp tsjensen $
#   Format:     GNU make
#   Web Site:   http://home.pages.de/~jensen/boxes/
#   Platforms:  sparc/Solaris 2.6 and others
#   Purpose:    Makefile for boxes, the box drawing program
# 
#   Remarks:  o This program is free software; you can redistribute it and/or
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
#
#   Revision History:
#
#     $Log: Makefile,v $
#     Revision 1.5  2000/03/17 23:52:29  tsjensen
#     Incremented version number to 1.0.1
#     Renamed snapshot file name to boxes-SNAP-latest
#     Added README.Win32.txt to snapshot generation commands
#
#     Revision 1.4  1999/08/22 11:43:55  tsjensen
#     Added comment to guide user to GLOBALCONF definition line
#
#     Revision 1.3  1999/08/21 23:39:01  tsjensen
#     Added GLOBALCONF and BVERSION macros whose values are put into boxes.h and
#     boxes.1
#     Added locsnap target for generation of archives without posting to the Web
#     page
#     Added README file
#     Added rules to generate boxes.h and boxes.1 from boxes.1.in and boxes.h.in
#
#     Revision 1.2  1999/08/14 19:01:31  tsjensen
#     After taking a snapshot, put it in the author's archives, too.
#
#     Revision 1.1  1999/08/13 23:45:34  tsjensen
#     Initial revision
#____________________________________________________________________________
#============================================================================


# The following line (GLOBALCONF) is the only line you should need to edit!
GLOBALCONF = /usr/local/share/boxes
BVERSION   = 1.0.1

SNAPFILE   = boxes-SNAP-$(shell date +%Y%m%d)
WEBHOME    = $(HOME)/d/public_html/software/boxes
CLOGFILE   = $(WEBHOME)/changelogs.html

CL_FILES   = boxes-config
RCS_FILES  = $(CL_FILES) Makefile doc/boxes.1.in
ALL_FILES  = COPYING README README.Win32.txt boxes-config Makefile
DOC_FILES  = doc/boxes.1 doc/boxes.1.in


build debug boxes: src/boxes.h doc/boxes.1
	$(MAKE) -C src $@


src/boxes.h: src/boxes.h.in src/regexp/regexp.h Makefile
	sed -e 's/--BVERSION--/$(BVERSION)/; s/--GLOBALCONF--/$(subst /,\/,$(GLOBALCONF))/' src/boxes.h.in > src/boxes.h

doc/boxes.1: doc/boxes.1.in Makefile
	sed -e 's/--BVERSION--/$(BVERSION)/; s/--GLOBALCONF--/$(subst /,\/,$(GLOBALCONF))/' doc/boxes.1.in > doc/boxes.1


clean:
	rm -f doc/boxes.1
	$(MAKE) -C src clean

locsnap: $(ALL_FILES) $(DOC_FILES)
	mkdir -p $(SNAPFILE)/doc
	cp $(ALL_FILES) $(SNAPFILE)
	cp $(DOC_FILES) $(SNAPFILE)/doc
	$(MAKE) -C src SNAPFILE=../$(SNAPFILE) snap
	gtar cfvz $(SNAPFILE).tar.gz $(SNAPFILE)/*
	rm -rf $(SNAPFILE)/

snap: locsnap
	cp $(SNAPFILE).tar.gz $(WEBHOME)/download/
	rm -f $(WEBHOME)/download/boxes-SNAP-latest.tar.gz
	(cd $(WEBHOME)/download; ln -s $(SNAPFILE).tar.gz boxes-SNAP-latest.tar.gz)
	mv -i $(SNAPFILE).tar.gz archive/
	chmod 444 archive/$(SNAPFILE).tar.gz

rcstest:
	-for i in $(RCS_FILES) ; do rcsdiff $$i ; done
	$(MAKE) -C src rcstest

logpage: $(CL_FILES)
	cd src; ../doc/create_changelog.pl $(patsubst %,../%,$(CL_FILES)) $(shell $(MAKE) -C src -s logpage) > $(CLOGFILE)


love:
	@echo "Not in front of the kids, honey!"



#EOF
