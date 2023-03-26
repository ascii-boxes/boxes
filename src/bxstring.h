/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2023 Thomas Jensen and the boxes contributors
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License, version 3, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <https://www.gnu.org/licenses/>.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

/*
 * The boxes-internal representation of strings.
 */

#ifndef BXSTRING_H
#define BXSTRING_H

#include <unitypes.h>


typedef struct {
    uint32_t *memory;       /* Immutable. Pointer to the original memory area for the string, NUL terminated */
    char     *ascii;        /* Immutable. ASCII version of the original string, tabs expanded, invisible characters removed, multi-byte chars replaced with one or more 'x'. NUL terminated. CHECK remove this eventually */
    size_t    indent;       /* Immutable. Number of spaces at the beginning of the original string */
    size_t    num_columns;  /* Immutable. Total number of screen columns required to display the string (important for double-wide characters such as Chinese) */
    size_t    num_chars;    /* Immutable. Number of characters in the original string (visible + invisible) */
    size_t    num_chars_visible;   /* Immutable. Number of visible characters in the original string */
    size_t    num_chars_invisible; /* Immutable. Number of invisible characters in the original string */
    size_t    trailing;     /* Immutable. Number of trailing spaces in the original string */
    size_t   *first_char;   /* Immutable. Array of index values into `memory` of the first actual character (possibly invisible) of each visible character */
    size_t   *visible_char; /* Immutable. Array of index values into `memory` of the visible characters themselves */
    int       offset_start; /* Number of visible characters to cut from the beginning of the string. Must be smaller than or equal to `indent`. Can be negative, in which case spaces will be prepended. */
    int       offset_end;   /* Number of visible characters to cut from the end of the string. Must be smaller than or equal to `trailing`. Can be negative, in which case spaces will be appended. */
} bxstr_t;


/**
 * Construct a `bxstr_t` from an ASCII string.
 * @param pAscii the ASCII string, which does not contain tabs or ANSI escapes
 * @return a pointer to a new `bxstr_t` for which memory has been allocated
 */
bxstr_t *bxs_from_ascii(char *pAscii);


/**
 * Construct a `bxstr_t` from a Unicode string.
 * @param pInput the utf-8 encoded string, which may include ANSI escapes, but NOT tabs
 * @return a pointer to a new `bxstr_t` for which memory has been allocated,
 *      or NULL if `pInput` was NULL or an error occurred (such as tabs encountered)
 */
bxstr_t *bxs_from_unicode(uint32_t *pInput);


/**
 * Free the memory allocated by the given `bxstr_t`.
 * @param pString the `bxstr_t` to free
 */
void bxs_free(bxstr_t *pString);


/* TODO */

#endif

/*EOF*/                                          /* vim: set cindent sw=4: */
