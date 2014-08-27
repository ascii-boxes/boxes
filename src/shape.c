/*
 *  File:             shape.c
 *  Project Main:     boxes.c
 *  Date created:     June 23, 1999 (Wednesday, 13:39h)
 *  Author:           Copyright (C) 1999 Thomas Jensen <boxes@thomasjensen.com>
 *  Version:          $Id: shape.c,v 1.6 2006/07/12 05:27:29 tsjensen Exp $
 *  Language:         ANSI C
 *  World Wide Web:   http://boxes.thomasjensen.com/
 *  Purpose:          Shape handling and information functions
 *
 *  Remarks: o This program is free software; you can redistribute it and/or
 *             modify it under the terms of the GNU General Public License as
 *             published by the Free Software Foundation; either version 2 of
 *             the License, or (at your option) any later version.
 *           o This program is distributed in the hope that it will be useful,
 *             but WITHOUT ANY WARRANTY; without even the implied warranty of
 *             MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *             GNU General Public License for more details.
 *           o You should have received a copy of the GNU General Public
 *             License along with this program; if not, write to the Free
 *             Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *             MA 02111-1307  USA
 *
 *  Revision History:
 *
 *    $Log: shape.c,v $
 *    Revision 1.6  2006/07/12 05:27:29  tsjensen
 *    Updated email and web addresses in comment header
 *
 *    Revision 1.5  1999-08-14 12:11:53-07  tsjensen
 *    Added findshape(), genshape(), freeshape(), and isdeepempty() functions
 *    Added on_side() back in with a few changes
 *    Changed empty_side() to use isdeepempty() -> much simpler code
 *
 *    Revision 1.4  1999/07/23 16:38:02  tsjensen
 *    Removed functions iscorner(), on_side(), shapecmp(), both_on_side(), and
 *    shape_distance() - nobody was using them anyway.
 *
 *    Revision 1.3  1999/07/22 12:28:25  tsjensen
 *    Added GNU GPL disclaimer
 *    Added include config.h
 *
 *    Revision 1.2  1999/06/25 18:52:28  tsjensen
 *    Added empty_side() function from boxes.c
 *    Removed #include regexp.h
 *
 *    Revision 1.1  1999/06/23 12:26:59  tsjensen
 *    Initial revision
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */

#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "shape.h"
#include "boxes.h"
#include "tools.h"

static const char rcsid_shape_c[] =
    "$Id: shape.c,v 1.6 2006/07/12 05:27:29 tsjensen Exp $";



char *shape_name[] = {
    "NW", "NNW", "N", "NNE", "NE", "ENE", "E", "ESE",
    "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW"
};

shape_t north_side[SHAPES_PER_SIDE] = { NW, NNW, N, NNE, NE };  /* clockwise */
shape_t  east_side[SHAPES_PER_SIDE] = { NE, ENE, E, ESE, SE };
shape_t south_side[SHAPES_PER_SIDE] = { SE, SSE, S, SSW, SW };
shape_t  west_side[SHAPES_PER_SIDE] = { SW, WSW, W, WNW, NW };
shape_t corners[ANZ_CORNERS] = { NW, NE, SE, SW };
shape_t *sides[] = { north_side, east_side, south_side, west_side };




shape_t findshape (const sentry_t *sarr, const int num)
/*
 *  Find a non-empty shape and return its name
 *
 *      sarr    the shape array to check
 *      num     number of entries in sarr to be checked
 *
 *  RETURNS: a shape_name  on success
 *           num           on error (e.g. empty shape array)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i;

    for (i=0; i<num; ++i) {
        if (isempty(sarr+i))
            continue;
        else
            break;
    }

    return (shape_t) i;
}



int on_side (const shape_t s, const int idx)
/*
 *  Compute the side that shape s is on.
 *
 *      s    shape to look for
 *      idx  which occurence to return (0 == first, 1 == second (for corners)
 *
 *  RETURNS: side number (BTOP etc.)  on success
 *           ANZ_SIDES                on error (e.g. idx==1 && s no corner)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int side;
    int i;
    int found = 0;

    for (side=0; side<ANZ_SIDES; ++side) {
        for (i=0; i<SHAPES_PER_SIDE; ++i) {
            if (sides[side][i] == s) {
                if (found == idx)
                    return side;
                else
                    ++found;
            }
        }
    }

    return ANZ_SIDES;
}



int genshape (const size_t width, const size_t height, char ***chars)
/*
 *  Generate a shape consisting of spaces only.
 *
 *      width   desired shape width
 *      height  desired shape height
 *      chars   pointer to the shape lines (should be NULL upon call)
 *
 *  Memory is allocated for the shape lines which must be freed by the caller.
 *
 *  RETURNS:  == 0   on success (memory allocated)
 *            != 0   on error   (no memory allocated)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t j;

    if (width <= 0 || height <= 0 || width > LINE_MAX) {
        fprintf (stderr, "%s: internal error\n", PROJECT);
        return 1;
    }

    *chars = (char **) calloc (height, sizeof(char *));
    if (*chars == NULL) {
        perror (PROJECT);
        return 2;
    }

    for (j=0; j<height; ++j) {
        (*chars)[j] = (char *) calloc (width+1, sizeof(char));
        if ((*chars)[j] == NULL) {
            perror (PROJECT);
            for (/*empty*/; j>0; --j)
                BFREE ((*chars)[j-1]);
            BFREE (*chars);
            return 3;
        }
        memset ((*chars)[j], ' ', width);
    }

    return 0;
}



