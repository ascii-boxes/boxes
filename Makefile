#
#   File:       Makefile
#   Creation:   August 14, 1999 (Saturday, 01:08h)
#   Author:     Copyright (C) 1999 Thomas Jensen
#               tsjensen@stud.informatik.uni-erlangen.de
#   Version:    $Id$
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
#     $Log$
#____________________________________________________________________________
#============================================================================


SNAPFILE  = boxes-SNAP-$(shell date +%Y%m%d)
WEBHOME   = $(HOME)/d/public_html/software/boxes
CLOGFILE  = $(WEBHOME)/changelogs.html

OTH_FILES = COPYING doc/boxes.1
CL_FILES  = boxes-config
RCS_FILES = $(CL_FILES) Makefile
ALL_FILES = $(RCS_FILES) $(OTH_FILES)

build debug boxes clean:
	$(MAKE) -C src $@

snap: $(ALL_FILES)
	mkdir $(SNAPFILE)
	cp $(ALL_FILES) $(SNAPFILE)
	$(MAKE) -C src SNAPFILE=../$(SNAPFILE) snap
	gtar cfvz $(SNAPFILE).tar.gz $(SNAPFILE)/*
	rm -rf $(SNAPFILE)/
	cp $(SNAPFILE).tar.gz $(WEBHOME)/download/
	rm -f $(WEBHOME)/download/current-SNAP.tar.gz
	(cd $(WEBHOME)/download; ln -s $(SNAPFILE).tar.gz current-SNAP.tar.gz)

rcstest:
	-for i in $(RCS_FILES) ; do rcsdiff $$i ; done
	$(MAKE) -C src rcstest

logpage: $(CL_FILES)
	cd src; ../doc/create_changelog.pl $(patsubst %,../%,$(CL_FILES)) $(shell $(MAKE) -C src -s logpage) > $(CLOGFILE)

love:
	@echo "Not in front of the kids, honey!"



#EOF
