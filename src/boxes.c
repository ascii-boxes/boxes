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

#include "config.h"
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include <unictype.h>
#include <unistdio.h>
#include <unistr.h>
#include <unitypes.h>
#include <uniwidth.h>

#include "boxes.h"
#include "cmdline.h"
#include "list.h"
#include "shape.h"
#include "tools.h"
#include "discovery.h"
#include "generate.h"
#include "parsing.h"
#include "regulex.h"
#include "remove.h"
#include "unicode.h"



/*       _\|/_
         (o o)
 +----oOO-{_}-OOo------------------------------------------------------------+
 |                    G l o b a l   V a r i a b l e s                        |
 +--------------------------------------------------------------------------*/

design_t *designs = NULL;            /* available box designs */
int anz_designs = 0;                 /* no of designs after parsing */

opt_t opt;                           /* command line options */

input_t input = INPUT_INITIALIZER;   /* input lines */


/*       _\|/_
         (o o)
 +----oOO-{_}-OOo------------------------------------------------------------+
 |                           F u n c t i o n s                               |
 +--------------------------------------------------------------------------*/


static int build_design(design_t **adesigns, const char *cld)
/*
 *  Build a box design.
 *
 *      adesigns    Pointer to global designs list
 *      cld         the W shape as specified on the command line
 *
 *  Builds the global design list containing only one design which was
 *  built from the -c command line definition.
 *
 *  RETURNS:  != 0   on error (out of memory)
 *            == 0   on success
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    design_t *dp;                        /* pointer to design to be created */
    sentry_t *c;                         /* pointer to current shape */
    int i;
    int rc;

    *adesigns = (design_t *) calloc(1, sizeof(design_t));
    if (*adesigns == NULL) {
        perror(PROJECT);
        return 1;
    }
    dp = *adesigns;                      /* for readability */

    dp->name = "<Command Line Definition>";
    dp->aliases = (char **) calloc(1, sizeof(char *));
    dp->created = "now";
    dp->revision = "1.0";
    dp->sample = "n/a";
    dp->indentmode = DEF_INDENTMODE;
    dp->padding[BLEF] = 1;
    dp->defined_in = "(command line)";

    dp->tags = (char **) calloc(2, sizeof(char *));
    dp->tags[0] = "transient";

    dp->shape[W].height = 1;
    dp->shape[W].width = strlen(cld);
    dp->shape[W].elastic = 1;
    rc = genshape(dp->shape[W].width, dp->shape[W].height, &(dp->shape[W].chars));
    if (rc) {
        return rc;
    }
    strcpy(dp->shape[W].chars[0], cld);

    for (i = 0; i < NUM_SHAPES; ++i) {
        c = dp->shape + i;

        if (i == NNW || i == NNE || i == WNW || i == ENE || i == W
                || i == WSW || i == ESE || i == SSW || i == SSE) {
                    continue;
        }

        switch (i) {
            case NW:
            case SW:
                c->width = dp->shape[W].width;
                c->height = 1;
                c->elastic = 0;
                break;

            case NE:
            case SE:
                c->width = 1;
                c->height = 1;
                c->elastic = 0;
                break;

            case N:
            case S:
            case E:
                c->width = 1;
                c->height = 1;
                c->elastic = 1;
                break;

            default:
                fprintf(stderr, "%s: internal error\n", PROJECT);
                return 1;                /* never happens ;-) */
        }

        rc = genshape(c->width, c->height, &(c->chars));
        if (rc) {
            return rc;
        }
    }

    dp->maxshapeheight = 1;
    dp->minwidth = dp->shape[W].width + 2;
    dp->minheight = 3;

    return 0;
}



int query_is_undoc()
{
    return opt.query != NULL && strcmp(opt.query[0], QUERY_UNDOC) == 0 && opt.query[1] == NULL;
}



