/*
 * boxes - Command line filter to draw/remove ASCII boxes around text
 * Copyright (C) 1999  Thomas Jensen and the boxes contributors
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
 * Box generation, i.e. the drawing of boxes
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "shape.h"
#include "boxes.h"
#include "tools.h"
#include "generate.h"


static int horiz_precalc (const sentry_t *sarr,
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
    int    tnumsh;               /* number of existent shapes in top part */
    int    bnumsh;
    size_t twidth;               /* current hspace for top side */
    size_t bwidth;               /* current hspace for bottom side */
    int    i;
    size_t target_width;         /* assumed text width for minimum box size */
    int    btoggle, ttoggle;     /* for case 3 w/ 2 elastics */

    /*
     *  Initialize future result values
     */
    memset (topiltf, 0, (SHAPES_PER_SIDE-2) * sizeof(size_t));
    memset (botiltf, 0, (SHAPES_PER_SIDE-2) * sizeof(size_t));
    *hspace = 0;

    /*
     *  Ensure minimum width for the insides of a box in order to ensure
     *  minimum box size required by current design
     */
    if (input.maxline >= (opt.design->minwidth - sarr[north_side[0]].width -
            sarr[north_side[SHAPES_PER_SIDE-1]].width)) {
        target_width = input.maxline;
    }
    else {
        target_width = opt.design->minwidth - sarr[north_side[0]].width -
            sarr[north_side[SHAPES_PER_SIDE-1]].width;
    }

    /*
     *  Compute number of existent shapes in top and in bottom part
     */
    tnumsh = 0;  bnumsh = 0;
    for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
        if (!isempty(sarr+north_side[i])) tnumsh++;
        if (!isempty(sarr+south_side[i])) bnumsh++;
    }

    #ifdef DEBUG
        fprintf (stderr, "in horiz_precalc:\n    ");
        fprintf (stderr, "opt.design->minwidth %d, input.maxline %d, target_width"
                " %d, tnumsh %d, bnumsh %d\n", opt.design->minwidth,
                input.maxline, target_width, tnumsh, bnumsh);
    #endif

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
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!isempty(&(sarr[seite[i]]))) {
                        if (iltf[i-1] == 0 ||
                            *res_hspace < target_width ||
                            twidth != bwidth)
                        {
                            iltf[i-1] += sarr[seite[i]].width;
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
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!isempty (sarr+seite[i]) && !(sarr[seite[i]].elastic)) {
                        if (iltf[i-1] == 0) {
                            iltf[i-1] += sarr[seite[i]].width;
                            *res_hspace += sarr[seite[i]].width;
                            break;
                        }
                    }
                }
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!isempty (sarr+seite[i]) && sarr[seite[i]].elastic) {
                        if (iltf[i-1] == 0 ||
                            *res_hspace < target_width ||
                            twidth != bwidth)
                        {
                            iltf[i-1] += sarr[seite[i]].width;
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
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!(sarr[seite[i]].elastic) && iltf[i-1] == 0) {
                        iltf[i-1] += sarr[seite[i]].width;
                        *res_hspace += sarr[seite[i]].width;
                    }
                }
                if (sarr[seite[1]].elastic && sarr[seite[3]].elastic) {
                    if (iltf[*stoggle-1] == 0 ||
                        *res_hspace < target_width ||
                        twidth != bwidth)
                    {
                        *res_hspace += sarr[seite[*stoggle]].width;
                        iltf[*stoggle-1] += sarr[seite[*stoggle]].width;
                    }
                    *stoggle = *stoggle == 1? 3 : 1;
                }
                else {
                    for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                        if (sarr[seite[i]].elastic) {
                            if (iltf[i-1] == 0 ||
                                *res_hspace < target_width ||
                                twidth != bwidth)
                            {
                                iltf[i-1] += sarr[seite[i]].width;
                                *res_hspace += sarr[seite[i]].width;
                            }
                            break;
                        }
                    }
                }
                break;

            default:
                fprintf (stderr, "%s: internal error in horiz_precalc()\n", PROJECT);
                return 1;
        }

    } while (twidth != bwidth || twidth < target_width || bwidth < target_width);

    *hspace = twidth;                    /* return either one */

    return 0;                            /* all clear */
}



