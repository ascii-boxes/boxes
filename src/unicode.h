/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (c) 1999-2024 Thomas Jensen and the boxes contributors
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
 * Functions and constants for handling unicode strings with libunistring.
 */

#ifndef UNICODE_H
#define UNICODE_H

#include <unitypes.h>



/** Character encosing of the boxes configuration file */
#define CONFIG_FILE_ENCODING "UTF-8"

/* effective character encoding of input and output text */
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


/**
 * Check whether the character at the given index has the given value.
 *
 * @param text the string to check
 * @param idx the index position of the character to check
 * @param expected_char the expected character value
 * @return flag indicating whether the character has the expected value
 */
int is_char_at(const uint32_t *text, const size_t idx, const ucs4_t expected_char);


/**
 * Set the character at the given index to the given value.
 *
 * @param text the string to modify
 * @param idx the index position of the character to modify
 * @param char_to_set the new character value
 */
void set_char_at(uint32_t *text, const size_t idx, const ucs4_t char_to_set);


/**
 *  Determine if a string is NULL/empty or not.
 *
 *  @param text the string to check
 *  @return > 0: the string is empty or NULL;
 *         == 0: the string contains at least 1 character
 */
int is_empty(const uint32_t *text);


/**
 *  Determine if a string consists entirely of blanks.
 *
 *  @param text the string to check, NULL/empty strings count as blank
 *  @return > 0: the string is blank, empty, or NULL;
 *         == 0: the string contains at least 1 non-blank character
 */
int u32_is_blank(const uint32_t *text);


int is_ascii_printable(const ucs4_t c);


/**
 * Determine if the character can occur in a boxes string under at least one condition. This will return false for
 * characters which can really never occur anywhere.
 * @param c the character to check
 * @return 0 for `false` or non-zero for `true`
 */
int is_allowed_anywhere(const ucs4_t c);


int is_allowed_in_shape(const ucs4_t c);


int is_allowed_in_sample(const ucs4_t c);


int is_allowed_in_filename(const ucs4_t c);


int is_allowed_in_kv_string(const ucs4_t c);


/**
 * Determine if the given character represents a blank or something else that is rendered thusly (like a tab). This
 * would be variations of space, including Unicode (for example em-space), plus tab. CR and LF are *not* blanks.
 * @param c a Unicode character
 * @return 1 if it's a blank, 0 otherwise
 */
int is_blank(const ucs4_t c);


/**
 * Return a freshly allocated empty UTF-32 string.
 * @return a new empty string
 */
uint32_t *new_empty_string32();


/**
 * Convert an ASCII character into a Unicode character.
 * @param ascii a printable ASCII character in the range 0x20 - 0x7E
 * @return the corresponding Unicode character, or NUL if `ascii` out of range
 */
ucs4_t to_utf32(char ascii);


/**
 * Return the next position in `s` in accordance with escape sequences. The result can be the next normal character,
 * or again an escape sequence, if it directly follows the first.
 *
 * @param s The pointer to the start position. Is assumed to point either at the ESC at the start of an escape
 *      sequence, or to be positioned outside an escape sequence.
 * @param invis Will contain the number of invisible characters skipped in order to get to the new position.
 *      This will be 0 unless `s` pointed to an ESC char, in which case it contains the length in characters of that
 *      escape sequence.
 * @return The next position, or a pointer to the terminating 0 if the end of the string was reached
 */
uint32_t *advance_next32(const uint32_t *s, size_t *invis);


/**
 * Convert a string from the input/output encoding (`encoding` in this .h file) to UTF-32 internal representation.
 * Memory will be allocated for the converted string.
 *
 * @param src string to convert, zero-terminated
 * @return UTF-32 string, or NULL in case of error (then an error message was already printed on stderr)
 */
uint32_t *u32_strconv_from_input(const char *src);


/**
 * Convert a string from the given source encoding to UTF-32 internal representation.
 * Memory will be allocated for the converted string.
 *
 * @param src string to convert, zero-terminated
 * @param sourceEncoding the character encoding of `src`
 * @return UTF-32 string, or NULL in case of error (then an error message was already printed on stderr)
 */
uint32_t *u32_strconv_from_arg(const char *src, const char *sourceEncoding);


/**
 * Convert a string from UTF-32 internal representation to input/output encoding (`encoding` in this .h file).
 * Memory will be allocated for the converted string.
 *
 * @param src UTF-32 string to convert, zero-terminated
 * @return string in input/output encoding, or NULL on error (then an error message was already printed on stderr)
 */
char *u32_strconv_to_output(const uint32_t *src);


/**
 * Convert a string from UTF-32 internal representation to the given target encoding.
 * Memory will be allocated for the converted string.
 *
 * @param src UTF-32 string to convert, zero-terminated
 * @param targetEncoding the character encoding of the result
 * @return string in target encoding, or NULL in case of error (then an error message was already printed on stderr)
 */
char *u32_strconv_to_arg(const uint32_t *src, const char *targetEncoding);


/**
 * Check if the given `manual_encoding` can be used to covert anything. This should reveal invalid encoding names that
 * have been specified on the command line. If no `manual_encoding` was specified, or if an invalid encoding is
 * detected, we fall back to the system encoding. No new memory is allocated.
 *
 * @param manual_encoding the encoding set on the command line, may be NULL
 * @param system_encoding the system encoding
 * @return `manual_encoding` if it is set to a valid value, `system_encoding` otherwise
 */
const char *check_encoding(const char *manual_encoding, const char *system_encoding);


/**
 * Convert the given UTF-32 string into UTF-8 as `char *` byte sequence. The conversion must be fully successful, or
 * an error will be returned (no question marks will be generated in the output).
 * @param src the UTF-32 string
 * @return a byte sequence in UTF-8 encoding
 */
char *to_utf8(uint32_t *src);


/**
 * Create a new string of `n` spaces.
 * @param n number of spaces
 * @return a new string of length `n`, for which new memory was allocated
 */
uint32_t *u32_nspaces(const size_t n);


/**
 * Return pointer to last occurrence of string `needle` in string `haystack`.
 * @param haystack string to search
 * @param needle string to search for in `haystack`
 * @param needle_len length in characters of `needle`
 * @return pointer to last occurrence of string `needle` in string `haystack`; NULL if not found or error
 */
uint32_t *u32_strnrstr(const uint32_t *haystack, const uint32_t *needle, const size_t needle_len);


/**
 * Insert `n` spaces at position `idx` into `*s`. This modifies the given string.
 * @param s **pointer to** the string to modify. `*s` will be `realloc()`ed if needed.
 * @param idx the position at which to insert. The character previously at this position will move to the right.
 * @param n the number of spaces to insert
 */
void u32_insert_space_at(uint32_t **s, const size_t idx, const size_t n);


#endif

/* vim: set cindent sw=4: */
