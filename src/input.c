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
 * Read and analyze input text.
 */

#include "config.h"
#include <errno.h>
#include <string.h>
#include <unistr.h>
#include <unitypes.h>

#include "boxes.h"
#include "regulex.h"
#include "tools.h"
#include "unicode.h"
#include "input.h"



/**
 * Determine if the given line of raw text is ended by a line break.
 * @param s the string to check
 * @param len length of s in characters
 * @returns != 0 if line break found;
 *          == 0 if line break not found
 */
static int has_linebreak(const uint32_t *s, const int len)
{
    int result = 0;
    if (s != NULL && len > 0) {
        ucs4_t the_last = s[len - 1];
        result = u32_cmp(&char_cr, &the_last, 1) == 0 || u32_cmp(&char_newline, &the_last, 1) == 0;
        #if defined(DEBUG)
            fprintf(stderr, "has_linebreak: (%#010x) %d\n", (int) the_last, result);
        #endif
    }
    return result;
}



/**
 * Determine indentation of given lines in spaces. Lines are assumed to be free of trailing whitespace.
 * @param lines the lines to examine
 * @param lines_size number of lines to examine
 * @returns >= 0: indentation in spaces; < 0: error
 */
static int get_indent(const line_t *lines, const size_t lines_size)
{
    int res = LINE_MAX_BYTES;  /* result */
    int nonblank = 0;          /* true if one non-blank line found */

    if (lines == NULL) {
        fprintf(stderr, "%s: internal error\n", PROJECT);
        return -1;
    }
    if (lines_size == 0) {
        return 0;
    }

    for (size_t j = 0; j < lines_size; ++j) {
        if (lines[j].len > 0) {
            nonblank = 1;
            size_t ispc = strspn(lines[j].text, " ");
            if ((int) ispc < res) {
                res = ispc;
            }
        }
    }

    if (nonblank) {
        return res;            /* success */
    } else {
        return 0;              /* success, but only blank lines */
    }
}



int apply_substitutions(input_t *result, const int mode)
{
    size_t anz_rules;
    reprule_t *rules;
    size_t j, k;

    if (opt.design == NULL) {
        return 1;
    }

    if (mode == 0) {
        anz_rules = opt.design->anz_reprules;
        rules = opt.design->reprules;
    }
    else if (mode == 1) {
        anz_rules = opt.design->anz_revrules;
        rules = opt.design->revrules;
    }
    else {
        fprintf(stderr, "%s: internal error\n", PROJECT);
        return 2;
    }

    /*
     *  Compile regular expressions
     */
    #ifdef REGEXP_DEBUG
        fprintf(stderr, "Compiling %d %s rule patterns\n", (int) anz_rules, mode ? "reversion" : "replacement");
    #endif
    errno = 0;
    opt.design->current_rule = rules;
    for (j = 0; j < anz_rules; ++j, ++(opt.design->current_rule)) {
        rules[j].prog = compile_pattern(rules[j].search);
        if (rules[j].prog == NULL) {
            return 5;
        }
    }
    opt.design->current_rule = NULL;
    if (errno) {
        return 3;
    }

    /*
     *  Apply regular expression substitutions to input lines
     */
    for (k = 0; k < result->num_lines; ++k) {
        opt.design->current_rule = rules;
        for (j = 0; j < anz_rules; ++j, ++(opt.design->current_rule)) {
            #ifdef REGEXP_DEBUG
            fprintf (stderr, "regex_replace(0x%p, \"%s\", \"%s\", %d, \'%c\') == ",
                    rules[j].prog, rules[j].repstr, u32_strconv_to_output(result->lines[k].mbtext),
                    (int) result->lines[k].num_chars, rules[j].mode);
            #endif
            uint32_t *newtext = regex_replace(rules[j].prog, rules[j].repstr,
                                              result->lines[k].mbtext, result->lines[k].num_chars, rules[j].mode == 'g');
            #ifdef REGEXP_DEBUG
                fprintf (stderr, "\"%s\"\n", newtext ? u32_strconv_to_output(newtext) : "NULL");
            #endif
            if (newtext == NULL) {
                return 1;
            }

            BFREE(result->lines[k].mbtext_org);  /* original address allocated for mbtext */
            result->lines[k].mbtext = newtext;
            result->lines[k].mbtext_org = newtext;

            analyze_line_ascii(result, result->lines + k);

            #ifdef REGEXP_DEBUG
                fprintf (stderr, "result->lines[%d] == {%d, \"%s\"}\n", (int) k,
                    (int) result->lines[k].num_chars, u32_strconv_to_output(result->lines[k].mbtext));
            #endif
        }
        opt.design->current_rule = NULL;
    }

    /*
     *  If text indentation was part of the lines processed, indentation
     *  may now be different -> recalculate result->indent.
     */
    if (opt.design->indentmode == 't') {
        int rc;
        rc = get_indent(result->lines, result->num_lines);
        if (rc >= 0) {
            result->indent = (size_t) rc;
        } else {
            return 4;
        }
    }

    return 0;
}



