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

void analyze_line_ascii(input_t *input_ptr, line_t *line);

int array_contains(char **array, const size_t array_len, const char *s);

/**
 * Determine if the given `array` contains the given string (case-insensitive!).
 * @param array an array of strings to search
 * @param s the string to search
 * @returns != 0 if found, 0 if not found
 */
int array_contains0(char **array, const char *s);

/**
 * Count the number of elements in a zero-terminated array.
 * @param array a zero-terminated array of strings (can be NULL)
 * @returns the number of elements, excluding the zero terminator
 */
size_t array_count0(char **array);

/**
 * Trim leading and trailing whitespace from a string and return the result in a new string, for which memory is
 * allocated. `s` and `e` may point into some other string, which will not be modified. `e` should be greater than `s`.
 * @param s pointer to the first character of the string
 * @param e pointer to the last character of the string (NOT the character AFTER the last)
 * @returns a new string, which may be the empty string if the input was invalid or consisted only of whitespace
 */
char *trimdup(char *s, char *e);

/**
 * Determine if the given string is a valid tag. Valid tags are lower-case alphanumeric strings which may have
 * intermixed, single hyphens. A valid tag starts with a letter. Hyphens may also not appear as last character.
 * @param tag a potential tag
 * @return flag indicating validity
 */
int tag_is_valid(char *tag);


/**
 * Duplicate at most `n` bytes from the given string `s`.  Memory for the new string is obtained with `malloc()`, and
 * can be freed with `free()`. A terminating null byte is added. We include this implementation because the libc's 
 * `strndup()` is not consistently available across all platforms.
 * @param s a string
 * @param n maximum number of characters to copy (excluding the null byte)
 * @returns a new string, or `NULL` if either `s` was also `NULL`, or out of memory
 */
char *bx_strndup(const char *s, size_t n);


#endif

/*EOF*/                                          /* vim: set cindent sw=4: */
