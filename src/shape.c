/*
 *  File:             shape.c
 *  Project Main:     boxes.c
 *  Date created:     June 23, 1999 (Wednesday, 13:39h)
 *  Author:           Copyright (C) 1999 Thomas Jensen
 *                    tsjensen@stud.informatik.uni-erlangen.de
 *  Version:          $Id: shape.c,v 1.2 1999/06/25 18:52:28 tsjensen Exp tsjensen $
 *  Language:         ANSI C
 *  World Wide Web:   http://home.pages.de/~jensen/boxes/
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
#include "shape.h"
#include "boxes.h"

static const char rcsid_shape_c[] =
    "$Id: shape.c,v 1.2 1999/06/25 18:52:28 tsjensen Exp tsjensen $";



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




int iscorner (const shape_t s)
/*
 *  Return true if shape s is a corner.
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i;
    shape_t *p;

    for (i=0, p=corners; i<ANZ_CORNERS; ++i, ++p) {
        if (*p == s)
            return 1;
    }

    return 0;
}



shape_t *on_side (const shape_t s, const int idx)
/*
 *  Compute the side that shape s is on.
 *
 *      s    shape to look for
 *      idx  which occurence to return (0 == first, 1 == second (for corners)
 *
 *  RETURNS: pointer to a side list  on success
 *           NULL                    on error (e.g. idx==1 && s no corner)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int       side;
    int       i;
    shape_t **sp;
    shape_t  *p;
    int       found = 0;

    for (side=0,sp=sides; side<ANZ_SIDES; ++side,++sp) {
        for (i=0,p=*sp; i<SHAPES_PER_SIDE; ++i,++p) {
            if (*p == s) {
                if (found == idx)
                    return *sp;
                else
                    ++found;
            }
        }
    }

    return NULL;
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



int shapecmp (const sentry_t *shape1, const sentry_t *shape2)
/*
 *  Compare two shapes.
 *
 *  RETURNS: == 0   if shapes are equal
 *           != 0   if shapes differ
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int    e1 = isempty (shape1);
    int    e2 = isempty (shape2);
    size_t i;

    if ( e1 &&  e2) return 0;
    if (!e1 &&  e2) return 1;
    if ( e1 && !e2) return -1;

    if (shape1->width != shape2->width || shape1->height != shape2->height) {
        if (shape1->width * shape1->height > shape2->width * shape2->height)
            return 1;
        else
            return -1;
    }

    for (i=0; i<shape1->height; ++i) {
        int c = strcmp (shape1->chars[i], shape2->chars[i]);  /* no casecmp! */
        if (c) return c;
    }

    return 0;
}



shape_t *both_on_side (const shape_t shape1, const shape_t shape2)
/*
 *  Compute the side that *both* shapes are on.
 *
 *  RETURNS: pointer to a side list  on success
 *           NULL                    on error (e.g. shape on different sides)
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i, j, found;

    for (i=0; i<ANZ_SIDES; ++i) {
        found = 0;
        for (j=0; j<SHAPES_PER_SIDE; ++j) {
            if (sides[i][j] == shape1 || sides[i][j] == shape2)
                ++found;
            if (found > 1) {
                switch (i) {
                    case 0: return north_side;
                    case 1: return east_side;
                    case 2: return south_side;
                    case 3: return west_side;
                    default: return NULL;
                }
            }
        }
    }

    return NULL;
}



int shape_distance (const shape_t s1, const shape_t s2)
/*
 *  Compute distance between two shapes which are located on the same side
 *  of the box. E.g. shape_distance(NW,N) == 2.
 *
 *  RETURNS: distance in steps   if ok
 *           -1                  on error
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int i;
    int distance = -1;
    shape_t *workside = both_on_side (s1, s2);

    if (!workside) return -1;
    if (s1 == s2) return 0;

    for (i=0; i<SHAPES_PER_SIDE; ++i) {
        if (workside[i] == s1 || workside[i] == s2) {
            if (distance == -1)
                distance = 0;
            else if (distance > -1) {
                ++distance;
                break;
            }
        }
        else {
            if (distance > -1)
                ++distance;
        }
    }

    if (distance > 0 && distance < SHAPES_PER_SIDE)
        return distance;
    else
        return -1;
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



int empty_side (sentry_t *d, const int aside)
/*
 *  Return true if the shapes on the given side consist entirely out of
 *  spaces - and spaces only, tabs are considered non-empty.
 *
 *      d       pointer to shape list of design to check
 *      aside   the box side (one of BTOP etc.)
 *
 *  RETURNS:  == 0   side is not empty
 *            != 0   side is empty
 *
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 */
{
    int       i;
    size_t    j;
    sentry_t *cs;                        /* current shape */
    char     *p;

    for (i=0; i<SHAPES_PER_SIDE; ++i) {
        cs = d + sides[aside][i];
        if (isempty(cs))
            continue;
        for (j=0; j<cs->height; ++j) {
            p = cs->chars[j];
            while (*p && *p == ' ')
                ++p;
            if (*p)
                return 0;                /* side is not empty */
        }
    }

    return 1;                            /* side is empty */
}



/*EOF*/                                                 /* vim: set sw=4: */
