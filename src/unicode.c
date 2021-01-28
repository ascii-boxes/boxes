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
 * Functions and constants for handling unicode strings with libunistring.
 */

#include "config.h"
#include <errno.h>
#include <stdlib.h>

#include <unictype.h>
#include <unistr.h>
#include <unitypes.h>

#include "unicode.h"


const char *encoding;                          /* the character encoding that we use */

const ucs4_t char_tab     = 0x00000009;        /* ucs4_t character '\t' (tab)  */
const ucs4_t char_space   = 0x00000020;        /* ucs4_t character ' '  (space) */
const ucs4_t char_cr      = 0x0000000d;        /* ucs4_t character '\r' (carriage return) */
const ucs4_t char_newline = 0x0000000a;        /* ucs4_t character '\n' (newline) */
const ucs4_t char_nul     = 0x00000000;        /* ucs4_t character '\0' (zero) */



/**
 * Check whether the character at the given index has the given value.
 *
 * @param <text> the string to check
 * @param <idx> the index position of the character to check
 * @param <expected_char> the expected character value
 * @return flag indicating whether the character has the expected value
 */
int is_char_at(const uint32_t *text, const size_t idx, const ucs4_t expected_char)
{
    return text != NULL && u32_cmp(text + idx, &expected_char, 1) == 0;
}



/**
 * Set the character at the given index to the given value.
 *
 * @param <text> the string to modify
 * @param <idx> the index position of the character to modify
 * @param <char_to_set> the new character value
 */
void set_char_at(uint32_t *text, const size_t idx, const ucs4_t char_to_set)
{
    u32_set(text + idx, char_to_set, 1);
}



/**
 *  Determine if a string is NULL/empty or not.
 *
 *  @param <text> the string to check
 *  @return > 0: the string is empty or NULL
 *            0: the string contains at least 1 character
 */
int is_empty(const uint32_t *text)
{
    return text == NULL || is_char_at(text, 0, char_nul);
}



int is_ascii_printable(const ucs4_t c)
{
    return c >= 0x20 && c < 0x7f;
}



/*EOF*/                                                  /* vim: set sw=4: */
