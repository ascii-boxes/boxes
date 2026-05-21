/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * SPDX-FileCopyrightText: Copyright (c) 1999-2026 Thomas Jensen and the boxes contributors
 * SPDX-License-Identifier: GPL-3.0-only
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

#include "config.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <uniconv.h>
#include <unictype.h>
#include <unistr.h>

#include "boxes.h"
#include "tools.h"
#include "unicode.h"



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



#ifdef _WIN32
static const char *get_locale_from_environment()
{
    const char *locale = getenv("LC_ALL");
    if (locale == NULL || locale[0] == '\0') {
        locale = getenv("LC_CTYPE");
    }
    if (locale == NULL || locale[0] == '\0') {
        locale = getenv("LANG");
    }
    return locale != NULL && locale[0] != '\0' ? locale : NULL;
}


static int is_all_digits(const char *s)
{
    if (s == NULL || s[0] == '\0') {
        return 0;
    }
    for (const char *p = s; *p != '\0'; p++) {
        if (*p < '0' || *p > '9') {
            return 0;
        }
    }
    return 1;
}


static const char *extract_locale_encoding(const char *locale)
{
    static char encoding_buffer[32];
    if (locale == NULL) {
        return NULL;
    }

    const char *dot = strrchr(locale, '.');
    if (dot == NULL || dot[1] == '\0') {
        return NULL;
    }

    const char *start = dot + 1;
    const char *end = strchr(start, '@');
    size_t len = end == NULL ? strlen(start) : (size_t) (end - start);
    if (len == 0 || len >= sizeof(encoding_buffer)) {
        return NULL;
    }

    memcpy(encoding_buffer, start, len);
    encoding_buffer[len] = '\0';
    if (strcasecmp(encoding_buffer, "UTF8") == 0
            || strcasecmp(encoding_buffer, "UTF-8") == 0
            || strcasecmp(encoding_buffer, "CP65001") == 0
            || strcmp(encoding_buffer, "65001") == 0)
    {
        return "UTF-8";
    }
    if ((encoding_buffer[0] == 'C' || encoding_buffer[0] == 'c')
            && (encoding_buffer[1] == 'P' || encoding_buffer[1] == 'p')
            && is_all_digits(encoding_buffer + 2))
    {
        encoding_buffer[0] = 'C';
        encoding_buffer[1] = 'P';
        return encoding_buffer;
    }
    if (is_all_digits(encoding_buffer)) {
        static char codepage_buffer[sizeof(encoding_buffer) + 2];
        memcpy(codepage_buffer, "CP", 2);
        strcpy(codepage_buffer + 2, encoding_buffer);
        return codepage_buffer;
    }
    return encoding_buffer;
}
#endif



int is_char_at(const uint32_t *text, const size_t idx, const ucs4_t expected_char)
{
    return text != NULL && u32_cmp(text + idx, &expected_char, 1) == 0;
}



void set_char_at(uint32_t *text, const size_t idx, const ucs4_t char_to_set)
{
    u32_set(text + idx, char_to_set, 1);
}



int is_empty(const uint32_t *text)
{
    return text == NULL || is_char_at(text, 0, char_nul);
}



int u32_is_blank(const uint32_t *text)
{
    if (is_empty(text)) {
        return 1;
    }

    for (const uint32_t *c = text; *c != char_nul; c++) {
        if (!is_blank(*c)) {
            return 0;
        }
    }
    return 1;
}



int is_ascii_printable(const ucs4_t c)
{
    return c >= 0x20 && c < 0x7f;
}



int is_allowed_anywhere(const ucs4_t c)
{
    /* ESC, CR, LF, and TAB are control characters */
    return !uc_is_cntrl(c) || c == char_tab || c == char_cr || c == char_newline || c == char_esc;
}



int is_allowed_in_shape(const ucs4_t c)
{
    return is_allowed_anywhere(c) && c != char_cr && c != char_newline;
}



int is_allowed_in_sample(const ucs4_t c)
{
    return is_allowed_anywhere(c);
}



int is_allowed_in_filename(const ucs4_t c)
{
    return is_allowed_anywhere(c) && c != char_cr && c != char_newline && c != char_esc;
}



int is_allowed_in_kv_string(const ucs4_t c)
{
    return is_allowed_anywhere(c) && c != char_cr && c != char_newline && c != char_esc;
}



int is_blank(const ucs4_t c)
{
    return c == char_tab || uc_is_blank(c);
}



uint32_t *new_empty_string32()
{
    return (uint32_t *) calloc(1, sizeof(uint32_t));
}



ucs4_t to_utf32(char ascii)
{
    ucs4_t c = char_nul;
    if (ascii >= 0x20 && ascii < 0x7f) {
        c = (ucs4_t)ascii;  // Store the ASCII value directly in c
    }
    return c;
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
        } else if (ansipos == 1 && c >= 0x40 && c <= 0x5f) { /* between '@' and '_' (mostly uppercase letters) */
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
    if (rest == NULL) {
        rest = s + u32_strlen(s);
    }
    return (uint32_t *) rest;
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



const char *get_default_encoding()
{
    const char *system_encoding = locale_charset();

    #ifdef _WIN32
        const char *env_encoding = extract_locale_encoding(get_locale_from_environment());
        if (env_encoding != NULL) {
            return check_encoding(env_encoding, system_encoding);
        }
    #endif

    return system_encoding;
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



char *to_utf8(uint32_t *src)
{
    if (src == NULL) {
        return NULL;
    }
    if (is_empty(src)) {
        return (char *) strdup("");
    }
    char *result = u32_strconv_to_encoding(src, "UTF-8", iconveh_error);
    if (result == NULL) {
        bx_fprintf(stderr, "%s: failed to convert a string to UTF-8: %s\n", PROJECT, strerror(errno));
        return NULL;
    }
    return result;
}



uint32_t *u32_nspaces(const size_t n)
{
    uint32_t *result = (uint32_t *) malloc((n + 1) * sizeof(uint32_t));
    if (result == NULL) {
        perror(PROJECT);
        return NULL;
    }
    if (n > 0) {
        u32_set(result, char_space, n);
    }
    set_char_at(result, n, char_nul);
    return result;
}



uint32_t *u32_strnrstr(const uint32_t *haystack, const uint32_t *needle, const size_t needle_len)
{
    if (is_empty(needle)) {
        return (uint32_t *) haystack;
    }
    if (is_empty(haystack)) {
        return NULL;
    }

    const uint32_t *p = u32_strrchr(haystack, needle[0]);
    if (!p) {
        return NULL;
    }

    while (p >= haystack) {
        if (u32_strncmp(p, needle, needle_len) == 0) {
            return (uint32_t *) p;
        }
        --p;
    }

    return NULL;
}



void u32_insert_space_at(uint32_t **s, const size_t idx, const size_t n)
{
    if (s == NULL || *s == NULL || n == 0) {
        return;
    }

    size_t len = u32_strlen(*s);
    size_t x = idx;
    if (idx > len) {
        x = len;
    }

    uint32_t *tmp = (uint32_t *) realloc(*s, (len + 1 + n) * sizeof(uint32_t));
    if (tmp == NULL) {
        perror(PROJECT);
        return;
    }

    *s = tmp;
    u32_move(*s + x + n, *s + x, len - x + 1);
    u32_set(*s + x, char_space, n);
}


size_t count_vs16_promotions(const uint32_t *s)
{
    size_t count = 0;
    for (size_t i = 0; s[i] != char_nul; i++) {
        if (s[i] == VARIATION_SELECTOR_16) {
            count++;
        }
    }
    return count;
}


/* vim: set cindent sw=4: */
