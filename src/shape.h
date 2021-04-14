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
 * Shape handling and information functions
 */

#ifndef SHAPE_H
#define SHAPE_H


typedef enum {
    NW, NNW, N, NNE, NE, ENE, E, ESE, SE, SSE, S, SSW, SW, WSW, W, WNW
} shape_t;

extern char *shape_name[];

#define NUM_SHAPES 16

#define SHAPES_PER_SIDE 5
#define NUM_SIDES       4
#define NUM_CORNERS     4

extern shape_t north_side[SHAPES_PER_SIDE];  /* groups of shapes, clockwise */
extern shape_t  east_side[SHAPES_PER_SIDE];
extern shape_t south_side[SHAPES_PER_SIDE];
extern shape_t  west_side[SHAPES_PER_SIDE];
extern shape_t corners[NUM_CORNERS];
extern shape_t *sides[NUM_SIDES];


typedef struct {
    char **chars;
    size_t height;
    size_t width;
    int    elastic;          /* elastic is used only in original definition */
} sentry_t;

#define SENTRY_INITIALIZER (sentry_t) {NULL, 0, 0, 0}



int genshape (const size_t width, const size_t height, char ***chars);
void freeshape (sentry_t *shape);

shape_t findshape (const sentry_t *sarr, const int num);
int on_side (const shape_t s, const int idx);

int isempty (const sentry_t *shape);
int isdeepempty (const sentry_t *shape);

size_t highest (const sentry_t *sarr, const int n, ...);
size_t widest (const sentry_t *sarr, const int n, ...);

shape_t leftmost (const int aside, const int cnt);

int empty_side (sentry_t *sarr, const int aside);



#endif /*SHAPE_H*/

/*EOF*/                                          /* vim: set cindent sw=4: */
