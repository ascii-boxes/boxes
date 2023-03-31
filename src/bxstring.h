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


/**
 * A boxes-internal string. Should be treated as immutable.
 */
typedef struct {
    /** Pointer to the original memory area for the string, NUL terminated */
    uint32_t *memory;

    /** ASCII version of the original string, tabs expanded, invisible characters removed, multi-byte chars replaced
     *  with one or more 'x'. NUL terminated. */
    char *ascii;

    /** Number of spaces at the beginning of the original string */
    size_t indent;

    /** Total number of screen columns required to display the string
     *  (important for double-wide characters such as Chinese) */
    size_t num_columns;

    /** Number of characters in the original string (visible + invisible) */
    size_t num_chars;

    /** Number of visible characters in the original string */
    size_t num_chars_visible;

    /** Number of invisible characters in the original string */
    size_t num_chars_invisible;

    /** Number of trailing spaces in the original string */
    size_t trailing;

    /** Array of index values into `memory` of the first actual character (possibly invisible) of each visible
     *  character. Length of this array is `num_chars_visible` + 1, as the last value is the index of the NUL
     *  terminator. */
    size_t *first_char;

    /** Array of index values into `memory` of the visible characters themselves.
     *  Length of this array is `num_chars_visible` + 1, as the last value is the index of the NUL terminator. */
    size_t *visible_char;
} bxstr_t;



/**
 * Construct a `bxstr_t` from an ASCII string.
 * @param pAscii the ASCII string, which does not contain tabs or ANSI escapes
 * @return a pointer to a new `bxstr_t` for which memory has been allocated
 */
bxstr_t *bxs_from_ascii(char *pAscii);


/**
 * Construct a `bxstr_t` from a Unicode string. When the string consists only of blanks, they count as indentation,
 * not as trailing blanks.
 * @param pInput the utf-8 encoded string, which may include ANSI escapes, but NOT tabs
 * @return a pointer to a new `bxstr_t` for which memory has been allocated,
 *      or NULL if `pInput` was NULL or an error occurred (such as tabs encountered)
 */
bxstr_t *bxs_from_unicode(uint32_t *pInput);


/**
 * Create an exact copy of a string.
 * @param pString the string to copy
 * @return the copied string, for which new memory was allocated
 */
bxstr_t *bxs_strdup(bxstr_t *pString);


/**
 * Take a substring from the given string, trim leading and trailing space from it, and duplicate the result in a new
 * string. If invisible characters are included in the string, they are also duplicated.
 * @param pString the source string
 * @param start_idx the index of the first visible character of the substring
 * @param end_idx the index of the first visible character following the substring
 * @return the trimmed and duplicated substring, for which new memory was allocated
 */
bxstr_t *bxs_trimdup(bxstr_t *pString, size_t start_idx, size_t end_idx);


/**
 * Combine `pString + pToAppend` into a new string. Memory for the input strings is NOT modified or freed.
 * @param pString the string to append to
 * @param pToAppend the string to append
 * @return the concatenated string, for which new memory was allocated
 */
bxstr_t *bxs_strcat(bxstr_t *pString, uint32_t *pToAppend);


/**
 * Return a pointer to the first visible occurrence of the character `c` in the string `pString`.
 * Invisible characters are ignored.
 * @param pString the string to search
 * @param c the character to find
 * @param cursor If specified, indicates the start position for the search (search will start on the next character,
 *      so the caller does not need to manage this value), and will be updated with the next position found (if found).
 *      On the first call, this value should point to -1. May be NULL, in which case the cursor feature is not used.
 * @return a pointer into the memory of `pString`, or NULL if the character was not found
 */
uint32_t *bxs_strchr(bxstr_t *pString, ucs4_t c, int *cursor);


/**
 * Create a new string from which all leading and trailing whitespace have been removed.
 * @param pString the string to trim, which will not be modified
 * @return a new, trimmed string
 */
bxstr_t *bxs_trim(bxstr_t *pString);


/**
 * Create a new string from which all trailing whitespace have been removed.
 * @param pString the string to trim, which will not be modified
 * @return a new, trimmed string
 */
bxstr_t *bxs_rtrim(bxstr_t *pString);


/**
 * Convert the string into boxes' output encoding for proper printing on stdout.
 * @param pString the string to convert
 * @return the same string in the target (output) encoding
 */
char *bxs_to_output(bxstr_t *pString);


/**
 * Determine if the given string is empty.
 * @param pString the string to check (may be NULL, which counts as empty)
 * @return 1 for empty, 0 for not empty
 */
int bxs_is_empty(bxstr_t *pString);


/**
 * Determine whether the given `pString` is a valid string under at least one condition. This will return `false` for
 * strings which should really never occur anywhere.
 * @param pString the string to check (may be NULL, which counts as invalid)
 * @param error_pos if invalid, this address is used to store the first invalid character position in the string;
 *      may be NULL, in which case the information is not provided
 * @return == 0: invalid; otherwise: valid
 */
int bxs_valid_anywhere(bxstr_t *pString, size_t *error_pos);


/**
 * Determine whether the given `pString` is a valid string representing a line of a shape in the config file.
 * @param pString the string to check (may be NULL, which counts as invalid)
 * @param error_pos if invalid, this address is used to store the first invalid character position in the string;
 *      may be NULL, in which case the information is not provided
 * @return == 0: invalid; otherwise: valid
 */
int bxs_valid_in_shape(bxstr_t *pString, size_t *error_pos);


/**
 * Determine whether the given `pString` is a valid string representing a complete SAMPLE block in the config file.
 * @param pString the string to check (may be NULL, which counts as invalid)
 * @param error_pos if invalid, this address is used to store the first invalid character position in the string;
 *      may be NULL, in which case the information is not provided
 * @return == 0: invalid; otherwise: valid
 */
int bxs_valid_in_sample(bxstr_t *pString, size_t *error_pos);


/**
 * Determine whether the given `pString` is a valid string representing a parent filename in the config file.
 * @param pString the string to check (may be NULL, which counts as invalid)
 * @param error_pos if invalid, this address is used to store the first invalid character position in the string;
 *      may be NULL, in which case the information is not provided
 * @return == 0: invalid; otherwise: valid
 */
int bxs_valid_in_filename(bxstr_t *pString, size_t *error_pos);


/**
 * Determine whether the given `pString` is a valid string representing a value from a key/value pair in the config
 * file.
 * @param pString the string to check (may be NULL, which counts as invalid)
 * @param error_pos if invalid, this address is used to store the first invalid character position in the string;
 *      may be NULL, in which case the information is not provided
 * @return == 0: invalid; otherwise: valid
 */
int bxs_valid_in_kv_string(bxstr_t *pString, size_t *error_pos);


/**
 * Compares `s1` and `s2` lexicographically. Returns a negative value if `s1` compares smaller than `s2`, a positive
 * value if `s1` compares larger than `s2`, or 0 if they compare equal. NULL values are considered "greather than"
 * non-NULL values.
 * This function is similar to `strcmp()`, except that it operates on `bxstr_t *` strings and supports NULL values.
 * @param s1 the first string (may be NULL)
 * @param s2 the second string (may be NULL)
 * @return comparison result as described above
 */
int bxs_strcmp(bxstr_t *s1, bxstr_t *s2);


/**
 * Free the memory allocated by the given `bxstr_t`.
 * @param pString the `bxstr_t` to free
 */
void bxs_free(bxstr_t *pString);


#endif

/* vim: set cindent sw=4: */
