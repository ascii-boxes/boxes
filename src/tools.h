/*
 *  File:             tools.h
 *  Project Main:     boxes.c
 *  Date created:     June 20, 1999 (Sunday, 16:57h)
 *  Author:           Copyright (C) 1999 Thomas Jensen <boxes@thomasjensen.com>
 *  Version:          $Id: tools.h,v 1.3 1999-08-31 08:38:42-07 tsjensen Exp tsjensen $
 *  Language:         ANSI C
 *  World Wide Web:   http://boxes.thomasjensen.com/
 *  Purpose:          Tool functions for error reporting and some string
 *                    handling and other needful things
 *
 *  Remarks: o This program is free software; you can redistribute it and/or
 *             modify it under the terms of the GNU General Public License as
 *             published by the Free Software Foundation; either version 2 of
 *             the License, or (at your option) any later version.
 *           o This program is distributed in the hope that it will be useful,
 *             but WITHOUT ANY WARRANTY; without even the implied warranty of
 *             MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *             GNU General Public License for more details.
 *           o You should have received a copy of the GNU General Public
 *             License along with this program; if not, write to the Free
 *             Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *             MA 02111-1307  USA
 *
 *  Revision History:
 *
 *    $Log: tools.h,v $
 *    Revision 1.3  1999-08-31 08:38:42-07  tsjensen
 *    Added concat_strings() function prototype from Joe Zbiciak's patches
 *
 *    Revision 1.2  1999/07/20 18:56:15  tsjensen
 *    Added GNU GPL disclaimer
 *    Removed include boxes.h
 *    Added prototypes for strisyes() and strisno()
 *
 *    Revision 1.1  1999/06/23 11:19:59  tsjensen
 *    Initial revision
 *
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#ifndef TOOLS_H
#define TOOLS_H


#define BMAX(a,b) ((a)>(b)? (a):(b))     /* return the larger value */

#define BFREE(p) {                       /* free memory and clear pointer */ \
   if (p) {           \
      free (p);       \
      (p) = NULL;     \
   }                  \
}


int    yyerror     (const char *fmt, ...);
void   regerror    (char *msg);
int    empty_line  (const line_t *line);
size_t expand_tabs_into (const char *input_buffer, const int in_len,
                    const int tabstop, char **text);
void   btrim       (char *text, size_t *len);
char*  my_strnrstr (const char *s1, const char *s2, const size_t s2_len,
                    int skip);
int strisyes (const char *s);
int strisno (const char *s);

void concat_strings (char *dst, int max_len, int count, ...);

#endif

/*EOF*/                                          /* vim: set cindent sw=4: */
