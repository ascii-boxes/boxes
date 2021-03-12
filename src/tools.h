/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2021 Thomas Jensen and the boxes contributors
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



#define BMAX(a, b) ({                    /* return the larger value */ \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

#define BFREE(p) {                       /* free memory and clear pointer */ \
    if (p) {              \
        free((void *) p); \
        (p) = NULL;       \
    }                     \
}


int empty_line(const line_t *line);

size_t expand_tabs_into(const uint32_t *input_buffer, const int tabstop, uint32_t **text,
                        size_t **tabpos, size_t *tabpos_len);

void btrim(char *text, size_t *len);

void btrim32(uint32_t *text, size_t *len);

char *my_strnrstr(const char *s1, const char *s2, const size_t s2_len,
                  int skip);

int strisyes(const char *s);

int strisno(const char *s);

/**
 * Concatenate variable number of strings into one. This would normally be achieved via snprintf(), but that's not
 * available on all platforms where boxes is compiled.
 * @param <count> number of strings given in the following
 * @param <...> the strings
 * @returns a new string, for which new memory was allocated
 */
char *concat_strings_alloc(size_t count, ...);

void concat_strings(char *dst, int max_len, int count, ...);

char *tabbify_indent(const size_t lineno, char *indentspc, const size_t indentspc_len);

char *nspaces(const size_t n);

void print_input_lines(const char *heading);

void analyze_line_ascii(line_t *line);

int array_contains(char **array, const size_t array_len, const char *s);


#endif

/*EOF*/                                          /* vim: set cindent sw=4: */