static int filter_by_tag(char **tags)
{
    #ifdef DEBUG
        fprintf(stderr, "filter_by_tag(");
        for (size_t tidx = 0; tags[tidx] != NULL; ++tidx) {
            fprintf(stderr, "%s%s", tidx > 0 ? ", " : "", tags[tidx]);
        }
    #endif

    int result = array_contains0(opt.query, QUERY_ALL);
    if (opt.query != NULL) {
        for (size_t qidx = 0; opt.query[qidx] != NULL; ++qidx) {
            if (opt.query[qidx][0] == '+') {
                result = array_contains0(tags, opt.query[qidx] + 1);
                if (!result) {
                    break;
                }
            }
            else if (opt.query[qidx][0] == '-') {
                if (array_contains0(tags, opt.query[qidx] + 1)) {
                    result = 0;
                    break;
                }
            }
            else if (array_contains0(tags, opt.query[qidx])) {
                result = 1;
            }
        }
    }

    #ifdef DEBUG
        fprintf(stderr, ") -> %d\n", result);
    #endif
    return result;
}



static int query_by_tag()
{
    design_t **list = sort_designs_by_name();                 /* temp list for sorting */
    if (list == NULL) {
        return 1;
    }
    for (int i = 0; i < anz_designs; ++i) {
        if (filter_by_tag(list[i]->tags)) {
            fprintf(opt.outfile, "%s%s", list[i]->name, opt.eol);
            for (size_t aidx = 0; list[i]->aliases[aidx] != NULL; ++aidx) {
                fprintf(opt.outfile, "%s (alias)%s", list[i]->aliases[aidx], opt.eol);
            }
        }
    }
    BFREE(list);
    return 0;
}



static int get_indent(const line_t *lines, const size_t lines_size)
/*
 *  Determine indentation of given lines in spaces.
 *
 *      lines      the lines to examine
 *      lines_size number of lines to examine
 *
 *  Lines are assumed to be free of trailing whitespace.
 *
 *  RETURNS:    >= 0   indentation in spaces
 *               < 0   error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
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



static int apply_substitutions(const int mode)
/*
 *  Apply regular expression substitutions to input text.
 *
 *    mode == 0   use replacement rules (box is being *drawn*)
 *         == 1   use reversion rules (box is being *removed*)
 *
 *  Attn: This modifies the actual input array!
 *
 *  RETURNS:  == 0   success
 *            != 0   error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
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
    for (k = 0; k < input.anz_lines; ++k) {
        opt.design->current_rule = rules;
        for (j = 0; j < anz_rules; ++j, ++(opt.design->current_rule)) {
            #ifdef REGEXP_DEBUG
            fprintf (stderr, "regex_replace(0x%p, \"%s\", \"%s\", %d, \'%c\') == ",
                    rules[j].prog, rules[j].repstr, u32_strconv_to_output(input.lines[k].mbtext),
                    (int) input.lines[k].num_chars, rules[j].mode);
            #endif
            uint32_t *newtext = regex_replace(rules[j].prog, rules[j].repstr,
                                              input.lines[k].mbtext, input.lines[k].num_chars, rules[j].mode == 'g');
            #ifdef REGEXP_DEBUG
                fprintf (stderr, "\"%s\"\n", newtext ? u32_strconv_to_output(newtext) : "NULL");
            #endif
            if (newtext == NULL) {
                return 1;
            }

            BFREE(input.lines[k].mbtext_org);  /* original address allocated for mbtext */
            input.lines[k].mbtext = newtext;
            input.lines[k].mbtext_org = newtext;

            analyze_line_ascii(input.lines + k);

            #ifdef REGEXP_DEBUG
                fprintf (stderr, "input.lines[%d] == {%d, \"%s\"}\n", (int) k,
                    (int) input.lines[k].num_chars, u32_strconv_to_output(input.lines[k].mbtext));
            #endif
        }
        opt.design->current_rule = NULL;
    }

    /*
     *  If text indentation was part of the lines processed, indentation
     *  may now be different -> recalculate input.indent.
     */
    if (opt.design->indentmode == 't') {
        int rc;
        rc = get_indent(input.lines, input.anz_lines);
        if (rc >= 0) {
            input.indent = (size_t) rc;
        } else {
            return 4;
        }
    }

    return 0;
}