static int vert_precalc (const sentry_t *sarr,
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
    int    lnumsh;               /* number of existent shapes in top part */
    int    rnumsh;
    size_t lheight;              /* current vspace for top side */
    size_t rheight;              /* current vspace for bottom side */
    int    i;
    size_t target_height;        /* assumed text height for minimum box size */
    int    rtoggle, ltoggle;     /* for case 3 w/ 2 elastics */

    /*
     *  Initialize future result values
     */
    memset (leftiltf, 0, (SHAPES_PER_SIDE-2) * sizeof(size_t));
    memset (rightiltf, 0, (SHAPES_PER_SIDE-2) * sizeof(size_t));
    *vspace = 0;

    /*
     *  Ensure minimum height for insides of box in order to ensure
     *  minimum box size required by current design
     */
    if (input.anz_lines >= (opt.design->minheight - sarr[west_side[0]].height -
            sarr[west_side[SHAPES_PER_SIDE-1]].height)) {
        target_height = input.anz_lines;
    }
    else {
        target_height = opt.design->minheight - sarr[west_side[0]].height -
            sarr[west_side[SHAPES_PER_SIDE-1]].height;
    }

    /*
     *  Compute number of existent shapes in left and right part (1..3)
     */
    lnumsh = 0;  rnumsh = 0;
    for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
        if (!isempty(sarr+west_side[i])) lnumsh++;
        if (!isempty(sarr+east_side[i])) rnumsh++;
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
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!isempty(&(sarr[seite[i]]))) {
                        if (iltf[i-1] == 0 ||
                            *res_vspace < target_height ||
                            lheight != rheight)
                        {
                            iltf[i-1] += sarr[seite[i]].height;
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
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!isempty (sarr+seite[i]) && !(sarr[seite[i]].elastic)) {
                        if (iltf[i-1] == 0) {
                            iltf[i-1] += sarr[seite[i]].height;
                            *res_vspace += sarr[seite[i]].height;
                            break;
                        }
                    }
                }
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!isempty (sarr+seite[i]) && sarr[seite[i]].elastic) {
                        if (iltf[i-1] == 0 ||
                            *res_vspace < target_height ||
                            lheight != rheight)
                        {
                            iltf[i-1] += sarr[seite[i]].height;
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
                for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                    if (!(sarr[seite[i]].elastic) && iltf[i-1] == 0) {
                        iltf[i-1] += sarr[seite[i]].height;
                        *res_vspace += sarr[seite[i]].height;
                    }
                }
                if (sarr[seite[1]].elastic && sarr[seite[3]].elastic) {
                    if (iltf[*stoggle-1] == 0 ||
                        *res_vspace < target_height ||
                        lheight != rheight)
                    {
                        *res_vspace += sarr[seite[*stoggle]].height;
                        iltf[*stoggle-1] += sarr[seite[*stoggle]].height;
                    }
                    *stoggle = *stoggle == 1? 3 : 1;
                }
                else {
                    for (i=1; i<SHAPES_PER_SIDE-1; ++i) {
                        if (sarr[seite[i]].elastic) {
                            if (iltf[i-1] == 0 ||
                                *res_vspace < target_height ||
                                lheight != rheight)
                            {
                                iltf[i-1] += sarr[seite[i]].height;
                                *res_vspace += sarr[seite[i]].height;
                            }
                            break;
                        }
                    }
                }
                break;

            default:
                fprintf (stderr, "%s: internal error in vert_precalc()\n", PROJECT);
                return 1;
        }

    } while (lheight != rheight || lheight < target_height || rheight < target_height);

    *vspace = lheight;                   /* return either one */

    return 0;                            /* all clear */
}



static int vert_assemble (const sentry_t *sarr, const shape_t *seite,
        size_t *iltf, sentry_t *result)
