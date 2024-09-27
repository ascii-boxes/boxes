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
 * Box generation, i.e. the drawing of boxes
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <unistr.h>

#include "shape.h"
#include "boxes.h"
#include "logging.h"
#include "tools.h"
#include "unicode.h"
#include "generate.h"



static int horiz_precalc(const sentry_t *sarr,
                         size_t *topiltf, size_t *botiltf, size_t *hspace)
/*
 *  Calculate data for horizontal box side generation.
 *
 *  sarr     Array of shapes from the current design
 *
 *  topiltf  RESULT: individual lines (columns) to fill by shapes 1, 2, and 3
 *  botiltf          in top part of box (topiltf) and bottom part of box
 *  hspace   RESULT: number of columns excluding corners (sum over iltf)
 *
 *  RETURNS:  == 0   on success  (result values are set)
 *            != 0   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int tnumsh;               /* number of existent shapes in top part */
    int bnumsh;
    size_t twidth;            /* current hspace for top side */
    size_t bwidth;            /* current hspace for bottom side */
    int i;
    size_t target_width;      /* assumed text width for minimum box size */
    int btoggle, ttoggle;     /* for case 3 w/ 2 elastics */

    /*
     *  Initialize future result values
     */
    memset(topiltf, 0, (SHAPES_PER_SIDE - 2) * sizeof(size_t));
    memset(botiltf, 0, (SHAPES_PER_SIDE - 2) * sizeof(size_t));
    *hspace = 0;

    /*
     *  Ensure minimum width for the insides of a box in order to ensure
     *  minimum box size required by current design
     */
    if (input.maxline >= (opt.design->minwidth - sarr[north_side[0]].width -
            sarr[north_side[SHAPES_PER_SIDE - 1]].width)) {
        target_width = input.maxline;
    }
    else {
        target_width = opt.design->minwidth - sarr[north_side[0]].width -
                sarr[north_side[SHAPES_PER_SIDE - 1]].width;
    }

    /*
     *  Compute number of existent shapes in top and in bottom part
     */
    tnumsh = 0;
    bnumsh = 0;
    for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
        if (!isempty(sarr + north_side[i])) {
            tnumsh++;
        }
        if (!isempty(sarr + south_side[i])) {
            bnumsh++;
        }
    }

    if (is_debug_logging(MAIN)) {
        log_debug(__FILE__, MAIN, "in horiz_precalc:\n");
        log_debug(__FILE__, MAIN, "    opt.design->minwidth %d, input.maxline %d, target_width"
            " %d, tnumsh %d, bnumsh %d\n", (int) opt.design->minwidth,
             (int) input.maxline, (int) target_width, tnumsh, bnumsh);
    }

    twidth = 0;
    bwidth = 0;

    btoggle = 1;                         /* can be 1 or 3 */
    ttoggle = 1;

    do {
        shape_t *seite;                  /* ptr to north_side or south_side */
        size_t *iltf;                    /* ptr to botiltf or topiltf */
        size_t *res_hspace;              /* ptr to bwidth or twidth */
        int *stoggle;                    /* ptr to btoggle or ttoggle */
        int numsh;                       /* either bnumsh or tnumsh */

        /*
         *  Set pointers to the side which is currently shorter,
         *  so it will be advanced in this step.
         */
        if (twidth > bwidth) {           /* south (bottom) is behind */
            seite = south_side;
            iltf = botiltf;
            res_hspace = &bwidth;
            numsh = bnumsh;
            stoggle = &btoggle;
        }
        else {                           /* north (top) is behind */
            seite = north_side;
            iltf = topiltf;
            res_hspace = &twidth;
            numsh = tnumsh;
            stoggle = &ttoggle;
        }

        switch (numsh) {

            case 1:
                /*
                 *  only one shape -> it must be elastic
                 */
                for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
                    if (!isempty(&(sarr[seite[i]]))) {
                        if (iltf[i - 1] == 0 ||
                                *res_hspace < target_width ||
                                twidth != bwidth) {
                            iltf[i - 1] += sarr[seite[i]].width;
                            *res_hspace += sarr[seite[i]].width;
                        }
                        break;
                    }
                }
                break;

            case 2:
                /*
                 *  two shapes -> one must be elastic, the other must not
                 */
                for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
                    if (!isempty(sarr + seite[i]) && !(sarr[seite[i]].elastic)) {
                        if (iltf[i - 1] == 0) {
                            iltf[i - 1] += sarr[seite[i]].width;
                            *res_hspace += sarr[seite[i]].width;
                            break;
                        }
                    }
                }
                for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
                    if (!isempty(sarr + seite[i]) && sarr[seite[i]].elastic) {
                        if (iltf[i - 1] == 0 ||
                                *res_hspace < target_width ||
                                twidth != bwidth) {
                            iltf[i - 1] += sarr[seite[i]].width;
                            *res_hspace += sarr[seite[i]].width;
                        }
                        break;
                    }
                }
                break;

            case 3:
                /*
                 *  three shapes -> one or two of them must be elastic
                 *  If two are elastic, they are the two outer ones.
                 */
                for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
                    if (!(sarr[seite[i]].elastic) && iltf[i - 1] == 0) {
                        iltf[i - 1] += sarr[seite[i]].width;
                        *res_hspace += sarr[seite[i]].width;
                    }
                }
                if (sarr[seite[1]].elastic && sarr[seite[3]].elastic) {
                    if (iltf[*stoggle - 1] == 0 ||
                            *res_hspace < target_width ||
                            twidth != bwidth) {
                        *res_hspace += sarr[seite[*stoggle]].width;
                        iltf[*stoggle - 1] += sarr[seite[*stoggle]].width;
                    }
                    *stoggle = *stoggle == 1 ? 3 : 1;
                }
                else {
                    for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
                        if (sarr[seite[i]].elastic) {
                            if (iltf[i - 1] == 0 ||
                                    *res_hspace < target_width ||
                                    twidth != bwidth) {
                                iltf[i - 1] += sarr[seite[i]].width;
                                *res_hspace += sarr[seite[i]].width;
                            }
                            break;
                        }
                    }
                }
                break;

            default:
                bx_fprintf(stderr, "%s: internal error in horiz_precalc()\n", PROJECT);
                return 1;
        }
    } while (twidth != bwidth || twidth < target_width || bwidth < target_width);

    *hspace = twidth;                    /* return either one */

    return 0;                            /* all clear */
}



