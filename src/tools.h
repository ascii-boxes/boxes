/*
 *  File:             tools.h
 *  Project Main:     boxes.c
 *  Date created:     June 20, 1999 (Sunday, 16:57h)
 *  Author:           Copyright (C) 1999 Thomas Jensen <boxes@thomasjensen.com>
 *  Language:         ANSI C
 *  Web Site:         http://boxes.thomasjensen.com/
 *  Purpose:          Tool functions for error reporting and some string
 *                    handling and other needful things
 *
 *  License: o This program is free software; you can redistribute it and/or
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
size_t expand_tabs_into (const char *input_buffer, const size_t in_len,
           const int tabstop, char **text, size_t **tabpos, size_t *tabpos_len);
void   btrim       (char *text, size_t *len);
char*  my_strnrstr (const char *s1, const char *s2, const size_t s2_len,
                    int skip);
int strisyes (const char *s);
int strisno (const char *s);

void concat_strings (char *dst, int max_len, int count, ...);

char *tabbify_indent (const size_t lineno, char *indentspc, const size_t indentspc_len);

#endif

/*EOF*/                                          /* vim: set cindent sw=4: */
