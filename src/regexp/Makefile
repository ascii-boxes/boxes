#
#   File:       Makefile
#   Creation:   August 13, 1999 (Friday, 22:13h)
#   Author:     Copyright (C) 1999 Thomas Jensen <boxes@thomasjensen.com>
#   Version:    $Id: Makefile,v 1.3 2006/07/12 21:28:38 tsjensen Exp tsjensen $
#   Format:     GNU make
#   Web Site:   http://boxes.thomasjensen.com/
#   Platforms:  sparc/Solaris 2.6 and others
#   Purpose:    Makefile for boxes, the box drawing program
#               generation of regexp handling library
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
#     Revision 1.3  2006/07/12 21:28:38  tsjensen
#     Added target rcslocks
#
#     Revision 1.2  2006/07/08 08:47:58  tsjensen
#     Removed -traditional flag from CFLAGS definition
#     Updated email and web addresses
#
#     Revision 1.1  1999/08/13 20:56:09  tsjensen
#     Initial revision
#
#____________________________________________________________________________
#============================================================================


CFLAGS   = -O -I. $(CFLAGS_ADDTL)

ALL_CL   = regexp/regexp.c regexp/regsub.c
C_SRC    = $(notdir $(ALL_CL))
ALLFILES = Makefile $(C_SRC) regexp.h regmagic.h
ALLOBJ   = $(C_SRC:.c=.o)


build: libregexp.a
debug: libregexp.a

libregexp.a: $(ALLOBJ)
	ar cr libregexp.a $(ALLOBJ)

regexp.o: regexp.c regmagic.h regexp.h ../config.h
regsub.o: regsub.c regmagic.h regexp.h ../config.h

.c.o:
	$(CC) $(CFLAGS) -c $<

snap: $(ALLFILES)
	@if [ -z "$(SNAPFILE)" ] ; then echo "make snap must be run from the main directory" ; exit 1 ; fi
	mkdir $(SNAPFILE)/src/regexp
	cp $(ALLFILES) $(SNAPFILE)/src/regexp

rcstest:
	-for i in $(ALLFILES) ; do rcsdiff $$i ; done

rcslocks:
	@rlog -L -R $(ALLFILES) | sed -e 's/^/    - src\/regexp\//'

logpage: $(C_SRC)
	@echo $(ALL_CL)

clean:
	rm -f $(ALLOBJ) libregexp.a core

love:
	@echo "Not in front of the kids, honey!"



#EOF