static int vert_precalc(const sentry_t *sarr,
                        size_t *leftiltf, size_t *rightiltf, size_t *vspace)
/*
 *  Calculate data for vertical box side generation.
 *
 *  sarr      Array of shapes from the current design
 *
 *  leftiltf  RESULT: individual lines to fill by shapes 1, 2, and 3
 *  rightiltf         in left part of box (leftiltf) and right part of box
 *  vspace    RESULT: number of columns excluding corners (sum over iltf)
 *
 *  RETURNS:  == 0   on success  (result values are set)
 *            != 0   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int lnumsh;               /* number of existent shapes in top part */
    int rnumsh;
    size_t lheight;           /* current vspace for top side */
    size_t rheight;           /* current vspace for bottom side */
    int i;
    size_t target_height;     /* assumed text height for minimum box size */
    int rtoggle, ltoggle;     /* for case 3 w/ 2 elastics */

    /*
     *  Initialize future result values
     */
    memset(leftiltf, 0, (SHAPES_PER_SIDE - 2) * sizeof(size_t));
    memset(rightiltf, 0, (SHAPES_PER_SIDE - 2) * sizeof(size_t));
    *vspace = 0;

    /*
     *  Ensure minimum height for insides of box in order to ensure
     *  minimum box size required by current design
     */
    if (input.num_lines >= (opt.design->minheight - sarr[west_side[0]].height -
            sarr[west_side[SHAPES_PER_SIDE - 1]].height)) {
        target_height = input.num_lines;
    }
    else {
        target_height = opt.design->minheight - sarr[west_side[0]].height -
                sarr[west_side[SHAPES_PER_SIDE - 1]].height;
    }

    /*
     *  Compute number of existent shapes in left and right part (1..3)
     */
    lnumsh = 0;
    rnumsh = 0;
    for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
        if (!isempty(sarr + west_side[i])) {
            lnumsh++;
        }
        if (!isempty(sarr + east_side[i])) {
            rnumsh++;
        }
    }

    lheight = 0;
    rheight = 0;

    rtoggle = 1;                         /* can be 1 or 3 */
    ltoggle = 1;

    do {
        shape_t *seite;                  /* ptr to west_side or east_side */
        size_t *iltf;                    /* ptr to rightiltf or leftiltf */
        size_t *res_vspace;              /* ptr to rheight or lheight */
        int *stoggle;                    /* ptr to rtoggle or ltoggle */
        int numsh;                       /* either rnumsh or lnumsh */

        /*
         *  Set pointers to the side which is currently shorter,
         *  so it will be advanced in this step.
         */
        if (lheight > rheight) {         /* east (right) is behind */
            seite = east_side;
            iltf = rightiltf;
            res_vspace = &rheight;
            numsh = rnumsh;
            stoggle = &rtoggle;
        }
        else {                           /* west (left) is behind */
            seite = west_side;
            iltf = leftiltf;
            res_vspace = &lheight;
            numsh = lnumsh;
            stoggle = &ltoggle;
        }

        switch (numsh) {

            case 1:
                /*
                 *  only one shape -> it must be elastic
                 */
                for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
                    if (!isempty(&(sarr[seite[i]]))) {
                        if (iltf[i - 1] == 0 ||
                                *res_vspace < target_height ||
                                lheight != rheight) {
                            iltf[i - 1] += sarr[seite[i]].height;
                            *res_vspace += sarr[seite[i]].height;
                        }
                        break;
                    }
                }
                break;

            case 2:
                /*
                 *  two shapes -> one must be elastic, the other must not
                 */
                for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
                    if (!isempty(sarr + seite[i]) && !(sarr[seite[i]].elastic)) {
                        if (iltf[i - 1] == 0) {
                            iltf[i - 1] += sarr[seite[i]].height;
                            *res_vspace += sarr[seite[i]].height;
                            break;
                        }
                    }
                }
                for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
                    if (!isempty(sarr + seite[i]) && sarr[seite[i]].elastic) {
                        if (iltf[i - 1] == 0 ||
                                *res_vspace < target_height ||
                                lheight != rheight) {
                            iltf[i - 1] += sarr[seite[i]].height;
                            *res_vspace += sarr[seite[i]].height;
                        }
                        break;
                    }
                }
                break;

            case 3:
                /*
                 *  three shapes -> one or two of them must be elastic
                 *  If two are elastic, they are the two outer ones.
                 */
                for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
                    if (!(sarr[seite[i]].elastic) && iltf[i - 1] == 0) {
                        iltf[i - 1] += sarr[seite[i]].height;
                        *res_vspace += sarr[seite[i]].height;
                    }
                }
                if (sarr[seite[1]].elastic && sarr[seite[3]].elastic) {
                    if (iltf[*stoggle - 1] == 0 ||
                            *res_vspace < target_height ||
                            lheight != rheight) {
                        *res_vspace += sarr[seite[*stoggle]].height;
                        iltf[*stoggle - 1] += sarr[seite[*stoggle]].height;
                    }
                    *stoggle = *stoggle == 1 ? 3 : 1;
                }
                else {
                    for (i = 1; i < SHAPES_PER_SIDE - 1; ++i) {
                        if (sarr[seite[i]].elastic) {
                            if (iltf[i - 1] == 0 ||
                                    *res_vspace < target_height ||
                                    lheight != rheight) {
                                iltf[i - 1] += sarr[seite[i]].height;
                                *res_vspace += sarr[seite[i]].height;
                            }
                            break;
                        }
                    }
                }
                break;

            default:
                bx_fprintf(stderr, "%s: internal error in vert_precalc()\n", PROJECT);
                return 1;
        }
    } while (lheight != rheight || lheight < target_height || rheight < target_height);

    *vspace = lheight;                   /* return either one */

    return 0;                            /* all clear */
}



