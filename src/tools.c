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
 * Provide tool functions for error reporting and some string handling
 */

#include "config.h"

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <uniconv.h>
#include <unictype.h>
#include <unistr.h>
#include <unitypes.h>
#include <uniwidth.h>

#include "boxes.h"
#include "logging.h"
#include "regulex.h"
#include "shape.h"
#include "tools.h"
#include "unicode.h"



static pcre2_code *pattern_ascii_id = NULL;
static pcre2_code *pattern_ascii_id_strict = NULL;

/**
 * Initialize the `bx_fprintf` function pointer to point to the original
 * `bx_fprintf` function, now renamed `bx_fprintf_original`. During unit
 * tests, this will be replaced with `__wrap_bx_fprintf`, which stores
 * the result that would have been printed so the output can be validated.
 * This is necessary for unit testing and CI to work with MacOS.
 */
bx_fprintf_t bx_fprintf = bx_fprintf_original;


static pcre2_code *get_pattern_ascii_id(int strict)
{
    pcre2_code *result = NULL;
    if (strict) {
        if (pattern_ascii_id_strict == NULL) {
            pattern_ascii_id_strict = compile_pattern("^(?!.*?--|none)[a-z][a-z0-9-]*(?<!-)$");
        }
        result = pattern_ascii_id_strict;
    }
    else {
        if (pattern_ascii_id == NULL) {
            pattern_ascii_id = compile_pattern("^(?!.*?[-_]{2,}|none)[a-zA-Z][a-zA-Z0-9_-]*(?<![-_])$");
        }
        result = pattern_ascii_id;
    }
    return result;
}



int strisyes(const char *s)
/*
 *  Determine if the string s has a contents indicating "yes".
 *
 *     s   string to examine
 *
 *  RETURNS:  == 0  string does NOT indicate yes (including errors)
 *            != 0  string indicates yes
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    if (s == NULL) {
        return 0;
    }

    if (!strncasecmp("on", s, 3)) {
        return 1;
    }
    else if (!strncasecmp("yes", s, 4)) {
        return 1;
    }
    else if (!strncasecmp("true", s, 5)) {
        return 1;
    }
    else if (!strncmp("1", s, 2)) {
        return 1;
    }
    else if (!strncasecmp("t", s, 2)) {
        return 1;
    }
    else {
        return 0;
    }
}



int strisno(const char *s)
/*
 *  Determine if the string s has a contents indicating "no".
 *
 *     s   string to examine
 *
 *  RETURNS:  == 0  string does NOT indicate no (including errors)
 *            != 0  string indicates no
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    if (s == NULL) {
        return 0;
    }

    if (!strncasecmp("off", s, 4)) {
        return 1;
    }
    else if (!strncasecmp("no", s, 3)) {
        return 1;
    }
    else if (!strncasecmp("false", s, 6)) {
        return 1;
    }
    else if (!strncmp("0", s, 2)) {
        return 1;
    }
    else if (!strncasecmp("f", s, 2)) {
        return 1;
    }
    else {
        return 0;
    }
}



char *concat_strings_alloc(size_t count, ...)
{
    if (count < 1) {
        return strdup("");
    }

    size_t total_len = 0;
    const char *src;
    va_list va;

    va_start(va, count);
    for (size_t i = 0; i < count; i++) {
        src = va_arg(va, const char *);
        if (src != NULL) {
            total_len += strlen(src);
        }
    }
    va_end(va);

    char *result = malloc(total_len + 1);
    char *p = result;

    va_start(va, count);
    for (size_t i = 0; i < count; i++) {
        src = va_arg(va, const char *);
        if (src != NULL && src[0] != '\0') {
            strcpy(p, src);
            p += strlen(src);
        }
    }
    va_end(va);

    *p = '\0';
    return result;
}



char *repeat(char *s, size_t count)
{
    if (s == NULL) {
        return NULL;
    }

    size_t len = strlen(s);
    char *result = (char *) malloc(count * len + 1);
    if (result != NULL) {
        char *dest = result;
        for (size_t i = 0; i < count; i++) {
            strcpy(dest, s);
            dest += len;
        }
        *dest = '\0';
    }
    return result;
}



int empty_line(const line_t *line)
{
    if (!line) {
        return 1;
    }
    return bxs_is_blank(line->text);
}



size_t expand_tabs_into(const uint32_t *input_buffer, const int tabstop, uint32_t **text, size_t **tabpos,
        size_t *tabpos_len)
{
    static uint32_t temp[LINE_MAX_BYTES + 100]; /* work string */
    size_t io;                                  /* character position in work string */
    size_t tabnum = 0;                          /* index of the current tab */

    *text = NULL;
    *tabpos = NULL;
    *tabpos_len = 0;

    if (opt.tabexp == 'k') {
        /* We need to know the exact tab positions only if expansion type 'k' is requested (keep tabs as much as they
         * were as possible). Else we'll just convert spaces and tabs without having to know where exactly the tabs
         * were in the first place. */
        ucs4_t puc;
        const uint32_t *rest = input_buffer;
        while ((rest = u32_next(&puc, rest))) {
            if (puc == char_tab) {
                (*tabpos_len)++;
            }
        }
    }

    if (*tabpos_len > 0) {
        *tabpos = (size_t *) calloc((*tabpos_len) + 1, sizeof(size_t));
        if (*tabpos == NULL) {
            return 0; /* out of memory */
        }
    }

    ucs4_t puc;
    const uint32_t *rest = input_buffer;
    io = 0;
    while ((rest = u32_next(&puc, rest)) && io < (LINE_MAX_BYTES - 12)) {
        if (puc == char_tab) {
            if (*tabpos_len > 0) {
                (*tabpos)[tabnum++] = io;
            }
            size_t num_spc = tabstop - (io % tabstop);
            u32_set(temp + io, char_space, num_spc);
            io += num_spc;
        }
        else {
            set_char_at(temp, io, puc);
            ++io;
        }
    }
    temp[io] = 0;

    *text = u32_strdup(temp);
    if (*text == NULL) {
        return 0;
    }
    return io;
}



