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
 * Read and analyze input text.
 */

#include "config.h"

#include <errno.h>
#include <string.h>
#include <unistr.h>
#include <unitypes.h>

#include "boxes.h"
#include "input.h"
#include "logging.h"
#include "regulex.h"
#include "tools.h"
#include "unicode.h"



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
        log_debug(__FILE__, MAIN, "has_linebreak: (%#010x) %d\n", (int) the_last, result);
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
    int res = LINE_MAX_BYTES; /* result */
    int nonblank = 0;         /* true if one non-blank line found */

    if (lines == NULL) {
        bx_fprintf(stderr, "%s: internal error\n", PROJECT);
        return -1;
    }
    if (lines_size == 0) {
        return 0;
    }

    for (size_t j = 0; j < lines_size; ++j) {
        if (lines[j].text->num_columns > 0) {
            nonblank = 1;
            size_t ispc = lines[j].text->indent;
            if ((int) ispc < res) {
                res = ispc;
            }
        }
    }

    if (nonblank) {
        return res; /* success */
    }
    else {
        return 0; /* success, but only blank lines */
    }
}



int apply_substitutions(input_t *result, const int mode)
{
    size_t num_rules;
    reprule_t *rules;
    size_t j, k;

    if (opt.design == NULL) {
        return 1;
    }

    if (mode == 0) {
        num_rules = opt.design->num_reprules;
        rules = opt.design->reprules;
    }
    else if (mode == 1) {
        num_rules = opt.design->num_revrules;
        rules = opt.design->revrules;
    }
    else {
        bx_fprintf(stderr, "%s: internal error\n", PROJECT);
        return 2;
    }

    /*
     *  Compile regular expressions
     */
    log_debug(__FILE__, REGEXP, "Compiling %d %s rule patterns\n", (int) num_rules, mode ? "reversion" : "replacement");
    errno = 0;
    opt.design->current_rule = rules;
    for (j = 0; j < num_rules; ++j, ++(opt.design->current_rule)) {
        rules[j].prog = u32_compile_pattern(rules[j].search->memory);
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
        for (j = 0; j < num_rules; ++j, ++(opt.design->current_rule)) {
            if (is_debug_logging(REGEXP)) {
                char *outtext = bxs_to_output(result->lines[k].text);
                char *outrepstr = bxs_to_output(rules[j].repstr);
                log_debug(__FILE__, REGEXP, "regex_replace(0x%p, \"%s\", \"%s\", %d, \'%c\') == ", rules[j].prog,
                    outrepstr, outtext, (int) result->lines[k].text->num_chars, rules[j].mode);
                BFREE(outtext);
                BFREE(outrepstr);
            }
            uint32_t *newtext = u32_regex_replace(rules[j].prog, rules[j].repstr->memory, result->lines[k].text->memory,
                    result->lines[k].text->num_chars, rules[j].mode == 'g');
            if (is_debug_logging(REGEXP)) {
                char *outnewtext = newtext ? u32_strconv_to_output(newtext) : strdup("NULL");
                log_debug_cont(REGEXP, "\"%s\"\n", outnewtext);
                BFREE(outnewtext);
            }
            if (newtext == NULL) {
                return 1;
            }

            bxs_free(result->lines[k].text);
            result->lines[k].text = bxs_from_unicode(newtext);

            analyze_line_ascii(result, result->lines + k);   /* update maxline value */

            if (is_debug_logging(REGEXP)) {
                char *outtext2 = bxs_to_output(result->lines[k].text);
                log_debug(__FILE__, REGEXP, "result->lines[%d] == {%d, \"%s\"}\n",
                    (int) k, (int) result->lines[k].text->num_chars, outtext2);
                BFREE(outtext2);
            }
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
        }
        else {
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
    char buf[LINE_MAX_BYTES + 3];      /* static input buffer incl. newline + zero terminator */
    size_t input_size = 0;             /* number of elements allocated */

    input_t *result = (input_t *) calloc(1, sizeof(input_t));
    result->indent = LINE_MAX_BYTES;

    while (fgets(buf, LINE_MAX_BYTES + 2, opt.infile)) {
        if (result->num_lines % 100 == 0) {
            input_size += 100;
            line_t *tmp = (line_t *) realloc(result->lines, input_size * sizeof(line_t));
            if (tmp == NULL) {
                perror(PROJECT);
                BFREE(result->lines);
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
            len_chars = expand_tabs_into(mbtemp, opt.tabstop, &temp, &(result->lines[result->num_lines].tabpos),
                    &(result->lines[result->num_lines].tabpos_len));
            if (len_chars == 0) {
                perror(PROJECT);
                BFREE(result->lines);
                return NULL;
            }
            result->lines[result->num_lines].text = bxs_from_unicode(temp);
            BFREE(temp);
        }
        else {
            result->lines[result->num_lines].text = bxs_new_empty_string();
        }

        BFREE(mbtemp);
        ++result->num_lines;
    }

    if (ferror(stdin)) {
        perror(PROJECT);
        BFREE(result->lines);
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
    }
    else {
        return 1;
    }

    /*
     *  Remove indentation, unless we want to preserve it (when removing
     *  a box or if the user wants to retain it inside the box)
     */
    if (opt.design->indentmode != 't' && opt.r == 0) {
        for (size_t i = 0; i < result->num_lines; ++i) {
            if (result->lines[i].text->num_columns >= result->indent) {
                /*
                 * We should really remove *columns* rather than *characters*, but since the removed characters are
                 * spaces (indentation), and there are no double-wide spaces in Unicode, both actions are equivalent.
                 */
                bxstr_t *unindented = bxs_cut_front(result->lines[i].text, result->indent);
                bxs_free(result->lines[i].text);
                result->lines[i].text = unindented;
            }
            if (is_debug_logging(MAIN)) {
                char *outtext = bxs_to_output(result->lines[i].text);
                log_debug(__FILE__, MAIN, "%2d: text = \"%s\" (%d chars, %d visible, %d invisible, %d columns)\n",
                    (int) i, outtext,
                    (int) result->lines[i].text->num_chars, (int) result->lines[i].text->num_chars_visible,
                    (int) result->lines[i].text->num_chars_invisible, (int) result->lines[i].text->num_columns);
                log_debug(__FILE__, MAIN, "    ascii = \"%s\"\n", result->lines[i].text->ascii);
                BFREE(outtext);
            }
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

    return 0;
}


/* vim: set sw=4: */