/**
 * Calculate the maximum number of characters in a line of a horizontal side (top or bottom). This excludes corners,
 * which always belong to the vertical sides. This is needed for allocating space for the assembled sides, which will
 * include shapes multiple times, and also include invisible characters.
 * @param sarr all shapes of the current design
 * @param side the side to calculate (`north_side` or `south_side`)
 * @param iltf the numbers of times that a shape shall appear (array of *three* values)
 * @param target_width the number of columns we must reach
 * @param target_height the number of lines of the side
 * @return the number of characters(!), visible plus invisible, that suffice to store every line of that side
 */
static size_t horiz_chars_required(const sentry_t *sarr, const shape_t *side, size_t *iltf, size_t target_width,
    size_t target_height)
{
    size_t *lens = (size_t *) calloc(target_height, sizeof(size_t));
    size_t *iltf_copy = (size_t *) malloc(3 * sizeof(size_t));
    memcpy(iltf_copy, iltf, 3 * sizeof(size_t));

    int cshape = (side == north_side) ? 0 : 2;
    for (size_t j = 0; j < target_width; j += sarr[side[cshape + 1]].width) {
        while (iltf_copy[cshape] == 0) {
            cshape += (side == north_side) ? 1 : -1;
        }
        for (size_t line = 0; line < target_height; ++line) {
            lens[line] += sarr[side[cshape + 1]].mbcs[line]->num_chars;
        }
        iltf_copy[cshape] -= sarr[side[cshape + 1]].width;
    }

    size_t result = 0;
    for (size_t i = 0; i < target_height; i++) {
        if (lens[i] > result) {
            result = lens[i];
        }
    }

    BFREE(lens);
    BFREE(iltf_copy);
    log_debug(__FILE__, MAIN, "%s side required characters: %d\n",
        (side == north_side) ? "Top": "Bottom", (int) result);
    return result;
}



static int vert_assemble(const sentry_t *sarr, const shape_t *seite,
                         size_t *iltf, sentry_t *result)
