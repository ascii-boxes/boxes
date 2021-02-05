/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (C) 1999  Thomas Jensen and the boxes contributors
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License, version 2, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/*
 * Tool functions for error reporting and some string handling and other needful things
 */

#ifndef TOOLS_H
#define TOOLS_H

#include <unitypes.h>

#include "boxes.h"


#define BMAX(a, b) ((a)>(b)? (a):(b))    /* return the larger value */

#define BFREE(p) {                       /* free memory and clear pointer */ \
    if (p) {              \
        free((void *) p); \
        (p) = NULL;       \
    }                     \
}


int yyerror(const char *fmt, ...);

void regerror(char *msg);

int empty_line(const line_t *line);

size_t expand_tabs_into(const uint32_t *input_buffer, const int tabstop, uint32_t **text,
                        size_t **tabpos, size_t *tabpos_len);

void btrim(char *text, size_t *len);

void btrim32(uint32_t *text, size_t *len);

char *my_strnrstr(const char *s1, const char *s2, const size_t s2_len,
                  int skip);

int strisyes(const char *s);

int strisno(const char *s);

void concat_strings(char *dst, int max_len, int count, ...);

char *tabbify_indent(const size_t lineno, char *indentspc, const size_t indentspc_len);

char *nspaces(const size_t n);


#endif

/*EOF*/                                          /* vim: set cindent sw=4: */