void freeshape (sentry_t *shape)
/*
 *  Free all memory allocated by the shape and set the struct to
 *  SENTRY_INITIALIZER. Do not free memory of the struct.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t j;

    for (j=0; j<shape->height; ++j)
        BFREE (shape->chars[j]);
    BFREE (shape->chars);

    *shape = SENTRY_INITIALIZER;
}



int isempty (const sentry_t *shape)
/*
 *  Return true if shape is empty.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    if (shape == NULL)
        return 1;
    else if (shape->chars == NULL)
        return 1;
    else if (shape->width == 0 || shape->height == 0)
        return 1;
    else
        return 0;
}



int isdeepempty (const sentry_t *shape)
/*
 *  Return true if shape is empty, also checking if lines consist of spaces
 *  only.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    size_t j;

    if (isempty (shape))
        return 1;

    for (j=0; j<shape->height; ++j) {
        if (shape->chars[j]) {
            if (strspn (shape->chars[j], " ") != shape->width)
                return 0;
        }
    }

    return 1;
}



size_t highest (const sentry_t *sarr, const int n, ...)
/*
 *  Return height (vert.) of highest shape in given list.
 *
 *  sarr         array of shapes to examine
 *  n            number of shapes following
 *  ...          the shapes to consider
 *
 *  RETURNS:     height in lines (may be zero)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    va_list ap;
    int i;
    size_t max = 0;                      /* current maximum height */

    #if defined(DEBUG) && 0
        fprintf (stderr, "highest (%d, ...)\n", n);
    #endif

    va_start (ap, n);

    for (i=0; i<n; ++i) {
        shape_t r = va_arg (ap, shape_t);
        if (!isempty (sarr + r)) {
            if (sarr[r].height > max)
                max = sarr[r].height;
        }
    }

    va_end (ap);

    return max;
}



size_t widest (const sentry_t *sarr, const int n, ...)
/*
 *  Return width (horiz.) of widest shape in given list.
 *
 *  sarr         array of shapes to examine
 *  n            number of shapes following
 *  ...          the shapes to consider
 *
 *  RETURNS:     width in chars (may be zero)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    va_list ap;
    int i;
    size_t max = 0;                      /* current maximum width */

    #if defined(DEBUG) && 0
        fprintf (stderr, "widest (%d, ...)\n", n);
    #endif

    va_start (ap, n);

    for (i=0; i<n; ++i) {
        shape_t r = va_arg (ap, shape_t);
        if (!isempty (sarr + r)) {
            if (sarr[r].width > max)
                max = sarr[r].width;
        }
    }
    
    va_end (ap);

    return max;
}



shape_t leftmost (const int aside, const int cnt)
/*
 *  Return leftmost existing shape in specification for side aside
 *  (BTOP or BBOT), skipping cnt shapes. Corners are not considered.
 *
 *  RETURNS:    shape       if shape was found
 *              ANZ_SHAPES  on error (e.g. cnt too high)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int c = 0;
    int s;

    if (cnt < 0)
        return ANZ_SHAPES;

    if (aside == BTOP) {
        s = 0;
        do {
            ++s;
            while (s < SHAPES_PER_SIDE-1 &&
                    isempty(opt.design->shape + north_side[s]))
                ++s;
            if (s == SHAPES_PER_SIDE-1)
                return ANZ_SHAPES;
        } while (c++ < cnt);
        return north_side[s];
    }

    else if (aside == BBOT) {
        s = SHAPES_PER_SIDE - 1;
        do {
            --s;
            while (s && isempty(opt.design->shape + south_side[s]))
                --s;
            if (!s)
                return ANZ_SHAPES;
        } while (c++ < cnt);
        return south_side[s];
    }

    return ANZ_SHAPES;
}



int empty_side (sentry_t *sarr, const int aside)
/*
 *  Return true if the shapes on the given side consist entirely out of
 *  spaces - and spaces only, tabs are considered non-empty.
 *
 *      sarr    pointer to shape list of design to check
 *      aside   the box side (one of BTOP etc.)
 *
 *  RETURNS:  == 0   side is not empty
 *            != 0   side is empty
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i;

    for (i=0; i<SHAPES_PER_SIDE; ++i) {
        if (isdeepempty (sarr + sides[aside][i]))
            continue;
        else
            return 0;                    /* side is not empty */
    }

    return 1;                            /* side is empty */
}



/*EOF*/                                                 /* vim: set sw=4: */