/*
 *  RETURNS:  == 0   on success  (result values are set)
 *            != 0   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t j;
    size_t line;
    int cshape;                      /* current shape (idx to iltf) */

    size_t max_chars = horiz_chars_required(sarr, seite, iltf, result->width, result->height);
    uint32_t **mbcs_tmp = (uint32_t **) calloc(result->height, sizeof(uint32_t *));

    for (line = 0; line < result->height; ++line) {
        result->chars[line] = (char *) calloc(1, result->width + 1);
        if (result->chars[line] == NULL) {
            perror(PROJECT);
            if ((long) --line >= 0) {
                do {
                    BFREE (mbcs_tmp[line]);
                    BFREE (result->chars[line--]);
                } while ((long) line >= 0);
            }
            BFREE(mbcs_tmp);
            return 1;                    /* out of memory */
        }
        mbcs_tmp[line] = (uint32_t *) calloc(max_chars + 1, sizeof(uint32_t));
    }

    cshape = (seite == north_side) ? 0 : 2;

    for (j = 0; j < result->width; j += sarr[seite[cshape + 1]].width) {
        while (iltf[cshape] == 0) {
            cshape += (seite == north_side) ? 1 : -1;
        }
        for (line = 0; line < result->height; ++line) {
            strcat(result->chars[line], sarr[seite[cshape + 1]].chars[line]);
            u32_strcat(mbcs_tmp[line], sarr[seite[cshape + 1]].mbcs[line]->memory);
        }
        iltf[cshape] -= sarr[seite[cshape + 1]].width;
    }

    for (line = 0; line < result->height; ++line) {
        result->mbcs[line] = bxs_from_unicode(mbcs_tmp[line]);
    }

    BFREE(mbcs_tmp);
    return 0;                            /* all clear */
}



static void horiz_assemble(const sentry_t *sarr, const shape_t *seite,
                           size_t *iltf, sentry_t *result)
{
    size_t j;
    size_t sc;                       /* index to shape chars (lines) */
    int cshape;                      /* current shape (idx to iltf) */
    shape_t ctop, cbottom;

    if (seite == east_side) {
        ctop = seite[0];
        cbottom = seite[SHAPES_PER_SIDE - 1];
        cshape = 0;
    }
    else {
        ctop = seite[SHAPES_PER_SIDE - 1];
        cbottom = seite[0];
        cshape = 2;
    }

    for (j = 0; j < sarr[ctop].height; ++j) {
        result->chars[j] = sarr[ctop].chars[j];
        result->mbcs[j] = sarr[ctop].mbcs[j];
    }
    for (j = 0; j < sarr[cbottom].height; ++j) {
        result->chars[result->height - sarr[cbottom].height + j] = sarr[cbottom].chars[j];
        result->mbcs[result->height - sarr[cbottom].height + j] = sarr[cbottom].mbcs[j];
    }

    sc = 0;
    for (j = sarr[ctop].height; j < result->height - sarr[cbottom].height; ++j) {
        while (iltf[cshape] == 0) {
            if (seite == east_side) {
                ++cshape;
            } else {
                --cshape;
            }
            sc = 0;
        }
        if (sc == sarr[seite[cshape + 1]].height) {
            sc = 0;
        }
        result->chars[j] = sarr[seite[cshape + 1]].chars[sc];
        result->mbcs[j] = sarr[seite[cshape + 1]].mbcs[sc];
        ++sc;
        iltf[cshape] -= 1;
    }
}