void btrim(char *text, size_t *len)
/*
 *  Remove trailing whitespace from line.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    long idx = (long) (*len) - 1;

    while (idx >= 0 && (text[idx] == '\n' || text[idx] == '\r' || text[idx] == '\t' || text[idx] == ' ')) {
        text[idx--] = '\0';
    }

    *len = idx + 1;
}



void btrim32(uint32_t *text, size_t *len)
/*
 *  Remove trailing whitespace from line (unicode and escape sequence enabled version).
 *  CHECK replace by bxs_rtrim() from bxstring module
 *
 *      text     string to trim
 *      len      pointer to the length of the string in characters
 *
 *  Both the string and the length will be modified as trailing whitespace is removed.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    if (text == NULL || len == 0) {
        return;
    }

    const uint32_t *rest = text;
    int last_char_pos = -1;
    size_t step_invis;

    for (ucs4_t c = text[0]; c != char_nul; c = rest[0]) {
        if (c != char_esc) {
            if (!uc_is_c_whitespace(c) && !uc_is_property_white_space(c) && !uc_is_property_bidi_whitespace(c)) {
                last_char_pos = (int) (rest - text);
            }
        }
        rest = advance_next32(rest, &step_invis);
    }

    /* If the last character is followed by an escape sequence, keep it (but only one). */
    if (last_char_pos >= 0) {
        rest = text + last_char_pos + 1;
        if (rest[0] == char_esc) {
            advance_next32(rest, &step_invis);
            last_char_pos += step_invis;
        }
    }

    set_char_at(text, (size_t) (last_char_pos + 1), char_nul);
    *len = (size_t) (last_char_pos + 1);
}



size_t my_strrspn(const char *s, const char *accept)
{
    if (!s || *s == '\0') {
        return 0;
    }
    if (!accept || *accept == '\0') {
        return 0;
    }

    for (int i = strlen(s) - 1; i >= 0; i--) {
        size_t idx = (size_t) i;
        if (strchr(accept, s[idx]) == NULL) {
            return strlen(s) - (idx + 1);
        }
    }
    return strlen(s);
}



uint32_t *tabbify_indent(const size_t lineno, uint32_t *indentspc, const size_t indentspc_len)
{
    size_t i;
    uint32_t *result;
    size_t result_len;

    if (opt.tabexp != 'k') {
        return indentspc;
    }
    if (lineno >= input.num_lines) {
        return NULL;
    }
    if (indentspc_len == 0) {
        return new_empty_string32();
    }

    result = (uint32_t *) malloc((indentspc_len + 1) * sizeof(uint32_t));
    if (result == NULL) {
        perror(PROJECT);
        return NULL;
    }
    u32_set(result, char_space, indentspc_len);
    set_char_at(result, indentspc_len, char_nul);
    result_len = indentspc_len;

    for (i = 0; i < input.lines[lineno].tabpos_len && input.lines[lineno].tabpos[i] < indentspc_len; ++i) {
        size_t tpos = input.lines[lineno].tabpos[i];
        size_t nspc = opt.tabstop - (tpos % opt.tabstop); /* no of spcs covered by tab */
        if (tpos + nspc > input.indent) {
            break;
        }
        set_char_at(result, tpos, char_tab);
        result_len -= nspc - 1;
        set_char_at(result, result_len, char_nul);
    }

    return result;
}



char *nspaces(const size_t n)
{
    char *spaces = (char *) memset(malloc(n + 1), (int) ' ', n);
    spaces[n] = '\0';
    return spaces;
}



