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
 * Convenience functions for PCRE2 regular expression processing
 */

#ifndef REGULEX_H
#define REGULEX_H

/* Building under Windows: If you want to statically link this program against a non-dll .a file, you must define
 * PCRE2_STATIC before including pcre2.h. */
#ifdef __MINGW32__
    #define PCRE2_STATIC
#endif

/* The PCRE2_CODE_UNIT_WIDTH macro must be defined before including pcre2.h. For a program that uses only one code unit
 * width, setting it to 8, 16, or 32 makes it possible to use generic function names such as pcre2_compile(). Note that
 * just changing 8 to 16 (for example) is not sufficient to convert this program to process 16-bit characters. Even in
 * a fully 16-bit environment, where string-handling functions such as strcmp() and printf() work with 16-bit
 * characters, the code for handling the table of named substrings will still need to be modified. */
#define PCRE2_CODE_UNIT_WIDTH 32


#include <pcre2.h>
#include <unitypes.h>



/**
 * Compile the given pattern into a PCRE2 regular expression.
 * @param pattern the pattern to compile
 * @return the compiled pattern
 */
pcre2_code *compile_pattern(char *pattern);


/**
 * Compile the given pattern into a PCRE2 regular expression.
 * @param pattern the pattern to compile
 * @return the compiled pattern
 */
pcre2_code *u32_compile_pattern(uint32_t *pattern);


/**
 * Determine if the given `subject_string` matches the given `pattern`.
 * @param pattern the compiled pattern
 * @param subject_string the string to check
 * @return flag indicating a match (0 == no match, otherwise: match)
 */
int regex_match(pcre2_code *pattern, char *subject_string);


/**
 * Determine if the given `subject_string` matches the given `pattern`.
 * @param pattern the compiled pattern
 * @param subject_string the string to check, in UTF-32
 * @return flag indicating a match (0 == no match, otherwise: match)
 */
int u32_regex_match(pcre2_code *pattern, uint32_t *subject_string);


/**
 * Perform a regex replacement on the given string.
 *
 *  @param search the compiled pattern to search for
 *  @param replace the replacement string
 *  @param input the string to which the replacements shall be applied
 *  @param input_len the length of <input> in characters, not bytes
 *  @param global flag indicating whether all occurrences shall be replaced (true) or just the first (false)
 *  @return a new string which is a copy of output with the replacements applied, or NULL on error
 */
uint32_t *regex_replace(pcre2_code *search, char *replace, uint32_t *input, const size_t input_len, const int global);


/**
 * Perform a regex replacement on the given string.
 *
 *  @param search the compiled pattern to search for
 *  @param replace the replacement string
 *  @param input the string to which the replacements shall be applied
 *  @param input_len the length of <input> in characters, not bytes
 *  @param global flag indicating whether all occurrences shall be replaced (true) or just the first (false)
 *  @return a new string which is a copy of output with the replacements applied, or NULL on error
 */
uint32_t *u32_regex_replace(pcre2_code *search, uint32_t *replace, uint32_t *input, const size_t input_len,
        const int global);


#endif

/*EOF*/                                          /* vim: set cindent sw=4: */
