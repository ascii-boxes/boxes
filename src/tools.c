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



void concat_strings(char *dst, int max_len, int count, ...)
/*
 *  Concatenate a variable number of strings into a fixed-length buffer.
 *
 *      dst     Destination array
 *      max_len Maximum resulting string length (including terminating NULL).
 *      count   Number of source strings.
 *
 *  The concatenation process terminates when either the destination
 *  buffer is full or all 'count' strings are processed.  Null string
 *  pointers are treated as empty strings.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    va_list va;
    const char *src;

    va_start(va, count);

    /*
     *  Sanity check.
     */
    if (max_len < 1) {
        return;
    }

    if (max_len == 1 || count < 1) {
        *dst = '\0';
        return;
    }

    /*
     *  Loop over all input strings.
     */
    while (count-- > 0 && max_len > 1) {
        /*
         * Grab an input string pointer.  If it's NULL, skip it (eg. treat
         * it as empty.
         */
        src = va_arg(va, const char *);

        if (src == NULL) {
            continue;
        }

        /*
         * Concatenate 'src' onto 'dst', as long as we have room.
         */
        while (*src && max_len > 1) {
            *dst++ = *src++;
            max_len--;
        }
    }

    va_end(va);

    /*
     * Terminate the string with an ASCII NUL.
     */
    *dst = '\0';
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
/*
 *  Return true if line is empty.
 *
 *  Empty lines either consist entirely of whitespace or don't exist.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char *p;
    size_t j;

    if (!line) {
        return 1;
    }
    if (line->text == NULL || line->len <= 0) {
        return 1;
    }

    for (p = line->text, j = 0; *p && j < line->len; ++j, ++p) {
        if (*p != ' ' && *p != '\t' && *p != '\r') {
            return 0;
        }
    }
    return 1;
}



size_t expand_tabs_into(const uint32_t *input_buffer, const int tabstop, uint32_t **text, size_t **tabpos,
        size_t *tabpos_len)
/*
 *  Expand tab chars in input_buffer and store result in text.
 *
 *  input_buffer   Line of text with tab chars
 *  tabstop        tab stop distance
 *  text           address of the pointer that will take the result
 *  tabpos         array of ints giving the positions of the first
 *                 space of an expanded tab in the text result buffer
 *  tabpos_len     number of tabs recorded in tabpos
 *
 *  Memory will be allocated for text and tabpos.
 *  Should only be called for lines of length > 0;
 *
 *  RETURNS:  Success: Length of the result line in characters (> 0)
 *            Error:   0       (e.g. out of memory)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
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



char *my_strnrstr(const char *s1, const char *s2, const size_t s2_len, int skip)
/*
 *  Return pointer to last occurrence of string s2 in string s1.
 *
 *      s1       string to search
 *      s2       string to search for in s1
 *      s2_len   length in characters of s2
 *      skip     number of finds to ignore before returning anything
 *
 *  RETURNS: pointer to last occurrence of string s2 in string s1
 *           NULL if not found or error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char *p;
    int comp;

    if (!s2 || *s2 == '\0') {
        return (char *) s1;
    }
    if (!s1 || *s1 == '\0') {
        return NULL;
    }
    if (skip < 0) {
        skip = 0;
    }

    p = strrchr(s1, s2[0]);
    if (!p) {
        return NULL;
    }

    while (p >= s1) {
        comp = strncmp(p, s2, s2_len);
        if (comp == 0) {
            if (skip--) {
                --p;
            }
            else {
                return p;
            }
        }
        else {
            --p;
        }
    }

    return NULL;
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



#if defined(DEBUG) || 0

/**
 *  Debugging Code: Display contents of input structure
 */
void print_input_lines(const char *heading)
{
    fprintf(stderr, "Input Lines%s:\n", heading != NULL ? heading : "");
    fprintf(stderr, "     [num_chars] \"real text\" [num_cols] \"ascii_text\"\n");
    for (size_t i = 0; i < input.num_lines; ++i) {
        fprintf(stderr, "%4d [%02d] \"%s\"  [%02d] \"%s\"", (int) i,
                (int) input.lines[i].num_chars, u32_strconv_to_output(input.lines[i].mbtext),
                (int) input.lines[i].len, input.lines[i].text);
        fprintf(stderr, "\tTabs: [");
        if (input.lines[i].tabpos != NULL) {
            for (size_t j = 0; j < input.lines[i].tabpos_len; ++j) {
                fprintf(stderr, "%d", (int) input.lines[i].tabpos[j]);
                if (j < input.lines[i].tabpos_len - 1) {
                    fprintf(stderr, ", ");
                }
            }
        }
        fprintf(stderr, "] (%d)", (int) input.lines[i].tabpos_len);
        fprintf(stderr, "\tinvisible=%d\n", (int) input.lines[i].invis);

        fprintf(stderr, "          posmap=");
        if (input.lines[i].posmap != NULL) {
            fprintf(stderr, "[");
            for (size_t j = 0; j < input.lines[i].len; j++) {
                fprintf(stderr, "%d%s", (int) input.lines[i].posmap[j], j == (input.lines[i].len - 1) ? "" : ", ");
            }
            fprintf(stderr, "]\n");
        }
        else {
            fprintf(stderr, "null\n");
        }
    }
    fprintf(stderr, " Longest line: %d columns\n", (int) input.maxline);
    fprintf(stderr, "  Indentation: %2d spaces\n", (int) input.indent);
    fprintf(stderr, "Final newline: %s\n", input.final_newline ? "yes" : "no");
}

#endif



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
    size_t num_esc = 0;
    char *ascii;
    size_t *map;
    size_t invis = count_invisible_chars(line->mbtext, &num_esc, &ascii, &(map));
    line->invis = invis;
    /* u32_strwidth() does not count control characters, i.e. ESC characters, for which we must correct */
    line->len = u32_strwidth(line->mbtext, encoding) - invis + num_esc;
    line->num_chars = u32_strlen(line->mbtext);
    BFREE(line->text);
    line->text = ascii;
    BFREE(line->posmap);
    line->posmap = map;

    if (line->len > input_ptr->maxline) {
        input_ptr->maxline = line->len;
    }
}



int array_contains(char **array, const size_t array_len, const char *s)
{
    int result = 0;
    if (array != NULL && array_len > 0) {
        for (size_t i = 0; i < array_len; ++i) {
            if (strcmp(array[i], s) == 0) {
                result = 1;
                break;
            }
        }
    }
    return result;
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
     */
    FILE *f = fopen(pathname, mode);
    // TODO Windows
    return f;
}


/* vim: set sw=4: */
