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