static int horiz_generate(sentry_t *tresult, sentry_t *bresult)
/*
 *  Generate top and bottom parts of box (excluding corners).
 *
 *  RETURNS:  == 0  if successful (resulting char array is stored in [bt]result)
 *            != 0  on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t biltf[SHAPES_PER_SIDE - 2];     /* individual lines to fill (bottom) */
    size_t tiltf[SHAPES_PER_SIDE - 2];     /* individual lines to fill (top) */
    int rc;                                /* received return code */

    tresult->height = highest(opt.design->shape,
                              SHAPES_PER_SIDE, NW, NNW, N, NNE, NE);
    bresult->height = highest(opt.design->shape,
                              SHAPES_PER_SIDE, SW, SSW, S, SSE, SE);

    rc = horiz_precalc(opt.design->shape, tiltf, biltf, &(tresult->width));
    if (rc) {
        return rc;
    }
    bresult->width = tresult->width;

    if (is_debug_logging(MAIN)) {
        log_debug(__FILE__, MAIN, "Top side box rect width %d, height %d.\n",
                (int) tresult->width, (int) tresult->height);
        log_debug(__FILE__, MAIN, "Top columns to fill: %s %d, %s %d, %s %d.\n",
                shape_name[north_side[1]], (int) tiltf[0],
                shape_name[north_side[2]], (int) tiltf[1],
                shape_name[north_side[3]], (int) tiltf[2]);
        log_debug(__FILE__, MAIN, "Bottom side box rect width %d, height %d.\n",
                (int) bresult->width, (int) bresult->height);
        log_debug(__FILE__, MAIN, "Bottom columns to fill: %s %d, %s %d, %s %d.\n",
                shape_name[south_side[1]], (int) biltf[0],
                shape_name[south_side[2]], (int) biltf[1],
                shape_name[south_side[3]], (int) biltf[2]);
    }

    tresult->chars = (char **) calloc(tresult->height, sizeof(char *));
    tresult->mbcs = (bxstr_t **) calloc(tresult->height, sizeof(bxstr_t *));
    bresult->chars = (char **) calloc(bresult->height, sizeof(char *));
    bresult->mbcs = (bxstr_t **) calloc(bresult->height, sizeof(bxstr_t *));
    if (tresult->chars == NULL || bresult->chars == NULL) {
        return 1;
    }

    rc = vert_assemble(opt.design->shape, north_side, tiltf, tresult);
    if (rc) {
        return rc;
    }
    rc = vert_assemble(opt.design->shape, south_side, biltf, bresult);
    if (rc) {
        return rc;
    }

    /*
     *  Debugging code - Output horizontal sides of box
     */
    if (is_debug_logging(MAIN)) {
        size_t j;
        log_debug(__FILE__, MAIN, "TOP SIDE:\n");
        for (j = 0; j < tresult->height; ++j) {
            char *out_sl = bxs_to_output(tresult->mbcs[j]);
            log_debug(__FILE__, MAIN, "  %2d: \'%s\' - \'%s\'\n", (int) j, out_sl, tresult->chars[j]);
            BFREE(out_sl);
        }
        log_debug(__FILE__, MAIN, "BOTTOM SIDE:\n");
        for (j = 0; j < bresult->height; ++j) {
            char *out_sl = bxs_to_output(bresult->mbcs[j]);
            log_debug(__FILE__, MAIN, "  %2d: \'%s\' - '%s'\n", (int) j, out_sl, bresult->chars[j]);
            BFREE(out_sl);
        }
    }
    return 0;                            /* all clear */
}



static int vert_generate(sentry_t *lresult, sentry_t *rresult)
/*
 *  Generate vertical sides of box.
 *
 *  RETURNS:  == 0   on success  (resulting char array is stored in [rl]result)
 *            != 0   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t vspace = 0;
    size_t leftiltf[SHAPES_PER_SIDE - 2];  /* individual lines to fill */
    size_t rightiltf[SHAPES_PER_SIDE - 2]; /* individual lines to fill */
    int rc;                                /* received return code */

    lresult->width = widest(opt.design->shape,
                            SHAPES_PER_SIDE, SW, WSW, W, WNW, NW);
    rresult->width = widest(opt.design->shape,
                            SHAPES_PER_SIDE, SE, ESE, E, ENE, NE);

    rc = vert_precalc(opt.design->shape, leftiltf, rightiltf, &vspace);
    if (rc) {
        return rc;
    }

    lresult->height = vspace +
            opt.design->shape[NW].height + opt.design->shape[SW].height;
    rresult->height = vspace +
            opt.design->shape[NE].height + opt.design->shape[SE].height;

    if (is_debug_logging(MAIN)) {
        log_debug(__FILE__, MAIN, "Left side box rect width %d, height %d, vspace %d.\n",
                (int) lresult->width, (int) lresult->height, (int) vspace);
        log_debug(__FILE__, MAIN, "Left lines to fill: %s %d, %s %d, %s %d.\n",
                shape_name[west_side[1]], (int) leftiltf[0],
                shape_name[west_side[2]], (int) leftiltf[1],
                shape_name[west_side[3]], (int) leftiltf[2]);
        log_debug(__FILE__, MAIN, "Right side box rect width %d, height %d, vspace %d.\n",
                (int) rresult->width, (int) rresult->height, (int) vspace);
        log_debug(__FILE__, MAIN, "Right lines to fill: %s %d, %s %d, %s %d.\n",
                shape_name[east_side[1]], (int) rightiltf[0],
                shape_name[east_side[2]], (int) rightiltf[1],
                shape_name[east_side[3]], (int) rightiltf[2]);
    }

    lresult->chars = (char **) calloc(lresult->height, sizeof(char *));
    if (lresult->chars == NULL) {
        return 1;
    }
    lresult->mbcs = (bxstr_t **) calloc(lresult->height, sizeof(bxstr_t *));
    if (lresult->mbcs == NULL) {
        return 1;
    }
    rresult->chars = (char **) calloc(rresult->height, sizeof(char *));
    if (rresult->chars == NULL) {
        return 1;
    }
    rresult->mbcs = (bxstr_t **) calloc(rresult->height, sizeof(bxstr_t *));
    if (rresult->mbcs == NULL) {
        return 1;
    }

    horiz_assemble(opt.design->shape, west_side, leftiltf, lresult);
    horiz_assemble(opt.design->shape, east_side, rightiltf, rresult);

    /*
     *  Debugging code - Output left and right side of box
     */
    if (is_debug_logging(MAIN)) {
        size_t j;
        log_debug(__FILE__, MAIN, "LEFT SIDE:\n");
        for (j = 0; j < lresult->height; ++j) {
            char *out_sl = bxs_to_output(lresult->mbcs[j]);
            log_debug(__FILE__, MAIN, "  %2d: \'%s\' - \'%s\'\n", (int) j, out_sl, lresult->chars[j]);
            BFREE(out_sl);
        }
        log_debug(__FILE__, MAIN, "RIGHT SIDE:\n");
        for (j = 0; j < rresult->height; ++j) {
            char *out_sl = bxs_to_output(rresult->mbcs[j]);
            log_debug(__FILE__, MAIN, "  %2d: \'%s\' - \'%s\'\n", (int) j, out_sl, rresult->chars[j]);
            BFREE(out_sl);
        }
    }
    return 0;                            /* all clear */
}



