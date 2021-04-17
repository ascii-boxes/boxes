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
#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <uniconv.h>

#include "boxes.h"
#include "cmdline.h"
#include "discovery.h"
#include "generate.h"
#include "input.h"
#include "list.h"
#include "parsing.h"
#include "query.h"
#include "remove.h"
#include "tools.h"
#include "unicode.h"



/*       _\|/_
         (o o)
 +----oOO-{_}-OOo------------------------------------------------------------+
 |                    G l o b a l   V a r i a b l e s                        |
 +--------------------------------------------------------------------------*/

design_t *designs = NULL;            /* available box designs */
int num_designs = 0;                 /* number of designs after parsing */

opt_t opt;                           /* command line options */

input_t input;                       /* input lines */


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



/**
 * Process command line options and store the result in the global `opt` struct. May exit the program.
 */
static void handle_command_line(int argc, char *argv[])
{
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
}



/**
 * Parse config file(s), then reset design pointer. May exit the program.
 */
static void handle_config_parsing()
{
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
        num_designs = (int) r_num_designs;
    }
    else {
        int rc = build_design(&designs, opt.cld);
        if (rc) {
            exit(EXIT_FAILURE);
        }
        num_designs = 1;
    }
    BFREE (opt.design);
    opt.design = designs;
}



/**
 * Adjust box size to command line specification.
 * Increase box width/height by width/height of empty sides in order to match appearance of box with the user's
 * expectations (if -s).
 */
static void apply_expected_size()
{
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
}



/**
 * Read all input lines and store the result in the global `input` structure. May exit the program.
 */
static void handle_input()
{
    #ifdef DEBUG
        fprintf (stderr, "Reading all input ...\n");
    #endif
    input_t *raw_input = NULL;
    if (opt.mend != 0) {
        raw_input = read_all_input();
        if (raw_input == NULL) {
            exit(EXIT_FAILURE);
        }
    }
    if (analyze_input(raw_input ? raw_input : &input)) {
        exit(EXIT_FAILURE);
    }
    if (raw_input) {
        memcpy(&input, raw_input, sizeof(input_t));
        BFREE(raw_input);
    }
    if (input.num_lines == 0) {
        exit(EXIT_SUCCESS);
    }
}



/**
 * Adjust box size to fit requested padding value.
 * Command line-specified box size takes precedence over padding.
 */
static void adjust_size_and_padding()
{
    for (int i = 0; i < NUM_SIDES; ++i) {
        if (opt.padding[i] > -1) {
            opt.design->padding[i] = opt.padding[i];
        }
    }

    size_t pad = opt.design->padding[BTOP] + opt.design->padding[BBOT];
    if (pad > 0) {
        pad += input.num_lines;
        pad += opt.design->shape[NW].height + opt.design->shape[SW].height;
        if (pad > opt.design->minheight) {
            if (opt.reqheight) {
                for (int i = 0; i < (int) (pad - opt.design->minheight); ++i) {
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
                for (int i = 0; i < (int) (pad - opt.design->minwidth); ++i) {
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
}



/**
 * Generate box. May exit the program.
 */
static void handle_generate_box()
{
    #ifdef DEBUG
        fprintf (stderr, "Generating Box ...\n");
    #endif
    sentry_t *thebox = (sentry_t *) calloc(NUM_SIDES, sizeof(sentry_t));
    if (thebox == NULL) {
        perror(PROJECT);
        exit(EXIT_FAILURE);
    }
    int rc = generate_box(thebox);
    if (rc) {
        exit(EXIT_FAILURE);
    }
    output_box(thebox);
}



/**
 * Remove box. May exit the program.
 */
static void handle_remove_box()
{
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
    int rc = remove_box();
    if (rc) {
        exit(EXIT_FAILURE);
    }
    rc = apply_substitutions(&input, 1);
    if (rc) {
        exit(EXIT_FAILURE);
    }
    output_input(opt.mend > 0);
}



/*       _\|/_
         (o o)
 +----oOO-{_}-OOo------------------------------------------------------------+
 |                       P r o g r a m   S t a r t                           |
 +--------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
    int rc;                           /* general return code */
    int saved_designwidth;            /* opt.design->minwith backup, used for mending */
    int saved_designheight;           /* opt.design->minheight backup, used for mending */

    #ifdef DEBUG
        fprintf (stderr, "BOXES STARTING ...\n");
    #endif

    handle_command_line(argc, argv);

    /* Store system character encoding */
    setlocale(LC_ALL, "");    /* switch from default "C" encoding to system encoding */
    encoding = check_encoding(opt.encoding, locale_charset());
    #ifdef DEBUG
        fprintf (stderr, "Character Encoding = %s\n", encoding);
    #endif

    handle_config_parsing();

    /* If "-l" option was given, list designs and exit. */
    if (opt.l) {
        rc = list_designs();
        exit(rc);
    }

    /* If "-q" option was given, print results of tag query and exit. */
    if (opt.query != NULL && opt.query[0] != NULL && !query_is_undoc()) {
        rc = query_by_tag();
        exit(rc);
    }

    apply_expected_size();
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

        handle_input();

        adjust_size_and_padding();

        if (opt.r) {
            handle_remove_box();
        }
        else {
            handle_generate_box();
        }
    } while (opt.mend > 0);

    return EXIT_SUCCESS;
}

/*EOF*/                                                  /* vim: set sw=4: */