/*
 *
 *  RETURNS:  == 0   on success  (result values are set)
 *            != 0   on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t  j;
    size_t  line;
    int     cshape;                      /* current shape (idx to iltf) */

    for (line=0; line<result->height; ++line) {
        result->chars[line] = (char *) calloc (1, result->width+1);
        if (result->chars[line] == NULL) {
            perror (PROJECT);
            if ((long)--line >= 0) do {
                BFREE (result->chars[line--]);
            } while ((long)line >= 0);
            return 1;                    /* out of memory */
        }
    }

    cshape = (seite == north_side)? 0 : 2;

    for (j=0; j<result->width; j+=sarr[seite[cshape+1]].width) {
        while (iltf[cshape] == 0)
            cshape += (seite == north_side)? 1 : -1;
        for (line=0; line<result->height; ++line)
            strcat (result->chars[line], sarr[seite[cshape+1]].chars[line]);
        iltf[cshape] -= sarr[seite[cshape+1]].width;
    }

    return 0;                            /* all clear */
}



static void horiz_assemble (const sentry_t *sarr, const shape_t *seite,
        size_t *iltf, sentry_t *result)
{
    size_t  j;
    size_t  sc;                          /* index to shape chars (lines) */
    int     cshape;                      /* current shape (idx to iltf) */
    shape_t ctop, cbottom;

    if (seite == east_side) {
        ctop = seite[0];
        cbottom = seite[SHAPES_PER_SIDE-1];
        cshape = 0;
    }
    else {
        ctop = seite[SHAPES_PER_SIDE-1];
        cbottom = seite[0];
        cshape = 2;
    }

    for (j=0; j<sarr[ctop].height; ++j)
        result->chars[j] = sarr[ctop].chars[j];
    for (j=0; j<sarr[cbottom].height; ++j)
        result->chars[result->height-sarr[cbottom].height+j] =
            sarr[cbottom].chars[j];

    sc = 0;
    for (j=sarr[ctop].height; j < result->height-sarr[cbottom].height; ++j)
    {
        while (iltf[cshape] == 0) {
            if (seite == east_side)
                ++cshape;
            else
                --cshape;
            sc = 0;
        }
        if (sc == sarr[seite[cshape+1]].height)
            sc = 0;
        result->chars[j] = sarr[seite[cshape+1]].chars[sc];
        ++sc;
        iltf[cshape] -= 1;
    }
}



static int horiz_generate (sentry_t *tresult, sentry_t *bresult)
/*
 *  Generate top and bottom parts of box (excluding corners).
 *
 *  RETURNS:  == 0  if successful (resulting char array is stored in [bt]result)
 *            != 0  on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t biltf[SHAPES_PER_SIDE-2];     /* individual lines to fill (bottom) */
    size_t tiltf[SHAPES_PER_SIDE-2];     /* individual lines to fill (top) */
    int    rc;                           /* received return code */

    tresult->height = highest (opt.design->shape,
            SHAPES_PER_SIDE, NW, NNW, N, NNE, NE);
    bresult->height = highest (opt.design->shape,
            SHAPES_PER_SIDE, SW, SSW, S, SSE, SE);

    rc = horiz_precalc (opt.design->shape, tiltf, biltf, &(tresult->width));
    if (rc) return rc;
    bresult->width = tresult->width;

    #ifdef DEBUG
        fprintf (stderr, "Top side box rect width %d, height %d.\n",
                tresult->width, tresult->height);
        fprintf (stderr, "Top columns to fill: %s %d, %s %d, %s %d.\n",
                shape_name[north_side[1]], tiltf[0],
                shape_name[north_side[2]], tiltf[1],
                shape_name[north_side[3]], tiltf[2]);
        fprintf (stderr, "Bottom side box rect width %d, height %d.\n",
                bresult->width, bresult->height);
        fprintf (stderr, "Bottom columns to fill: %s %d, %s %d, %s %d.\n",
                shape_name[south_side[1]], biltf[0],
                shape_name[south_side[2]], biltf[1],
                shape_name[south_side[3]], biltf[2]);
    #endif

    tresult->chars = (char **) calloc (tresult->height, sizeof(char *));
    bresult->chars = (char **) calloc (bresult->height, sizeof(char *));
    if (tresult->chars == NULL || bresult->chars == NULL) return 1;

    rc = vert_assemble (opt.design->shape, north_side, tiltf, tresult);
    if (rc) return rc;
    rc = vert_assemble (opt.design->shape, south_side, biltf, bresult);
    if (rc) return rc;

    #if defined(DEBUG) && 1
    {
        /*
         *  Debugging code - Output horizontal sides of box
         */
        size_t j;
        fprintf (stderr, "TOP SIDE:\n");
        for (j=0; j<tresult->height; ++j) {
            fprintf (stderr, "  %2d: \'%s\'\n", j,
                    tresult->chars[j]? tresult->chars[j] : "(null)");
        }
        fprintf (stderr, "BOTTOM SIDE:\n");
        for (j=0; j<bresult->height; ++j) {
            fprintf (stderr, "  %2d: \'%s\'\n", j,
                    bresult->chars[j]? bresult->chars[j] : "(null)");
        }
    }
    #endif

    return 0;                            /* all clear */
}