int generate_box(sentry_t *thebox)
/*
 *
 *  RETURNS:  == 0  if successful  (thebox is set)
 *            != 0  on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int rc;
    int i;

    if (is_debug_logging(MAIN)) {
        for (i = 0; i < NUM_SHAPES; i++) {
            debug_print_shape(opt.design->shape + i);
        }
    }

    rc = horiz_generate(&(thebox[0]), &(thebox[2]));
    if (rc) {
        goto err;
    }

    rc = vert_generate(&(thebox[3]), &(thebox[1]));
    if (rc) {
        goto err;
    }

    return 0;                            /* all clear */

    err:
    for (i = 0; i < NUM_SIDES; ++i) {
        if (!isempty(&(thebox[i]))) {
            BFREE (thebox[i].chars);     /* free only pointer array */
            memset(thebox + i, 0, sizeof(sentry_t));
        }
    }
    return rc;                           /* error */
}



static int justify_line(line_t *line, int skew)
/*
 *  Justify input line according to specified justification
 *
 *     line   line to justify
 *     skew   difference in spaces right/left of text block (hpr-hpl)
 *
 *  line is assumed to be already free of trailing whitespace.
 *
 *  RETURNS:  number of space characters which must be added to (> 0)
 *            or removed from (< 0) the beginning of the line
 *            A return value of 0 means "nothing to do".
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    if (empty_line(line)) {
        return 0;
    }
    if (opt.justify == '\0') {
        return 0;
    }

    if (is_debug_logging(MAIN)) {
        char *outtext = bxs_to_output(line->text);
        log_debug(__FILE__, MAIN,
            "justify_line(%c):  Input: real: (%02d) \"%s\", text: (%02d) \"%s\", invisible=%d, skew=%d",
            opt.justify ? opt.justify : '0', (int) line->text->num_chars, outtext, (int) line->text->num_columns,
            line->text->ascii, (int) line->text->num_chars_invisible, skew);
        BFREE(outtext);
    }

    int result = 0;
    size_t initial_space_size = line->text->indent;
    size_t newlen = line->text->num_columns - initial_space_size;
    size_t shift;

    switch (opt.justify) {

        case 'l':
            if (opt.design->indentmode == 't') {
                /* text indented inside of box */
                result = (int) input.indent - (int) initial_space_size;
            }
            else {
                result = -1 * (int) initial_space_size;
            }
            break;

        case 'c':
            if (opt.design->indentmode == 't') {
                /* text indented inside of box */
                shift = (input.maxline - input.indent - newlen) / 2 + input.indent;
                skew -= input.indent;
                if ((input.maxline - input.indent - newlen) % 2 && skew == 1) {
                    ++shift;
                }
            }
            else {
                shift = (input.maxline - newlen) / 2;
                if ((input.maxline - newlen) % 2 && skew == 1) {
                    ++shift;
                }
            }
            result = (int) shift - (int) initial_space_size;
            break;

        case 'r':
            shift = input.maxline - newlen;
            result = (int) shift - (int) initial_space_size;
            break;

        default:
            bx_fprintf(stderr, "%s: internal error (unknown justify option: %c)\n", PROJECT, opt.justify);
            result = 0;
    }

    log_debug_cont(MAIN, " -> %d\n", result);
    return result;
}