/**
 * Debugging Code: Display contents of input structure
 * @param heading a heading to show for identification of the printed lines
 */
void print_input_lines(const char *heading)
{
    if (is_debug_logging(MAIN)) {
        log_debug(__FILE__, MAIN, "%d Input Lines%s:\n", (int) input.num_lines, heading != NULL ? heading : "");
        log_debug(__FILE__, MAIN, "     [num_chars] \"real text\" [num_cols] \"ascii_text\"\n");
        for (size_t i = 0; i < input.num_lines; ++i) {
            char *outtext = bxs_to_output(input.lines[i].text);
            log_debug(__FILE__, MAIN, "%4d [%02d] \"%s\"  [%02d] \"%s\"", (int) i,
                    (int) input.lines[i].text->num_chars, outtext,
                    (int) input.lines[i].text->num_columns, input.lines[i].text->ascii);
            BFREE(outtext);
            log_debug_cont(MAIN, "\tTabs: [");
            if (input.lines[i].tabpos != NULL) {
                for (size_t j = 0; j < input.lines[i].tabpos_len; ++j) {
                    log_debug_cont(MAIN, "%d", (int) input.lines[i].tabpos[j]);
                    if (j < input.lines[i].tabpos_len - 1) {
                        log_debug_cont(MAIN, ", ");
                    }
                }
            }
            log_debug_cont(MAIN, "] (%d)", (int) input.lines[i].tabpos_len);
            log_debug_cont(MAIN, "\tinvisible=%d\n", (int) input.lines[i].text->num_chars_invisible);

            log_debug(__FILE__, MAIN, "    visible_char=");
            if (input.lines[i].text->visible_char != NULL) {
                log_debug_cont(MAIN, "[");
                for (size_t j = 0; j < input.lines[i].text->num_chars_visible; j++) {
                    log_debug_cont(MAIN, "%d%s", (int) input.lines[i].text->visible_char[j],
                        j == (input.lines[i].text->num_chars_visible - 1) ? "" : ", ");
                }
                log_debug_cont(MAIN, "]\n");
            }
            else {
                log_debug_cont(MAIN, "null\n");
            }

            log_debug(__FILE__, MAIN, "      first_char=");
            if (input.lines[i].text->first_char != NULL) {
                log_debug_cont(MAIN, "[");
                for (size_t j = 0; j < input.lines[i].text->num_chars_visible; j++) {
                    log_debug_cont(MAIN, "%d%s", (int) input.lines[i].text->first_char[j],
                        j == (input.lines[i].text->num_chars_visible - 1) ? "" : ", ");
                }
                log_debug_cont(MAIN, "]\n");
            }
            else {
                log_debug_cont(MAIN, "null\n");
            }
        }
        log_debug(__FILE__, MAIN, " Longest line: %d columns\n", (int) input.maxline);
        log_debug(__FILE__, MAIN, "  Indentation: %2d spaces\n", (int) input.indent);
        log_debug(__FILE__, MAIN, "Final newline: %s\n", input.final_newline ? "yes" : "no");
    }
}



/**
 * Analyze the multi-byte string in order to determine its metrics:
 * - number of visible columns it occupies
 * - number of escape characters (== number of escape sequences)
 * - the ASCII equivalent of the string
 * - the number of invisible characters in the string
 *
 * @param <s> the multi-byte string to analyze
 * @param <num_esc> pointer to where the number of escape sequences should be stored
 * @param <ascii> pointer to where the ASCII equivalent of the string should be stored
 * @param <posmap> pointer to the position map, which maps each position in <ascii> to a position in <s>
 * @returns the number of invisible characters in <s>
 */
size_t count_invisible_chars(const uint32_t *s, size_t *num_esc, char **ascii, size_t **posmap)
{
    size_t invis = 0; /* counts invisible column positions */
    *num_esc = 0;     /* counts the number of escape sequences found */

    if (is_empty(s)) {
        (*ascii) = (char *) strdup("");
        (*posmap) = NULL;
        return 0;
    }

    size_t buflen = (size_t) u32_strwidth(s, encoding) + 1;
    size_t map_size = BMAX((size_t) 5, buflen);
    size_t map_idx = 0;
    size_t *map = (size_t *) calloc(map_size, sizeof(size_t)); /* might not be enough if many double-wide chars */
    (*ascii) = (char *) calloc(buflen, sizeof(char));          /* maybe a little too much, but certainly enough */
    char *p = *ascii;

    size_t mb_idx = 0;
    size_t step_invis;
    const uint32_t *rest = s;

    for (ucs4_t c = s[0]; c != char_nul; c = rest[0]) {
        if (map_idx >= map_size - 4) {
            map_size = map_size * 2 + 1;
            map = (size_t *) realloc(map, map_size * sizeof(size_t));
        }

        if (c == char_esc) {
            (*num_esc)++;
        }
        else if (is_ascii_printable(c)) {
            *p = c & 0xff;
            map[map_idx++] = mb_idx;
            ++p;
        }
        else {
            int cols = uc_width(c, encoding);
            if (cols > 0) {
                memset(p, (int) 'x', cols);
                for (int i = 0; i < cols; i++) {
                    map[map_idx++] = mb_idx;
                }
                p += cols;
            }
        }

        rest = advance_next32(rest, &step_invis);

        mb_idx += BMAX((size_t) 1, step_invis);
        invis += step_invis;
    }

    *p = '\0';
    (*posmap) = map;
    return invis;
}



