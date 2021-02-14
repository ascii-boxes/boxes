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

#include "config.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <uniconv.h>
#include <unictype.h>
#include <unistr.h>

#include "boxes.h"
#include "tools.h"
#include "unicode.h"



const char *config_encoding = "ISO_8859-15";

/* effective character encoding of input and output text */
const char *encoding;

/* ucs4_t character '\t' (tab)  */
const ucs4_t char_tab = 0x00000009;

/* ucs4_t character ' '  (space) */
const ucs4_t char_space = 0x00000020;

/* ucs4_t character '\r' (carriage return) */
const ucs4_t char_cr = 0x0000000d;

/* ucs4_t character '\n' (newline) */
const ucs4_t char_newline = 0x0000000a;

/* ucs4_t character 0x1b (escape)  */
const ucs4_t char_esc = 0x0000001b;

/* ucs4_t character '\0' (zero) */
const ucs4_t char_nul = 0x00000000;



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



uint32_t *new_empty_string32()
{
    return (uint32_t *) calloc(1, sizeof(uint32_t));
}



uint32_t *advance_next32(const uint32_t *s, size_t *invis)
{
    if (is_empty(s)) {
        return (uint32_t *) s;
    }

    int ansipos = 0;
    (*invis) = 0;
    ucs4_t c;
    const uint32_t *rest = s;
    while ((rest = u32_next(&c, rest))) {
        if (ansipos == 0 && c == char_esc) {
            /* Found an ESC char, count it as invisible and move 1 forward in the detection of CSI sequences */
            (*invis)++;
            ansipos++;
        } else if (ansipos == 1 && (c == '[' || c == '(')) {
            /* Found '[' char after ESC. A CSI sequence has started. */
            (*invis)++;
            ansipos++;
        } else if (ansipos == 1 && c >= 0x40 && c <= 0x5f) {
            /* Found a byte designating the end of a two-byte escape sequence */
            (*invis)++;
            ansipos = 0;
            break;
        } else if (ansipos == 2) {
            /* Inside CSI sequence - Keep counting chars as invisible */
            (*invis)++;

            /* A char between 0x40 and 0x7e signals the end of an CSI or escape sequence */
            if (c >= 0x40 && c <= 0x7e) {
                ansipos = 0;
                break;
            }
        } else {
            break;
        }
    }
    return (uint32_t *) rest;
}



uint32_t *advance32(uint32_t *s, const size_t offset)
{
    if (is_empty(s)) {
        return new_empty_string32();
    }
    if (offset == 0) {
        return s;
    }

    size_t count = 0;                 /* the count of visible characters */
    int visible = 1;                  /* flag indicating whether the previous char was a visible char */
    const uint32_t *last_esc = NULL;  /* pointer to the start of the last escape sequence encountered */
    const uint32_t *rest = s;         /* pointer to the next character coming up */
    size_t step_invis = 0;            /* unused, but required for advance_next32() call */

    for (ucs4_t c = s[0]; c != char_nul; c = rest[0]) {
        if (c == char_esc) {
            last_esc = rest;
            visible = 0;
        } else {
            if (count++ == offset) {
                if (!visible && last_esc != NULL) {
                    return (uint32_t *) last_esc;
                }
                break;
            }
            visible = 1;
        }
        rest = advance_next32(rest, &step_invis);
    }
    return (uint32_t *) rest;         /* may point to zero terminator when offset too large */
}



uint32_t *u32_strconv_from_input(const char *src)
{
    return u32_strconv_from_arg(src, encoding);
}



uint32_t *u32_strconv_from_arg(const char *src, const char *sourceEncoding)
{
    if (src == NULL) {
        return NULL;
    }
    if (src[0] == '\0') {
        return new_empty_string32();
    }

    uint32_t *result = u32_strconv_from_encoding(
            src,                    /* the source string to convert */
            sourceEncoding,         /* the character encoding from which to convert */
            iconveh_question_mark); /* produce one question mark '?' per unconvertible character */

    if (result == NULL) {
        fprintf(stderr, "%s: failed to convert from '%s' to UTF-32: %s\n", PROJECT, sourceEncoding, strerror(errno));
    }
    return result;
}



char *u32_strconv_to_output(const uint32_t *src)
{
    return u32_strconv_to_arg(src, encoding);
}



char *u32_strconv_to_arg(const uint32_t *src, const char *targetEncoding)
{
    if (src == NULL) {
        return NULL;
    }
    if (is_empty(src)) {
        return strdup("");
    }

    char *result = u32_strconv_to_encoding(
            src,                    /* the source string to convert */
            targetEncoding,         /* the character encoding to which to convert */
            iconveh_question_mark); /* produce one question mark '?' per unconvertible character */

    if (result == NULL) {
        fprintf(stderr, "%s: failed to convert from UTF-32 to '%s': %s\n", PROJECT, targetEncoding, strerror(errno));
    }
    return result;
}



const char *check_encoding(const char *manual_encoding, const char *system_encoding)
{
    if (manual_encoding != NULL) {
        uint32_t *unicode = u32_strconv_from_encoding(" ", manual_encoding, iconveh_error);
        if (unicode != NULL) {
            BFREE(unicode);
            return manual_encoding;
        }
        fprintf(stderr, "%s: Invalid character encoding: %s - falling back to %s\n",
                PROJECT, manual_encoding, system_encoding);
        fflush(stderr);
    }
    return system_encoding;
}


/*EOF*/                                                  /* vim: set sw=4: */
