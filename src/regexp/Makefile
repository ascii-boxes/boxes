#
#   File:       Makefile
#   Creation:   August 13, 1999 (Friday, 22:13h)
#   Author:     Copyright (C) 1999 Thomas Jensen <boxes@thomasjensen.com>
#   Format:     GNU make
#   Web Site:   http://boxes.thomasjensen.com/
#   Purpose:    Makefile for boxes, the box drawing program
#               generation of regexp handling library
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


CFLAGS   = -O -I. $(CFLAGS_ADDTL)

ALL_CL   = regexp/regexp.c regexp/regsub.c
C_SRC    = $(notdir $(ALL_CL))
ALLFILES = Makefile $(C_SRC) regexp.h regmagic.h
ALLOBJ   = $(C_SRC:.c=.o)


.PHONY: clean build debug


build: libregexp.a
debug: libregexp.a

libregexp.a: $(ALLOBJ)
	ar cr libregexp.a $(ALLOBJ)

regexp.o: regexp.c regmagic.h regexp.h ../config.h
regsub.o: regsub.c regmagic.h regexp.h ../config.h

.c.o:
	$(CC) $(CFLAGS) -c $<


clean:
	rm -f $(ALLOBJ) libregexp.a core


#EOF
