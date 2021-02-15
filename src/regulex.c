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
 * Convenience functions for PCRE2 regular expression processing
 */

#include "config.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boxes.h"
#include "tools.h"
#include "unicode.h"
#include "regulex.h"



pcre2_code *compile_pattern(char *pattern)
{
    int errornumber;
    PCRE2_SIZE erroroffset;
    PCRE2_SPTR pattern32 = u32_strconv_from_arg(pattern, config_encoding);
    if (pattern32 == NULL) {
        return NULL;
    }

    pcre2_code *re = pcre2_compile(
            pattern32,               /* the pattern */
            PCRE2_ZERO_TERMINATED,   /* indicates pattern is zero-terminated */
            0,                       /* default options */
            &errornumber,
            &erroroffset,
            NULL);                   /* use default compile context */

    if (re == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        fprintf(stderr, "Regular expression pattern \"%s\" failed to compile at offset %d: %s\n",
                pattern, (int) erroroffset, u32_strconv_to_output(buffer));
    }
    return re;
}



uint32_t *regex_replace(pcre2_code *search, char *replace, uint32_t *input, const size_t input_len, const int global)
{
    PCRE2_SPTR replacement = u32_strconv_from_arg(replace, config_encoding);
    if (replacement == NULL) {
        fprintf(stderr, "Failed to convert replacement string to UTF-32 - \"%s\"\n", replace);
        return NULL;
    }

    uint32_t options = PCRE2_SUBSTITUTE_OVERFLOW_LENGTH | PCRE2_SUBSTITUTE_EXTENDED
            | (global ? PCRE2_SUBSTITUTE_GLOBAL : 0);
    PCRE2_SIZE outlen = input_len * 2;     /* estimated length of output buffer in characters, fine if too small */

    PCRE2_SIZE bufsize = (input_len < 8) ? 16 : outlen;
    uint32_t *output = (uint32_t *) malloc(sizeof(uint32_t) * bufsize);   /* output buffer */
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
                                    0,         /* start offset */
                                    options,
                                    NULL,      /* ptr to a match data block */
                                    NULL,      /* match context */
                                    replacement, PCRE2_ZERO_TERMINATED,
                                    output, &outlen);

        if (pcre2_rc != PCRE2_ERROR_NOMEMORY || bufsize >= outlen) {
            done = 1;
        } else {
            #ifdef REGEXP_DEBUG
                fprintf(stderr, "Reallocating output buffer from %ld to %ld UTF-32 chars\n", bufsize, outlen);
            #endif
            bufsize = outlen;
            output = (uint32_t *) realloc(output, sizeof(uint32_t) * bufsize);
        }
    }
    BFREE(replacement);

    if (pcre2_rc < 0) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(pcre2_rc, buffer, sizeof(buffer));
        /* buffer will normally contain "invalid replacement string" */
        fprintf(stderr, "Error substituting \"%s\": %s\n", replace, u32_strconv_to_output(buffer));
        BFREE(output);
        return NULL;
    }

    return output;
}



/*EOF*/                                                  /* vim: set sw=4: */
