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
 * Tool functions for error reporting and some string handling and other needful things
 */

#ifndef TOOLS_H
#define TOOLS_H

#include <unitypes.h>

#include "boxes.h"



/** return the larger value */
#define BMAX(a, b) ({        \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a > _b ? _a : _b;       \
})

/** return the smaller value */
#define BMIN(a, b) ({        \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})

/** free memory and clear pointer */
#define BFREE(p) {          \
    if (p) {                \
        free((void *) (p)); \
        (p) = NULL;         \
    }                       \
}


/**
 * Define type for a function pointer to specify which `bx_fprintf` function
 * will be called. This enables unit testing on MacOS, since Apple's `ld`
 * linker does not support the `--wrap` flag that GNU `ld` does.
 */
typedef void (*bx_fprintf_t)(FILE *stream, const char *format, ...);

/**
 * Declare function pointer to be changed when running unit tests
 */
extern bx_fprintf_t bx_fprintf;


/*
 * Return true (1) if line is empty.
 * Empty lines either consist entirely of whitespace or don't exist.
 * @param line the line to check
 * @return 1 if line is empty, 0 if it isn't
 */
int empty_line(const line_t *line);


/**
 * Expand tab chars in `input_buffer` and store result in `text`.
 * Memory will be allocated for text and tabpos.
 * Should only be called for lines of length > 0;
 *
 * @param input_buffer Line of text with tab chars
 * @param tabstop tab stop distance as per command line options
 * @param text address of the pointer that will take the result of this function
 * @param tabpos array of ints giving the positions of the first space of an expanded tab in the text result buffer
 * @param tabpos_len number of tabs recorded in tabpos
 * @return Success: Length of the result line in characters (> 0);
 *         Error:   0       (e.g. out of memory)
 */
size_t expand_tabs_into(const uint32_t *input_buffer, const int tabstop, uint32_t **text,
                        size_t **tabpos, size_t *tabpos_len);


void btrim(char *text, size_t *len);


void btrim32(uint32_t *text, size_t *len);


/**
 * Calculates the length (in bytes) of the segment at the end of `s` which consists entirely of bytes in `accept`.
 * This is like `strspn()`, but from the end of the string.
 * @param s the string to search
 * @param accept acceptable characters that form the trailing segment
 * @return the number of bytes found as described above
 */
size_t my_strrspn(const char *s, const char *accept);


int strisyes(const char *s);


int strisno(const char *s);


/**
 * Concatenate variable number of strings into one. This would normally be achieved via snprintf(), but that's not
 * available on all platforms where boxes is compiled.
 * @param count number of strings given in the following
 * @param <...> the strings
 * @return a new string, for which new memory was allocated
 */
char *concat_strings_alloc(size_t count, ...);


/**
 * Repeat the string `s` `count` times.
 * @param s the string to repeat
 * @param count the number of times the string should be repeated
 * @return a new string, for which new memory was allocated, or NULL if `s` was NULL or out of memory
 */
char *repeat(char *s, size_t count);


/**
 * Checks if tab expansion mode is "keep", and if so, calculates a new indentation string based on the one given.
 * The new string contains tabs in their original positions.
 *
 * @param lineno index of the input line we are referring to
 * @param indentspc previously calculated "space-only" indentation string (may be NULL). This is only used when
 *                  opt.tabexp != 'k', in which case it will be used as the function result.
 * @param indentspc_len desired result length, measured in spaces only
 * @return if successful and opt.tabexp == 'k': new string;
 *         on error (invalid input or out of memory): NULL
 */
uint32_t *tabbify_indent(const size_t lineno, uint32_t *indentspc, const size_t indentspc_len);


char *nspaces(const size_t n);


void print_input_lines(const char *heading);


void analyze_line_ascii(input_t *input_ptr, line_t *line);


size_t count_invisible_chars(const uint32_t *s, size_t *num_esc, char **ascii, size_t **posmap);


/**
 * Determine if `csi` points at a CSI which can be considered a "reset" sequence.
 * https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_(Select_Graphic_Rendition)_parameters
 * We recognize `\x1b[0m`, and also 10, 39, 49, 59, and 75 instead of 0 as "resets", because they all serve to reset
 * the terminal to some default. Especially, lolcat uses `\x1b[39m` as reset CSI.
 * @param csi a string which might or might not be a CSI "reset" sequence (zero-terminated UTF-32 string)
 * @return 1 if it was a CSI "reset", 0 otherwise
 */
int is_csi_reset(const uint32_t *csi);


/**
 * Determine if the given `array` contains the given string (case-insensitive!).
 * @param array an array of strings to search
 * @param s the string to search
 * @returns != 0 if found, 0 if not found
 */
int array_contains0(char **array, const char *s);


int array_contains_bxs(bxstr_t **array, const size_t array_len, bxstr_t *s);


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


/**
 * Just calls `fprintf()` internally, but this function can be mocked in unit tests. So, it's a "mockable `fprintf()`".
 * @param stream Where to print, for example `stderr`
 * @param format the format string, followed by the arguments of the format string
 */
void bx_fprintf_original(FILE *stream, const char *format, ...);


/**
 * Set the bx_fprintf_ptr function pointer to point to a specific function
 */
void set_bx_fprintf(bx_fprintf_t bx_fprintf_function);

/**
 * Determine if the given string is an "ASCII ID", which means:
 * - It consists only of the letters `abcdefghijklmnopqrstuvwxyz-0123456789`. If not in strict mode, upper case A-Z
 *   and underscores are also allowed.
 * - The first letter is a-z (lower case). If not in strict mode, upper case A-Z is also allowed.
 * - The last character may not be a hyphen or underscore.
 * - No two hyphens or underscores, or mixture thereof, may appear in sequence.
 * - The entire string must not be the word `none`.
 * - Minimum length is 1 character.
 * @param s the string to check
 * @param strict flag indicating whether "strict checks" should be applied (1) or not (0)
 * @return flag (1 or 0)
 */
int is_ascii_id(bxstr_t *s, int strict);


/**
 * Open the file pointed to by `pathname` and associate a stream with it. Supports non-ASCII pathnames and encapsulates
 * the logic for different operating systems.
 * @param pathname the pathname of the file to open
 * @param mode a mode sequence like for standard `fopen()`
 * @return the file stream
 */
FILE *bx_fopens(bxstr_t *pathname, char *mode);


/**
 * Open the file pointed to by `pathname` and associate a stream with it. Supports non-ASCII pathnames and encapsulates
 * the logic for different operating systems.
 * @param pathname the pathname of the file to open, as a UTF-8 or ASCII (single byte) byte sequence
 * @param mode a mode sequence like for standard `fopen()`
 * @return the file stream
 */
FILE *bx_fopen(char *pathname, char *mode);


#endif

/* vim: set cindent sw=4: */
