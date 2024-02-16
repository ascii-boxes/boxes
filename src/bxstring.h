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
 * The boxes-internal representation of strings.
 */

#ifndef BXSTRING_H
#define BXSTRING_H

#include <unitypes.h>


/**
 * A boxes-internal string. Should be treated as immutable, although some functions DO modify an instance. At the very
 * least, make sure never to change the values of an instance from outside this module.
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

    /** Number of trailing spaces in the original string. In a line consisting only of spaces, this is zero, because
     *  the spaces would count towards the indent. */
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
 * Return a freshly allocated empty string.
 * @return a new empty string
 */
bxstr_t *bxs_new_empty_string();


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
 * @param start_idx the index of the first visible character of the substring, used as index into `first_char` and
 *      `visible_char`
 * @param end_idx the index of the first visible character following the substring
 * @return the trimmed and duplicated substring, for which new memory was allocated
 */
bxstr_t *bxs_trimdup(bxstr_t *pString, size_t start_idx, size_t end_idx);


/**
 * Take a substring from the given string.
 * @param pString the source string
 * @param start_idx the index of the first character (visible or invisible) of the substring, an index into `memory`
 * @param end_idx the index of the first character (visible or invisible) following the substring
 * @return the substring, in new memory
 */
bxstr_t *bxs_substr(bxstr_t *pString, size_t start_idx, size_t end_idx);


/**
 * Combine `pString + pToAppend` into a new string. Memory for the input strings is NOT modified or freed.
 * @param pString the string to append to
 * @param pToAppend the string to append
 * @return the concatenated string, for which new memory was allocated
 */
bxstr_t *bxs_strcat(bxstr_t *pString, uint32_t *pToAppend);


/**
 * Concatenate all given strings into a new string.
 * @param count number of strings given in the following
 * @param <...> the strings, all of them `uint32_t *`. NULL is discouraged as an argument, but will probably be
 *      printed as "(NULL)". (That's what libunistring did in our tests, but they promise nothing in their docs.)
 * @return a new string, for which new memory was allocated, might be empty, but never NULL
 */
bxstr_t *bxs_concat(size_t count, ...);


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
 * Remove the first `n` visible characters from the given string. Invisible characters are additionally removed where
 * they are associated with the removed visible characters.
 * @param pString the string to shorten
 * @param n the number of visible characters to cut from the front of the string. If this is greater than the total
 *          number of visible characters in the string, will lead to an empty string being produced
 * @return a new, shortened string, or NULL only if `pString` was NULL
 */
bxstr_t *bxs_cut_front(bxstr_t *pString, size_t n);


/**
 * Return the first character of the visible character with index `n` in the given string's `memory`.
 * @param pString the string to use
 * @param n the index of the visible character (zero-based)
 * @return a pointer into existing memory
 */
uint32_t *bxs_first_char_ptr(bxstr_t *pString, size_t n);


/**
 * Return a pointer to the NUL terminator of the given string's `memory`.
 * @param pString the string to use
 * @return a pointer into existing memory
 */
uint32_t *bxs_last_char_ptr(bxstr_t *pString);


/**
 * Return the first character of the visible character directly following the indent (for example a letter following
 * some spaces).
 * @param pString the string to use
 * @return a pointer into existing memory
 */
uint32_t *bxs_unindent_ptr(bxstr_t *pString);


/**
 * Create a new string from which all leading and trailing whitespace have been removed.
 * @param pString the string to trim, which will not be modified
 * @return a new, trimmed string
 */
bxstr_t *bxs_trim(bxstr_t *pString);


/**
 * Create a new string from which up to `max` leading spaces have been removed along with associated invisible
 * characters.
 * @param pString the string to trim, which will not be modified
 * @param max the maximum number of leading spaces to remove
 * @return a new, trimmed string
 */
uint32_t *bxs_ltrim(bxstr_t *pString, size_t max);


/**
 * Create a new string from which all trailing whitespace have been removed.
 * @param pString the string to trim, which will not be modified
 * @return a new, trimmed string
 */
bxstr_t *bxs_rtrim(bxstr_t *pString);


/**
 * Prepend `n` spaces to the start of the given string. New memory is allocated for the result.
 * @param pString the original string (will not be modified, can be NULL, in which case spaces only in result)
 * @param n the number of spaces to prepend
 * @returns spaces plus original string, in new memory. Will be NULL only of `pString` == NULL and `n` == 0.
 */
bxstr_t *bxs_prepend_spaces(bxstr_t *pString, size_t n);


/**
 * Append `n` spaces to the end of the given string. The given string is *modified* accordingly.
 * @param pString the string to modify
 * @param n the number of spaces to add
 */
void bxs_append_spaces(bxstr_t *pString, size_t n);


/**
 * Convert the string into boxes' output encoding for proper printing on stdout.
 * @param pString the string to convert
 * @return the same string in the target (output) encoding, for which new memory has been allocated
 */
char *bxs_to_output(bxstr_t *pString);


/**
 * Determine if the given string is empty, which means it does not contain any characters at all (neither visible nor
 * invisible).
 * @param pString the string to check (may be NULL, which counts as empty)
 * @return 1 for empty, 0 for not empty
 */
int bxs_is_empty(bxstr_t *pString);


/**
 * Determine if the line is either empty or its visible characters are all whitespace.
 * @param pString the string to check (may be NULL, which counts as blank)
 * @return 1 for blank, 0 for not blank
 */
int bxs_is_blank(bxstr_t *pString);


/**
 * Determine if the character at position `idx` in the given `pString` is a visible character. If `idx` is out of
 * bounds, this function will return 0.
 * @param pString the string to check (may be NULL, which means no visible characters)
 * @param idx the index in `pString` of the character to check (zero-based)
 * @return 1 for visible, 0 for not visible
 */
int bxs_is_visible_char(bxstr_t *pString, size_t idx);


/**
 * Filter the given string so that only the visible characters are returned.
 * @param pString the string to filter (may be NULL, in which case NULL is returned)
 * @return a new UTF-32 string which contains only the visible characters from the given string, or NULL when the
 *      given string was NULL
 */
uint32_t *bxs_filter_visible(bxstr_t *pString);


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
