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

#ifndef UNICODE_H
#define UNICODE_H

extern const char *encoding;                   /* the character encoding that we use */

extern const ucs4_t char_tab;                  /* ucs4_t character '\t' (tab)  */
extern const ucs4_t char_space;                /* ucs4_t character ' '  (space) */
extern const ucs4_t char_cr;                   /* ucs4_t character '\r' (carriage return) */
extern const ucs4_t char_newline;              /* ucs4_t character '\n' (newline) */
extern const ucs4_t char_nul;                  /* ucs4_t character '\0' (zero) */



int is_char_at(const uint32_t *text, const size_t idx, const ucs4_t expected_char);

void set_char_at(uint32_t *text, const size_t idx, const ucs4_t char_to_set);

int is_empty(const uint32_t *text);

int is_ascii_printable(const ucs4_t c);


#endif

/*EOF*/                                          /* vim: set cindent sw=4: */