static void trim_trailing_ws_carefully(uint32_t *mbtemp, size_t *len_chars)
{
    if (opt.r) {
        /* remove only trailing line breaks, but keep the space */
        if (is_char_at(mbtemp, *len_chars - 1, char_newline)) {
            set_char_at(mbtemp, *len_chars - 1, char_nul);
            --(*len_chars);
        }
        if (is_char_at(mbtemp, *len_chars - 1, char_cr)) {
            set_char_at(mbtemp, *len_chars - 1, char_nul);
            --(*len_chars);
        }
    }
    else {
        /* remove all trailing whitespace, including unicode whitespace */
        btrim32(mbtemp, len_chars);
    }
}



input_t *read_all_input()
{
    char buf[LINE_MAX_BYTES + 3];    /* static input buffer incl. newline + zero terminator */
    size_t input_size = 0;           /* number of elements allocated */

    input_t *result = (input_t *) calloc(1, sizeof(input_t));
    result->indent = LINE_MAX_BYTES;

    while (fgets(buf, LINE_MAX_BYTES + 2, opt.infile))
    {
        if (result->num_lines % 100 == 0) {
            input_size += 100;
            line_t *tmp = (line_t *) realloc(result->lines, input_size * sizeof(line_t));
            if (tmp == NULL) {
                perror(PROJECT);
                BFREE (result->lines);
                return NULL;
            }
            result->lines = tmp;
        }

        memset(result->lines + result->num_lines, 0, sizeof(line_t));

        uint32_t *mbtemp = u32_strconv_from_input(buf);
        size_t len_chars = u32_strlen(mbtemp);
        result->final_newline = has_linebreak(mbtemp, len_chars);
        trim_trailing_ws_carefully(mbtemp, &len_chars);

        /*
         * Expand tabs
         */
        if (len_chars > 0) {
            uint32_t *temp = NULL;
            len_chars = expand_tabs_into(mbtemp, opt.tabstop, &temp,
                                         &(result->lines[result->num_lines].tabpos),
                                         &(result->lines[result->num_lines].tabpos_len));
            if (len_chars == 0) {
                perror(PROJECT);
                BFREE (result->lines);
                return NULL;
            }
            result->lines[result->num_lines].mbtext = temp;
            BFREE(mbtemp);
            temp = NULL;
        }
        else {
            result->lines[result->num_lines].mbtext = mbtemp;
        }
        result->lines[result->num_lines].mbtext_org = result->lines[result->num_lines].mbtext;
        result->lines[result->num_lines].num_chars = len_chars;

        ++result->num_lines;
    }

    if (ferror(stdin)) {
        perror(PROJECT);
        BFREE (result->lines);
        return NULL;
    }
    return result;
}



int analyze_input(input_t *result)
{
    result->indent = LINE_MAX_BYTES;
    result->maxline = 0;

    /*
     * Build ASCII equivalent of the multi-byte string, update line stats
     */
    for (size_t i = 0; i < result->num_lines; ++i) {
        analyze_line_ascii(result, result->lines + i);
    }

    /*
     *  Exit if there was no input at all
     */
    if (result->lines == NULL || result->lines[0].text == NULL) {
        return 0;
    }

    /*
     *  Compute indentation
     */
    int rc = get_indent(result->lines, result->num_lines);
    if (rc >= 0) {
        result->indent = (size_t) rc;
    } else {
        return 1;
    }

    /*
     *  Remove indentation, unless we want to preserve it (when removing
     *  a box or if the user wants to retain it inside the box)
     */
    if (opt.design->indentmode != 't' && opt.r == 0) {
        for (size_t i = 0; i < result->num_lines; ++i) {
            #ifdef DEBUG
                fprintf(stderr, "%2d: mbtext = \"%s\" (%d chars)\n", (int) i,
                    u32_strconv_to_output(result->lines[i].mbtext), (int) result->lines[i].num_chars);
            #endif
            if (result->lines[i].num_chars >= result->indent) {
                memmove(result->lines[i].text, result->lines[i].text + result->indent,
                        result->lines[i].len - result->indent + 1);
                result->lines[i].len -= result->indent;

                result->lines[i].mbtext = advance32(result->lines[i].mbtext, result->indent);
                result->lines[i].num_chars -= result->indent;
            }
            #ifdef DEBUG
                fprintf(stderr, "%2d: mbtext = \"%s\" (%d chars)\n", (int) i,
                    u32_strconv_to_output(result->lines[i].mbtext), (int) result->lines[i].num_chars);
            #endif
        }
        result->maxline -= result->indent;
    }

    /*
     *  Apply regular expression substitutions
     */
    if (opt.r == 0) {
        if (apply_substitutions(result, 0) != 0) {
            return 1;
        }
    }

    #ifdef DEBUG
        fprintf (stderr, "Effective encoding: %s\n", encoding);
        print_input_lines(NULL);
    #endif
    return 0;
}

/*EOF*/                                                  /* vim: set sw=4: */