int output_box(const sentry_t *thebox)
/*
 *  Generate final output using the previously generated box parts.
 *
 *    thebox    Array of four shapes which contain the previously generated
 *              box parts in the following order: BTOP, BRIG, BBOT, BLEF
 *
 *  RETURNS:  == 0  if successful
 *            != 0  on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t j;
    size_t nol = thebox[BRIG].height;   /* number of output lines */
    size_t vfill, vfill1, vfill2;       /* empty lines/columns in box */
    size_t hfill;
    uint32_t *hfill1, *hfill2;          /* space before/after text */
    size_t hpl, hpr;
    size_t skip_start;                  /* lines to skip for box top */
    size_t skip_end;                    /* lines to skip for box bottom */
    size_t skip_left;                   /* true if left box part is to be skipped */
    size_t ntabs, nspcs;                /* needed for unexpand of tabs */

    log_debug(__FILE__, MAIN, "Padding used: left %d, top %d, right %d, bottom %d\n",
        opt.design->padding[BLEF], opt.design->padding[BTOP],
        opt.design->padding[BRIG], opt.design->padding[BBOT]);

    /*
     *  Create string of spaces for indentation
     */
    uint32_t *indentspc = NULL;
    size_t indentspclen = 0;
    ntabs = nspcs = 0;
    if (opt.design->indentmode == 'b') {
        if (opt.tabexp == 'u') {
            ntabs = input.indent / opt.tabstop;
            nspcs = input.indent % opt.tabstop;
            indentspclen = ntabs + nspcs;
        }
        else {
            indentspclen = input.indent;
        }

        indentspc = (uint32_t *) malloc((indentspclen + 1) * sizeof(uint32_t));
        if (indentspc == NULL) {
            perror(PROJECT);
            return 1;
        }

        if (opt.tabexp == 'u') {
            u32_set(indentspc, char_tab, ntabs);
            u32_set(indentspc + ntabs, char_space, nspcs);
        }
        else {
            u32_set(indentspc, char_space, indentspclen);
        }
        set_char_at(indentspc, indentspclen, char_nul);
    }
    else {
        indentspc = new_empty_string32();
        if (indentspc == NULL) {
            perror(PROJECT);
            return 1;
        }
    }

    /*
     *  Compute number of empty lines in box (vfill).
     */
    vfill = nol - thebox[BTOP].height - thebox[BBOT].height - input.num_lines;
    vfill -= opt.design->padding[BTOP] + opt.design->padding[BBOT];
    if (opt.valign == 'c') {
        vfill1 = vfill / 2;
        vfill2 = vfill1 + (vfill % 2);
    }
    else if (opt.valign == 'b') {
        vfill1 = vfill;
        vfill2 = 0;
    }
    else {
        vfill1 = 0;
        vfill2 = vfill;
    }
    vfill1 += opt.design->padding[BTOP];
    vfill2 += opt.design->padding[BBOT];
    vfill += opt.design->padding[BTOP] + opt.design->padding[BBOT];

    /*
     *  Provide strings for horizontal text alignment.
     */
    hfill = thebox[BTOP].width - input.maxline;
    hfill1 = (uint32_t *) malloc((hfill + 1) * sizeof(uint32_t));
    hfill2 = (uint32_t *) malloc((hfill + 1) * sizeof(uint32_t));
    if (!hfill1 || !hfill2) {
        perror(PROJECT);
        return 1;
    }
    u32_set(hfill1, char_space, hfill);
    u32_set(hfill2, char_space, hfill);
    set_char_at(hfill1, hfill, char_nul);
    set_char_at(hfill2, hfill, char_nul);
    hpl = 0;
    hpr = 0;
    if (hfill == 1) {
        if (opt.halign == 'r'
                || opt.design->padding[BLEF] > opt.design->padding[BRIG]) {
            hpl = 1;
            hpr = 0;
        }
        else {
            hpl = 0;
            hpr = 1;
        }
    }
    else {
        hfill -= opt.design->padding[BLEF] + opt.design->padding[BRIG];
        if (opt.halign == 'c') {
            hpl = hfill / 2 + opt.design->padding[BLEF];
            hpr = hfill / 2 + opt.design->padding[BRIG] + (hfill % 2);
        }
        else if (opt.halign == 'r') {
            hpl = hfill + opt.design->padding[BLEF];
            hpr = opt.design->padding[BRIG];
        }
        else {
            hpl = opt.design->padding[BLEF];
            hpr = hfill + opt.design->padding[BRIG];
        }
        hfill += opt.design->padding[BLEF] + opt.design->padding[BRIG];
    }
    set_char_at(hfill1, hpl, char_nul);
    set_char_at(hfill2, hpr, char_nul);

    if (is_debug_logging(MAIN)) {
        log_debug(__FILE__, MAIN, "Alignment: hfill %d hpl %d hpr %d, vfill %d vfill1 %d vfill2 %d.\n",
                (int) hfill, (int) hpl, (int) hpr, (int) vfill, (int) vfill1, (int) vfill2);
        char *out_hfill1 = u32_strconv_to_output(hfill1);
        char *out_hfill2 = u32_strconv_to_output(hfill2);
        char *out_indentspc = u32_strconv_to_output(indentspc);
        log_debug(__FILE__, MAIN, "           hfill1 = \"%s\";  hfill2 = \"%s\";  indentspc = \"%s\";\n",
                out_hfill1, out_hfill2, out_indentspc);
        BFREE(out_indentspc);
        BFREE(out_hfill2);
        BFREE(out_hfill1);
    }

    /*
     *  Find out if and how many leading or trailing blank lines must be
     *  skipped because the corresponding box side was defined empty.
     */
    skip_start = 0;
    skip_end = 0;
    skip_left = 0;
    if (empty_side(opt.design->shape, BTOP)) {
        skip_start = opt.design->shape[NW].height;
    }
    if (empty_side(opt.design->shape, BBOT)) {
        skip_end = opt.design->shape[SW].height;
    }
    if (empty_side(opt.design->shape, BLEF)) {
        skip_left = opt.design->shape[NW].width;
    } /* could simply be 1, though */
    log_debug(__FILE__, MAIN, "skip_start = %d;  skip_end = %d;  skip_left = %d;  nol = %d;\n",
            (int) skip_start, (int) skip_end, (int) skip_left, (int) nol);

    /*
     *  Generate actual output
     */
    bxstr_t *obuf = NULL;           /* final output string */
    uint32_t *restored_indent;
    uint32_t *empty_string = new_empty_string32();
    for (j = skip_start; j < nol - skip_end; ++j) {

        if (j < thebox[BTOP].height) {   /* box top */
            restored_indent = tabbify_indent(0, indentspc, indentspclen);
            obuf = bxs_concat(4, restored_indent, 
                            skip_left ? empty_string : thebox[BLEF].mbcs[j]->memory,
                            thebox[BTOP].mbcs[j]->memory,
                            thebox[BRIG].mbcs[j]->memory);
        }

        else if (vfill1) {               /* top vfill */
            restored_indent = tabbify_indent(0, indentspc, indentspclen);
            uint32_t *wspc = u32_nspaces(thebox[BTOP].width);
            obuf = bxs_concat(4, restored_indent,
                            skip_left ? empty_string : thebox[BLEF].mbcs[j]->memory,
                            wspc,
                            thebox[BRIG].mbcs[j]->memory);
            --vfill1;
            BFREE(wspc);
        }

        else if (j < nol - thebox[BBOT].height) {
            long ti = j - thebox[BTOP].height - (vfill - vfill2);
            if (ti < (long) input.num_lines) {      /* box content (lines) */
                int shift = justify_line(input.lines + ti, hpr - hpl);
                restored_indent = tabbify_indent(ti, indentspc, indentspclen);
                bxstr_t *text_shifted = bxs_cut_front(input.lines[ti].text, shift < 0 ? (size_t) (-shift) : 0);
                uint32_t *spc1 = empty_string;
                if (ti >= 0 && shift > 0) {
                    spc1 = u32_nspaces(shift);
                }
                uint32_t *spc2 = u32_nspaces(input.maxline - input.lines[ti].text->num_columns - shift);
                obuf = bxs_concat(8, restored_indent,
                                skip_left ? empty_string : thebox[BLEF].mbcs[j]->memory, hfill1, spc1,
                                ti >= 0 ? text_shifted->memory : empty_string, hfill2, spc2,
                                thebox[BRIG].mbcs[j]->memory);
                bxs_free(text_shifted);
                if (spc1 != empty_string) {
                    BFREE(spc1);
                }
                BFREE(spc2);
            }
            else {                       /* bottom vfill */
                restored_indent = tabbify_indent(input.num_lines - 1, indentspc, indentspclen);
                uint32_t *spc = u32_nspaces(thebox[BTOP].width);
                obuf = bxs_concat(4, restored_indent,
                                skip_left ? empty_string : thebox[BLEF].mbcs[j]->memory,
                                spc,
                                thebox[BRIG].mbcs[j]->memory);
                BFREE(spc);
            }
        }

        else {                           /* box bottom */
            restored_indent = tabbify_indent(input.num_lines - 1, indentspc, indentspclen);
            obuf = bxs_concat(4, restored_indent,
                            skip_left ? empty_string : thebox[BLEF].mbcs[j]->memory,
                            thebox[BBOT].mbcs[j - (nol - thebox[BBOT].height)]->memory,
                            thebox[BRIG].mbcs[j]->memory);
        }

        bxstr_t *obuf_trimmed = bxs_rtrim(obuf);
        fprintf(opt.outfile, "%s%s", bxs_to_output(obuf_trimmed),
                    (input.final_newline || j < nol - skip_end - 1 ? opt.eol : ""));

        bxs_free(obuf);
        bxs_free(obuf_trimmed);
        if (opt.tabexp == 'k') {
            BFREE(restored_indent);
        }
    }

    BFREE (indentspc);
    BFREE (empty_string);
    BFREE (hfill1);
    BFREE (hfill2);
    return 0;                            /* all clear */
}



/* vim: set cindent sw=4: */