static int vert_generate (sentry_t *lresult, sentry_t *rresult)
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
    size_t leftiltf[SHAPES_PER_SIDE-2];  /* individual lines to fill */
    size_t rightiltf[SHAPES_PER_SIDE-2]; /* individual lines to fill */
    int    rc;                           /* received return code */

    lresult->width = widest (opt.design->shape,
            SHAPES_PER_SIDE, SW, WSW, W, WNW, NW);
    rresult->width = widest (opt.design->shape,
            SHAPES_PER_SIDE, SE, ESE, E, ENE, NE);

    rc = vert_precalc (opt.design->shape, leftiltf, rightiltf, &vspace);
    if (rc) return rc;

    lresult->height = vspace +
        opt.design->shape[NW].height + opt.design->shape[SW].height;
    rresult->height = vspace +
        opt.design->shape[NE].height + opt.design->shape[SE].height;

    #ifdef DEBUG
        fprintf (stderr, "Left side box rect width %d, height %d, vspace %d.\n",
                lresult->width, lresult->height, vspace);
        fprintf (stderr, "Left lines to fill: %s %d, %s %d, %s %d.\n",
                shape_name[west_side[1]], leftiltf[0],
                shape_name[west_side[2]], leftiltf[1],
                shape_name[west_side[3]], leftiltf[2]);
        fprintf (stderr, "Right side box rect width %d, height %d, vspace %d.\n",
                rresult->width, rresult->height, vspace);
        fprintf (stderr, "Right lines to fill: %s %d, %s %d, %s %d.\n",
                shape_name[east_side[1]], rightiltf[0],
                shape_name[east_side[2]], rightiltf[1],
                shape_name[east_side[3]], rightiltf[2]);
    #endif

    lresult->chars = (char **) calloc (lresult->height, sizeof(char *));
    if (lresult->chars == NULL) return 1;
    rresult->chars = (char **) calloc (rresult->height, sizeof(char *));
    if (rresult->chars == NULL) return 1;

    horiz_assemble (opt.design->shape, west_side, leftiltf, lresult);
    horiz_assemble (opt.design->shape, east_side, rightiltf, rresult);

    #if defined(DEBUG) && 1
    {
        /*
         *  Debugging code - Output left and right side of box
         */
        size_t j;
        fprintf (stderr, "LEFT SIDE:\n");
        for (j=0; j<lresult->height; ++j) {
            fprintf (stderr, "  %2d: \'%s\'\n", j,
                    lresult->chars[j]? lresult->chars[j] : "(null)");
        }
        fprintf (stderr, "RIGHT SIDE:\n");
        for (j=0; j<rresult->height; ++j) {
            fprintf (stderr, "  %2d: \'%s\'\n", j,
                    rresult->chars[j]? rresult->chars[j] : "(null)");
        }
    }
    #endif

    return 0;                            /* all clear */
}



int generate_box (sentry_t *thebox)
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

    rc = horiz_generate (&(thebox[0]), &(thebox[2]));
    if (rc) goto err;

    rc = vert_generate (&(thebox[3]), &(thebox[1]));
    if (rc) goto err;

    return 0;                            /* all clear */

err:
    for (i=0; i<ANZ_SIDES; ++i) {
        if (!isempty(&(thebox[i]))) {
            BFREE (thebox[i].chars);     /* free only pointer array */
            memset (thebox+i, 0, sizeof(sentry_t));
        }
    }
    return rc;                           /* error */
}



