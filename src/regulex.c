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
 * Convenience functions for PCRE2 regular expression processing
 */

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boxes.h"
#include "logging.h"
#include "regulex.h"
#include "tools.h"
#include "unicode.h"



pcre2_code *compile_pattern(char *pattern)
{
    uint32_t *ustr = u32_strconv_from_arg(pattern, CONFIG_FILE_ENCODING);
    if (ustr == NULL) {
        bx_fprintf(stderr, "Failed to convert pattern string to UTF-32 - \"%s\"\n", pattern);
        return NULL;
    }
    pcre2_code *result = u32_compile_pattern(ustr);
    BFREE(ustr);
    return result;
}



pcre2_code *u32_compile_pattern(uint32_t *pattern)
{
    int errornumber;
    PCRE2_SIZE erroroffset;

    pcre2_code *re = pcre2_compile(
            (PCRE2_SPTR) pattern,    /* the pattern */
            PCRE2_ZERO_TERMINATED,   /* indicates pattern is zero-terminated */
            0,                       /* default options */
            &errornumber,
            &erroroffset,
            NULL);                   /* use default compile context */

    if (re == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        bx_fprintf(stderr, "Regular expression pattern \"%s\" failed to compile at position %d: %s\n",
                u32_strconv_to_output(pattern), (int) erroroffset, u32_strconv_to_output(buffer));
    }
    return re;
}



int regex_match(pcre2_code *pattern, char *subject_string)
{
    uint32_t *ustr = u32_strconv_from_arg(subject_string, "ASCII");
    int result = u32_regex_match(pattern, ustr);
    BFREE(ustr);
    return result;
}



int u32_regex_match(pcre2_code *pattern, uint32_t *subject_string)
{
    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(pattern, NULL);
    int rc = pcre2_match(pattern, (PCRE2_SPTR) subject_string, PCRE2_ZERO_TERMINATED, 0, 0, match_data, NULL);
    pcre2_match_data_free(match_data);
    return rc < 0 ? 0 : 1;
}



uint32_t *regex_replace(pcre2_code *search, char *replace, uint32_t *input, const size_t input_len, const int global)
{
    uint32_t *ustr = u32_strconv_from_arg(replace, CONFIG_FILE_ENCODING);
    if (ustr == NULL) {
        bx_fprintf(stderr, "Failed to convert replacement string to UTF-32 - \"%s\"\n", replace);
        return NULL;
    }
    uint32_t *result = u32_regex_replace(search, ustr, input, input_len, global);
    BFREE(ustr);
    return result;
}



uint32_t *u32_regex_replace(pcre2_code *search, uint32_t *replace, uint32_t *input, const size_t input_len,
        const int global)
{
    uint32_t options = PCRE2_SUBSTITUTE_OVERFLOW_LENGTH | PCRE2_SUBSTITUTE_EXTENDED
                        | (global ? PCRE2_SUBSTITUTE_GLOBAL : 0);
    PCRE2_SIZE outlen = input_len * 2; /* estimated length of output buffer in characters, fine if too small */

    PCRE2_SIZE bufsize = (input_len < 8) ? 16 : outlen;
    uint32_t *output = (uint32_t *) malloc(sizeof(uint32_t) * bufsize); /* output buffer */
    int pcre2_rc;

    int done = 0;
    while (!done) {
        if (output == NULL) {
            fprintf(stderr, "out of memory");
            return NULL;
        }
        PCRE2_SIZE outlen = bufsize;

        pcre2_rc = pcre2_substitute(search,
                (PCRE2_SPTR) input, PCRE2_ZERO_TERMINATED,
                0,             /* start offset */
                options,
                NULL,          /* ptr to a match data block */
                NULL,          /* match context */
                (PCRE2_SPTR) replace, PCRE2_ZERO_TERMINATED,
                output, &outlen);

        if (pcre2_rc != PCRE2_ERROR_NOMEMORY || bufsize >= outlen) {
            done = 1;
        }
        else {
            log_debug(__FILE__, REGEXP, "Reallocating output buffer from %ld to %ld UTF-32 chars\n", bufsize, outlen);
            bufsize = outlen;
            output = (uint32_t *) realloc(output, sizeof(uint32_t) * bufsize);
        }
    }

    if (pcre2_rc < 0) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(pcre2_rc, buffer, sizeof(buffer));
        /* buffer will normally contain "invalid replacement string" */
        bx_fprintf(stderr, "Error substituting \"%s\": %s\n", u32_strconv_to_output(replace),
                u32_strconv_to_output(buffer));
        BFREE(output);
        return NULL;
    }

    return output;
}


/* vim: set sw=4: */