int is_csi_reset(const uint32_t *csi)
{
    ucs4_t puc = '\0';
    const uint32_t *rest = csi;
    size_t csi_pos = 0;
    while ((rest = u32_next(&puc, rest))) {
        switch(csi_pos) {
            case 0:
                if (puc != char_esc) {
                    return 0;
                }
                break;
            case 1:
                if (puc != '[' && puc != '(') {
                    return 0;
                }
                break;
            case 2:
                if (puc != '0') {
                    if (puc >= 0x40 && puc <= 0x7e) {
                        return 1;
                    }
                    else if ((puc == '1' && *rest == '0')
                          || (puc == '3' && *rest == '9')
                          || (puc == '4' && *rest == '9')
                          || (puc == '5' && *rest == '9')
                          || (puc == '7' && *rest == '5')) {
                        rest = u32_next(&puc, rest);
                        break;
                    }
                    return 0;
                }
                break;
            default:
                return (puc >= 0x40 && puc <= 0x7e) ? 1 : 0;
        }
        csi_pos++;
    }
    return 0;
}



void analyze_line_ascii(input_t *input_ptr, line_t *line)
{
    if (line->text->num_columns > input_ptr->maxline) {
        input_ptr->maxline = line->text->num_columns;
    }
}



int array_contains0(char **array, const char *s)
{
    int result = 0;
    if (array != NULL) {
        for (size_t i = 0; array[i] != NULL; ++i) {
            if (strcasecmp(array[i], s) == 0) {
                result = 1;
                break;
            }
        }
    }
    return result;
}



int array_contains_bxs(bxstr_t **array, const size_t array_len, bxstr_t *s)
{
    int result = 0;
    if (array != NULL && array_len > 0) {
        for (size_t i = 0; i < array_len; ++i) {
            if (bxs_strcmp(array[i], s) == 0) {
                result = 1;
                break;
            }
        }
    }
    return result;
}



size_t array_count0(char **array)
{
    size_t num_elems = 0;
    if (array != NULL) {
        while (array[num_elems] != NULL) {
            ++num_elems;
        }
    }
    return num_elems;
}



char *trimdup(char *s, char *e)
{
    if (s > e || (s == e && *s == '\0')) {
        return strdup("");
    }
    while (s <= e && (*s == ' ' || *s == '\t')) {
        ++s;
    }
    while (e > s && (*e == ' ' || *e == '\t')) {
        --e;
    }
    return bx_strndup(s, e - s + 1);
}



int tag_is_valid(char *tag)
{
    pcre2_code *pattern = get_pattern_ascii_id(1);
    return regex_match(pattern, tag);
}



int is_ascii_id(bxstr_t *s, int strict)
{
    if (s == NULL || s->num_chars == 0) {
        return 0;
    }
    pcre2_code *pattern = get_pattern_ascii_id(strict);
    return u32_regex_match(pattern, s->memory);
}



char *bx_strndup(const char *s, size_t n)
{
    if (s == NULL) {
        return NULL;
    }

    size_t len = strlen(s);
    if (n < len) {
        len = n;
    }

    char *result = (char *) malloc(len + 1);
    if (result == NULL) {
        return NULL;
    }

    result[len] = '\0';
    return (char *) memcpy(result, s, len);
}



void bx_fprintf_original(FILE *stream, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    vfprintf(stream, format, va);
    va_end(va);
}


void set_bx_fprintf(bx_fprintf_t bx_fprintf_function) {
    bx_fprintf = bx_fprintf_function;
}


FILE *bx_fopens(bxstr_t *pathname, char *mode)
{
    return bx_fopen(to_utf8(pathname->memory), mode);
}



FILE *bx_fopen(char *pathname, char *mode)
{
    /*
     * On Linux/UNIX and OS X (Mac), one can access files with non-ASCII file names by passing them to fopen() as UTF-8.
     * On Windows, a different function must be called. (Info: https://stackoverflow.com/a/35065142/1005481)
     * On newer Windows, we're good:
     * https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/setlocale-wsetlocale#utf-8-support
     */
    FILE *f = fopen(pathname, mode);
    return f;
}


/* vim: set sw=4: */