static int justify_line (line_t *line, int skew)
/*
 *  Justify input line according to specified justification
 *
 *     line   line to justify
 *     skew   difference in spaces right/left of text block (hpr-hpl)
 *
 *  line is assumed to be already free of trailing whitespace.
 *
 *  RETURNS:  == 0  success, input array was modified
 *            != 0  error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    char  *p;                        /* pointer to first non-whitespace char */
    char  *newtext;
    size_t newlen;
    char  *spaces;
    size_t shift;
    size_t oldlen = line->len;

    if (empty_line(line)) {
        line->num_leading_blanks = SIZE_MAX;
        return 0;
    }
    if (opt.justify == '\0') {
        return 0;
    }

    for (p=line->text; *p==' ' || *p=='\t'; ++p);
    newlen = line->len - (p-line->text);

    switch (opt.justify) {

        case 'l':
            if (opt.design->indentmode == 't') {
                memmove (line->text+input.indent, p, newlen+1);
                line->vischar = newlen + input.indent - line->invis;
                line->len = line->vischar;
                line->num_leading_blanks = input.indent;
            }
            else {
                memmove (line->text, p, newlen+1);
                line->vischar = newlen - line->invis;
                line->len = line->vischar;
                line->num_leading_blanks = 0;
            }
            break;

        case 'c':
            if (opt.design->indentmode == 't') {
                shift = (input.maxline - input.indent - newlen + line->invis) / 2 + input.indent;
                skew -= input.indent;
                if ((input.maxline-input.indent - newlen) % 2 && skew == 1)
                    ++shift;
            }
            else {
                shift = (input.maxline - newlen + line->invis) / 2;
                if ((input.maxline - newlen +line->invis) % 2 && skew == 1)
                    ++shift;
            }

            newtext = (char *) calloc (shift + newlen + line->invis + 1, sizeof(char));
            if (newtext == NULL) {
                perror (PROJECT);
                return 2;
            }
            spaces = (char *) malloc (shift+1);
            if (spaces == NULL) {
                perror (PROJECT);
                BFREE (newtext);
                return 3;
            }
            memset (spaces, ' ', shift);
            spaces[shift] = '\0';
            #if defined(DEBUG) && 0
                fprintf (stderr, "j(c): newlen=%d, shift=%d, spaces=\"%s\"\n",
                        newlen, shift, spaces);
            #endif
            strncpy (newtext, spaces, shift);
            strncat (newtext, p, newlen+line->invis);
            newtext[shift+newlen+line->invis] = '\0';
            BFREE (spaces);
            BFREE (line->text);
            line->text = newtext;
            line->len = shift + newlen - line->invis;
            line->vischar = line->len;
            line->num_leading_blanks = shift;
            break;

        case 'r':
            shift = input.maxline - (newlen - line->invis);
            newtext = (char *) calloc (input.maxline+1000, sizeof(char));
            if (newtext == NULL) {
                perror (PROJECT);
                return 2;
            }
            spaces = (char *) malloc (shift+1);
            if (spaces == NULL) {
                perror (PROJECT);
                BFREE (newtext);
                return 3;
            }
            memset (spaces, ' ', shift);
            spaces[shift] = '\0';
            strncpy (newtext, spaces, shift);
            strncat (newtext, p, newlen);
            newtext[input.maxline+line->invis] = '\0';
            BFREE (spaces);
            BFREE (line->text);
            line->text = newtext;
            line->vischar = input.maxline;
            line->len = input.maxline;
            line->num_leading_blanks = shift;
            break;

        default:
            fprintf (stderr, "%s: internal error\n", PROJECT);
            return 1;
    }

    /*
     *  If we might have broken the maxline value, recalculate it.
     */
    if (opt.justify != 'r' && oldlen == input.maxline) {
        size_t k;
        input.maxline = 0;
        for (k=0; k<input.anz_lines; ++k) {
            if (input.lines[k].len > input.maxline) {
                input.maxline = input.lines[k].len;
            }
        }
    }

    return 0;
}



