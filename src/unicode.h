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
 * Functions and constants for handling unicode strings with libunistring.
 */

#ifndef UNICODE_H
#define UNICODE_H

#include <unitypes.h>



/** The boxes config file is still encoded with a single-byte character set. Officially, it is ASCII!
 *  However, people might not conform to this, so we use ISO_8859-15 as a reasonable superset. */
extern const char *config_encoding;

/** the character encoding of input (and output) text */
extern const char *encoding;

/** ucs4_t character '\t' (tab)  */
extern const ucs4_t char_tab;

/** ucs4_t character ' '  (space) */
extern const ucs4_t char_space;

/** ucs4_t character '\r' (carriage return) */
extern const ucs4_t char_cr;

/** ucs4_t character '\n' (newline) */
extern const ucs4_t char_newline;

/** ucs4_t character 0x1b (escape)  */
extern const ucs4_t char_esc;

/** ucs4_t character '\0' (zero) */
extern const ucs4_t char_nul;

int is_char_at(const uint32_t *text, const size_t idx, const ucs4_t expected_char);

void set_char_at(uint32_t *text, const size_t idx, const ucs4_t char_to_set);

int is_empty(const uint32_t *text);

int is_ascii_printable(const ucs4_t c);

/** Return a freshly allocated empty UTF-32 string. */
uint32_t *new_empty_string32();

/**
 * Return the next position in <s> in accordance with escape sequences. The result can be the next normal character,
 * or again an escape sequence, if it directly follows the first.
 *
 * @param <s> The pointer to the start position. Is assumed to point either at the ESC at the start of an escape
 *      sequence, or to be positioned outside an escape sequence.
 * @param <invis> Will contain the number of invisible characters skipped in order to get to the new position.
 *      This will be 0 unless <s> pointed to an ESC char, in which case it contains the length in characters of that
 *      escape sequence.
 * @return The next position, or 0 if the end of the string was reached
 */
uint32_t *advance_next32(const uint32_t *s, size_t *invis);

/**
 * Determine a new position in the given string s with the given offset of visible characters.
 * If the character right in front of the target character is invisible, then the pointer is moved to the start of
 * that invisible sequence. The purpose is to catch any escape sequences which would for example color the character.
 *
 * @param <s> The pointer to the start position. Is assumed to point either at the ESC at the start of an escape
 *      sequence, or to be positioned outside an escape sequence.
 * @param <offset> the number of visible character positions to advance the pointer
 * @return a pointer to the new position in s, or 0 if the end of the string was reached
 */
uint32_t *advance32(uint32_t *s, const size_t offset);

/**
 * Convert a string from the input/output encoding (`encoding` in this .h file) to UTF-32 internal representation.
 * Memory will be allocated for the converted string.
 *
 * @param <src> string to convert, zero-terminated
 * @return UTF-32 string, or NULL in case of error (then an error message was already printed on stderr)
 */
uint32_t *u32_strconv_from_input(const char *src);

/**
 * Convert a string from the given source encoding to UTF-32 internal representation.
 * Memory will be allocated for the converted string.
 *
 * @param <src> string to convert, zero-terminated
 * @param <sourceEncoding> the character encoding of <src>
 * @return UTF-32 string, or NULL in case of error (then an error message was already printed on stderr)
 */
uint32_t *u32_strconv_from_arg(const char *src, const char *sourceEncoding);

/**
 * Convert a string from UTF-32 internal representation to input/output encoding (`encoding` in this .h file).
 * Memory will be allocated for the converted string.
 *
 * @param <src> UTF-32 string to convert, zero-terminated
 * @return string in input/output encoding, or NULL on error (then an error message was already printed on stderr)
 */
char *u32_strconv_to_output(const uint32_t *src);

/**
 * Convert a string from UTF-32 internal representation to the given target encoding.
 * Memory will be allocated for the converted string.
 *
 * @param <src> UTF-32 string to convert, zero-terminated
 * @param <targetEncoding> the character encoding of the result
 * @return string in target encoding, or NULL in case of error (then an error message was already printed on stderr)
 */
char *u32_strconv_to_arg(const uint32_t *src, const char *targetEncoding);

/**
 * Check if the given <manual_encoding> can be used to covert anything. This should reveal invalid encoding names that
 * have been specified on the command line. If no <manual_encoding> was specified, or if an invalid encoding is
 * detected, we fall back to the system encoding. No new memory is allocated.
 *
 * @param <manual_encoding> the encoding set on the command line, may be NULL
 * @param <system_encoding> the system encoding
 * @return <manual_encoding> if it is set to a valid value, <system_encoding> otherwise
 */
const char *check_encoding(const char *manual_encoding, const char *system_encoding);


#endif

/*EOF*/                                          /* vim: set cindent sw=4: */