static int has_linebreak(const uint32_t *s, const int len)
/*
 *  Determine if the given line of raw text is ended by a line break.
 *
 *  s: the string to check
 *  len: length of s in characters
 *
 *  RETURNS:  != 0   line break found
 *            == 0   line break not found
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
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



static int read_all_input(const int use_stdin)
/*
 *  Read entire input (possibly from stdin) and store it in 'input' array.
 *
 *  Tabs are expanded.
 *  Might allocate slightly more memory than it needs. Trade-off for speed.
 *
 *  use_stdin: flag indicating whether to read from stdin (use_stdin != 0)
 *             or use the data currently present in input (use_stdin == 0).
 *
 *  RETURNS:  != 0   on error (out of memory)
 *            == 0   on success
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char buf[LINE_MAX_BYTES + 3];    /* input buffer incl. newline + zero terminator */
    size_t len_chars;
    size_t input_size = 0;           /* number of elements allocated */
    uint32_t *mbtemp = NULL;         /* temp string for preparing the multi-byte input */
    size_t i;
    int rc;

    input.indent = LINE_MAX_BYTES;
    input.maxline = 0;

    if (use_stdin) {
        input.anz_lines = 0;

        /*
         *  Start reading
         */
        while (fgets(buf, LINE_MAX_BYTES + 2, opt.infile)) {
            if (input.anz_lines % 100 == 0) {
                input_size += 100;
                line_t *tmp = (line_t *) realloc(input.lines, input_size * sizeof(line_t));
                if (tmp == NULL) {
                    perror(PROJECT);
                    BFREE (input.lines);
                    return 1;
                }
                input.lines = tmp;
            }

            mbtemp = u32_strconv_from_input(buf);
            len_chars = u32_strlen(mbtemp);
            input.final_newline = has_linebreak(mbtemp, len_chars);
            input.lines[input.anz_lines].posmap = NULL;
            input.lines[input.anz_lines].tabpos = NULL;
            input.lines[input.anz_lines].tabpos_len = 0;

            if (opt.r) {
                if (is_char_at(mbtemp, len_chars - 1, char_newline)) {
                    set_char_at(mbtemp, len_chars - 1, char_nul);
                    --len_chars;
                }
                if (is_char_at(mbtemp, len_chars - 1, char_cr)) {
                    set_char_at(mbtemp, len_chars - 1, char_nul);
                    --len_chars;
                }
            }
            else {
                btrim32(mbtemp, &len_chars);
            }

            /*
             * Expand tabs
             */
            if (len_chars > 0) {
                uint32_t *temp = NULL;
                len_chars = expand_tabs_into(mbtemp, opt.tabstop, &temp,
                                             &(input.lines[input.anz_lines].tabpos),
                                             &(input.lines[input.anz_lines].tabpos_len));
                if (len_chars == 0) {
                    perror(PROJECT);
                    BFREE (input.lines);
                    return 1;
                }
                input.lines[input.anz_lines].mbtext = temp;
                BFREE(mbtemp);
                temp = NULL;
            }
            else {
                input.lines[input.anz_lines].mbtext = mbtemp;
            }
            input.lines[input.anz_lines].mbtext_org = input.lines[input.anz_lines].mbtext;
            input.lines[input.anz_lines].num_chars = len_chars;

            /*
             * Build ASCII equivalent of the multi-byte string, update line stats
             */
            input.lines[input.anz_lines].text = NULL;  /* we haven't used it yet! */
            analyze_line_ascii(input.lines + input.anz_lines);

            ++input.anz_lines;
        }

        if (ferror(stdin)) {
            perror(PROJECT);
            BFREE (input.lines);
            return 1;
        }
    }

    else {
        /* recalculate input statistics for redrawing the mended box */
        for (i = 0; i < input.anz_lines; ++i) {
            analyze_line_ascii(input.lines + i);
        }
    }

    /*
     *  Exit if there was no input at all
     */
    if (input.lines == NULL || input.lines[0].text == NULL) {
        return 0;
    }

    /*
     *  Compute indentation
     */
    rc = get_indent(input.lines, input.anz_lines);
    if (rc >= 0) {
        input.indent = (size_t) rc;
    } else {
        return 1;
    }

    /*
     *  Remove indentation, unless we want to preserve it (when removing
     *  a box or if the user wants to retain it inside the box)
     */
    if (opt.design->indentmode != 't' && opt.r == 0) {
        for (i = 0; i < input.anz_lines; ++i) {
            #ifdef DEBUG
                fprintf(stderr, "%2d: mbtext = \"%s\" (%d chars)\n", (int) i,
                    u32_strconv_to_output(input.lines[i].mbtext), (int) input.lines[i].num_chars);
            #endif
            if (input.lines[i].num_chars >= input.indent) {
                memmove(input.lines[i].text, input.lines[i].text + input.indent,
                        input.lines[i].len - input.indent + 1);
                input.lines[i].len -= input.indent;

                input.lines[i].mbtext = advance32(input.lines[i].mbtext, input.indent);
                input.lines[i].num_chars -= input.indent;
            }
            #ifdef DEBUG
                fprintf(stderr, "%2d: mbtext = \"%s\" (%d chars)\n", (int) i,
                    u32_strconv_to_output(input.lines[i].mbtext), (int) input.lines[i].num_chars);
            #endif
        }
        input.maxline -= input.indent;
    }

    /*
     *  Apply regular expression substitutions
     */
    if (opt.r == 0) {
        if (apply_substitutions(0) != 0) {
            return 1;
        }
    }