int output_box (const sentry_t *thebox)
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
    size_t  j;
    size_t  nol = thebox[BRIG].height;   /* number of output lines */
    char    trailspc[LINE_MAX+1];
    char   *indentspc;
    int     indentspclen;
    size_t  vfill, vfill1, vfill1_save, vfill2; /* empty lines/columns in box */
    size_t  hfill;
    char   *hfill1, *hfill2;             /* space before/after text */
    size_t  hpl, hpr;
    size_t  r;
    int     rc;
    char    obuf[LINE_MAX+1];            /* final output buffer */
    size_t  obuf_len;                    /* length of content of obuf */
    size_t  skip_start;                  /* lines to skip for box top */
    size_t  skip_end;                    /* lines to skip for box bottom */
    size_t  skip_left;                   /* true if left box part is to be skipped */
    int     ntabs, nspcs;                /* needed for unexpand of tabs */
    char   *restored_indent;
    size_t *contentPos;                  /* column of first char of input text in output text */

    #ifdef DEBUG
        fprintf (stderr, "Padding used: left %d, top %d, right %d, bottom %d\n",
                opt.design->padding[BLEF], opt.design->padding[BTOP],
                opt.design->padding[BRIG], opt.design->padding[BBOT]);
    #endif

    /*
     *  Create string of spaces for indentation
     */
    indentspc = NULL;
    ntabs = nspcs = indentspclen = 0;
    if (opt.design->indentmode == 'b') {
        if (opt.tabexp == 'u') {
            ntabs = input.indent / opt.tabstop;
            nspcs = input.indent % opt.tabstop;
            indentspclen = ntabs + nspcs;
        }
        else {
            indentspclen = input.indent;
        }
        indentspc = (char *) malloc (indentspclen + 1);
        if (indentspc == NULL) {
            perror (PROJECT);
            return 1;
        }

        if (opt.tabexp == 'u') {
            memset (indentspc, (int)'\t', ntabs);
            memset (indentspc + ntabs, (int)' ', nspcs);
        }
        else {
            memset (indentspc, (int)' ', indentspclen);
        }
        indentspc[indentspclen] = '\0';
    }
    else {
        indentspc = (char *) strdup ("");
        if (indentspc == NULL) {
            perror (PROJECT);
            return 1;
        }
    }

    /*
     *  Provide string of spaces for filling of space between text and
     *  right side of box
     */
    memset (trailspc, (int)' ', LINE_MAX);
    trailspc[LINE_MAX] = '\0';

    /*
     *  Compute number of empty lines in box (vfill).
     */
    vfill = nol - thebox[BTOP].height - thebox[BBOT].height - input.anz_lines;
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
    vfill  += opt.design->padding[BTOP] + opt.design->padding[BBOT];
    vfill1_save = vfill1;

    /*
     *  Provide strings for horizontal text alignment.
     */
    hfill = thebox[BTOP].width - input.maxline;
    hfill1 = (char *) malloc (hfill+1);
    hfill2 = (char *) malloc (hfill+1);
    if (!hfill1 || !hfill2) {
        perror (PROJECT);
        return 1;
    }
    memset (hfill1, (int)' ', hfill+1);
    memset (hfill2, (int)' ', hfill+1);
    hfill1[hfill] = '\0';
    hfill2[hfill] = '\0';
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
            hpl = hfill/2 + opt.design->padding[BLEF];
            hpr = hfill/2 + opt.design->padding[BRIG] + (hfill%2);
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
    hfill1[hpl] = '\0';
    hfill2[hpr] = '\0';

    #if defined(DEBUG)
        fprintf (stderr, "Alignment: hfill %d hpl %d hpr %d, vfill %d "
            "vfill1 %d vfill2 %d.\n", hfill, hpl, hpr, vfill, vfill1, vfill2);
        fprintf (stderr, "           hfill1 = \"%s\";  hfill2 = \"%s\";  "
                "indentspc = \"%s\";\n", hfill1, hfill2, indentspc);
    #endif

    /*
     *  Find out if and how many leading or trailing blank lines must be
     *  skipped because the corresponding box side was defined empty.
     */
    skip_start = 0;
    skip_end   = 0;
    skip_left  = 0;
    if (empty_side (opt.design->shape, BTOP))
        skip_start = opt.design->shape[NW].height;
    if (empty_side (opt.design->shape, BBOT))
        skip_end = opt.design->shape[SW].height;
    if (empty_side (opt.design->shape, BLEF))
        skip_left = opt.design->shape[NW].width; /* could simply be 1, though */
    #if defined(DEBUG)
        fprintf (stderr, "skip_start = %d;  skip_end = %d;  skip_left = %d;  "
                "nol = %d;\n", skip_start, skip_end, skip_left, nol);
    #endif

    /*
     *  Generate actual output
     */
    contentPos = calloc (input.anz_lines, sizeof(size_t));
    for (j=skip_start; j<nol-skip_end; ++j) {

        if (j < thebox[BTOP].height) {   /* box top */
            restored_indent = tabbify_indent (0, indentspc, indentspclen);
            concat_strings (obuf, LINE_MAX+1, 4, restored_indent,
                    skip_left?"":thebox[BLEF].chars[j], thebox[BTOP].chars[j],
                    thebox[BRIG].chars[j]);
        }

        else if (vfill1) {               /* top vfill */
            r = thebox[BTOP].width;
            trailspc[r] = '\0';
            restored_indent = tabbify_indent (0, indentspc, indentspclen);
            concat_strings (obuf, LINE_MAX+1, 4, restored_indent,
                    skip_left?"":thebox[BLEF].chars[j], trailspc,
                    thebox[BRIG].chars[j]);
            trailspc[r] = ' ';
            --vfill1;
        }

        else if (j < nol-thebox[BBOT].height) {
            long ti = j - thebox[BTOP].height - (vfill-vfill2);
            if (ti < (long) input.anz_lines) {      /* box content (lines) */
                #if defined(DEBUG) && 0
                    fprintf (stderr, "justify_line ({\"%s\",%d}, %d);\n",
                            input.lines[ti].text, input.lines[ti].len, hpr-hpl);
                #endif
                rc = justify_line (input.lines+ti, hpr-hpl);
                if (rc)
                    return rc;
                r = input.maxline - input.lines[ti].vischar;
                trailspc[r] = '\0';
                restored_indent = tabbify_indent (ti, indentspc, indentspclen);
                if (input.lines[ti].num_leading_blanks == SIZE_MAX) {
                    contentPos[ti] = SIZE_MAX;
                } else {
                    contentPos[ti] = strlen(restored_indent)
                            + (skip_left? 0: strlen(thebox[BLEF].chars[j]))
                            + strlen(hfill1)
                            + input.lines[ti].num_leading_blanks;
                }
                concat_strings (obuf, LINE_MAX+1, 7, restored_indent,
                        skip_left?"":thebox[BLEF].chars[j], hfill1,
                        ti >= 0? input.lines[ti].text : "", hfill2,
                        trailspc, thebox[BRIG].chars[j]);
            }
            else {                       /* bottom vfill */
                r = thebox[BTOP].width;
                trailspc[r] = '\0';
                restored_indent = tabbify_indent (input.anz_lines - 1, indentspc, indentspclen);
                concat_strings (obuf, LINE_MAX+1, 4, restored_indent,
                        skip_left?"":thebox[BLEF].chars[j], trailspc,
                        thebox[BRIG].chars[j]);
            }
            trailspc[r] = ' ';
        }

        else {                           /* box bottom */
            restored_indent = tabbify_indent (input.anz_lines - 1, indentspc, indentspclen);
            concat_strings (obuf, LINE_MAX+1, 4, restored_indent,
                    skip_left?"":thebox[BLEF].chars[j],
                    thebox[BBOT].chars[j-(nol-thebox[BBOT].height)],
                    thebox[BRIG].chars[j]);
        }

        obuf_len = strlen (obuf);

        if (obuf_len > LINE_MAX) {
            size_t newlen = LINE_MAX;
            btrim (obuf, &newlen);
        }
        else {
            btrim (obuf, &obuf_len);
        }
        if (opt.tabexp == 'k') {
            BFREE (restored_indent);
        }

        fprintf (opt.outfile, "%s%s", obuf, (input.final_newline || j < nol-skip_end-1 ? "\n" : ""));
    }

    /* add info line for web ui if requested with -q */
    if (opt.q) {
        fprintf (opt.outfile, "%d ", (int) (thebox[BTOP].height + vfill1_save - skip_start));
        for (j = 0; j < input.anz_lines; j++) {
            fprintf (opt.outfile, "%d%s",
                (contentPos[j] == SIZE_MAX ? (int) -1 : (int) contentPos[j]),
                j < input.anz_lines-1 ? " ": "\n");
        }
    }

    BFREE (contentPos);
    BFREE (indentspc);
    BFREE (hfill1);
    BFREE (hfill2);
    return 0;                            /* all clear */
}



/*EOF*/                                                 /* vim: set sw=4: */