#ifdef DEBUG
    fprintf (stderr, "Encoding: %s\n", encoding);
    print_input_lines(NULL);
#endif

    return 0;
}



/*       _\|/_
         (o o)
 +----oOO-{_}-OOo------------------------------------------------------------+
 |                       P r o g r a m   S t a r t                           |
 +--------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int rc;                           /* general return code */
    size_t pad;
    int i;
    int saved_designwidth;            /* opt.design->minwith backup, used for mending */
    int saved_designheight;           /* opt.design->minheight backup, used for mending */

    #ifdef DEBUG
        fprintf (stderr, "BOXES STARTING ...\n");
    #endif

    /*
     *  Process command line options
     */
    #ifdef DEBUG
        fprintf (stderr, "Processing Command Line ...\n");
    #endif
    opt_t *parsed_opts = process_commandline(argc, argv);
    if (parsed_opts == NULL) {
        exit(EXIT_FAILURE);
    }
    if (parsed_opts->help) {
        usage_long(stdout);
        exit(EXIT_SUCCESS);
    }
    if (parsed_opts->version_requested) {
        printf("%s version %s\n", PROJECT, VERSION);
        exit(EXIT_SUCCESS);
    }
    memcpy(&opt, parsed_opts, sizeof(opt_t));
    BFREE(parsed_opts);

    /*
     * Store system character encoding
     */
    setlocale(LC_ALL, "");    /* switch from default "C" encoding to system encoding */
    encoding = check_encoding(opt.encoding, locale_charset());
    #ifdef DEBUG
        fprintf (stderr, "Character Encoding = %s\n", encoding);
    #endif

    /*
     *  Parse config file(s), then reset design pointer
     */
    char *config_file = discover_config_file(0);
    if (config_file == NULL) {
        exit(EXIT_FAILURE);
    }
    if (opt.cld == NULL) {
        size_t r_num_designs = 0;
        designs = parse_config_files(config_file, &r_num_designs);
        if (designs == NULL) {
            exit(EXIT_FAILURE);
        }
        anz_designs = (int) r_num_designs;
    }
    else {
        rc = build_design(&designs, opt.cld);
        if (rc) {
            exit(EXIT_FAILURE);
        }
        anz_designs = 1;
    }
    BFREE (opt.design);
    opt.design = designs;

    /*
     *  If "-l" option was given, list designs and exit.
     */
    if (opt.l) {
        rc = list_designs();
        exit(rc);
    }

    /*
     *  If "-q" option was given, print results of tag query and exit.
     */
    if (opt.query != NULL && opt.query[0] != NULL && !query_is_undoc()) {
        rc = query_by_tag();
        exit(rc);
    }

    /*
     *  Adjust box size and indentmode to command line specification
     *  Increase box width/height by width/height of empty sides in order
     *  to match appearance of box with the user's expectations (if -s).
     */
    if (opt.reqheight > (long) opt.design->minheight) {
        opt.design->minheight = opt.reqheight;
    }
    if (opt.reqwidth > (long) opt.design->minwidth) {
        opt.design->minwidth = opt.reqwidth;
    }
    if (opt.reqwidth) {
        if (empty_side(opt.design->shape, BRIG)) {
            opt.design->minwidth += opt.design->shape[SE].width;
        }
        if (empty_side(opt.design->shape, BLEF)) {
            opt.design->minwidth += opt.design->shape[NW].width;
        }
    }
    if (opt.reqheight) {
        if (empty_side(opt.design->shape, BTOP)) {
            opt.design->minheight += opt.design->shape[NW].height;
        }
        if (empty_side(opt.design->shape, BBOT)) {
            opt.design->minheight += opt.design->shape[SE].height;
        }
    }
    if (opt.indentmode) {
        opt.design->indentmode = opt.indentmode;
    }
    saved_designwidth = opt.design->minwidth;
    saved_designheight = opt.design->minheight;

    do {
        if (opt.mend == 1) {  /* Mending a box works in two phases: */
            opt.r = 0;        /* opt.mend == 2: remove box          */
        }
        --opt.mend;           /* opt.mend == 1: add it back         */
        opt.design->minwidth = saved_designwidth;
        opt.design->minheight = saved_designheight;

        /*
         *  Read input lines
         */
        #ifdef DEBUG
            fprintf (stderr, "Reading all input ...\n");
        #endif
        rc = read_all_input(opt.mend);
        if (rc) {
            exit(EXIT_FAILURE);
        }
        if (input.anz_lines == 0) {
            exit(EXIT_SUCCESS);
        }

        /*
         *  Adjust box size to fit requested padding value
         *  Command line-specified box size takes precedence over padding.
         */
        for (i = 0; i < NUM_SIDES; ++i) {
            if (opt.padding[i] > -1) {
                opt.design->padding[i] = opt.padding[i];
            }
        }
        pad = opt.design->padding[BTOP] + opt.design->padding[BBOT];
        if (pad > 0) {
            pad += input.anz_lines;
            pad += opt.design->shape[NW].height + opt.design->shape[SW].height;
            if (pad > opt.design->minheight) {
                if (opt.reqheight) {
                    for (i = 0; i < (int) (pad - opt.design->minheight); ++i) {
                        if (opt.design->padding[i % 2 ? BBOT : BTOP]) {
                            opt.design->padding[i % 2 ? BBOT : BTOP] -= 1;
                        } else if (opt.design->padding[i % 2 ? BTOP : BBOT]) {
                            opt.design->padding[i % 2 ? BTOP : BBOT] -= 1;
                        } else {
                            break;
                        }
                    }
                }
                else {
                    opt.design->minheight = pad;
                }
            }
        }
        pad = opt.design->padding[BLEF] + opt.design->padding[BRIG];
        if (pad > 0) {
            pad += input.maxline;
            pad += opt.design->shape[NW].width + opt.design->shape[NE].width;
            if (pad > opt.design->minwidth) {
                if (opt.reqwidth) {
                    for (i = 0; i < (int) (pad - opt.design->minwidth); ++i) {
                        if (opt.design->padding[i % 2 ? BRIG : BLEF]) {
                            opt.design->padding[i % 2 ? BRIG : BLEF] -= 1;
                        } else if (opt.design->padding[i % 2 ? BLEF : BRIG]) {
                            opt.design->padding[i % 2 ? BLEF : BRIG] -= 1;
                        } else {
                            break;
                        }
                    }
                }
                else {
                    opt.design->minwidth = pad;
                }
            }
        }

        if (opt.r) {
            /*
             *  Remove box
             */
            #ifdef DEBUG
                fprintf (stderr, "Removing Box ...\n");
            #endif
            if (opt.killblank == -1) {
                if (empty_side(opt.design->shape, BTOP) && empty_side(opt.design->shape, BBOT)) {
                    opt.killblank = 0;
                } else {
                    opt.killblank = 1;
                }
            }
            rc = remove_box();
            if (rc) {
                exit(EXIT_FAILURE);
            }
            rc = apply_substitutions(1);
            if (rc) {
                exit(EXIT_FAILURE);
            }
            output_input(opt.mend > 0);
        }

        else {
            /*
             *  Generate box
             */
            sentry_t *thebox;

            #ifdef DEBUG
                fprintf (stderr, "Generating Box ...\n");
            #endif
            thebox = (sentry_t *) calloc(NUM_SIDES, sizeof(sentry_t));
            if (thebox == NULL) {
                perror(PROJECT);
                exit(EXIT_FAILURE);
            }
            rc = generate_box(thebox);
            if (rc) {
                exit(EXIT_FAILURE);
            }
            output_box(thebox);
        }
    } while (opt.mend > 0);

    return EXIT_SUCCESS;
}

/*EOF*/                                                  /* vim: set sw=4: */
